#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__


#include "constants.h"
#include "coordinate.h"
#include "printprofile.h"
#include "movementsequencer.h"
#include "ordermanifestmanager.h"


enum class SliceDirectoryType
{
    SLICE_BASE,
    SLICE_BODY
};

enum class SliceType
{
    SliceBase,
    SliceBody,
};


enum class WaitBeforeZmoveType{
    WaitBeforeLift,
    WaitBeforeProject
};



class SliceInformation
{
public:
    SliceInformation(SliceType t):
        type(t), layerCount(-1), layerThickness(100)
    {
        if (t == SliceType::SliceBase)
            layerCount = 2;
    }

    SliceType type;
    QString sliceDirectory;
    bool isPreSliced;
    int layerCount;
    int layerThickness;
    double exposureTime;
};

class PrintJob
{
public:
    PrintJob() = default;
    PrintJob(const PrintJob &other) = default;

    QString modelFileName;
    QString directoryPath;
    QString modelHash;
    QString currentImageFile;
    
    size_t                  vertexCount     { };
    Coordinate              x               { };
    Coordinate              y               { };
    Coordinate              z               { };
    double                  estimatedVolume { }; // unit: µL
    int             firstLayerOffset;
    bool            directoryMode;

    SliceInformation        baseSlices { SliceType::SliceBase };
    SliceInformation        bodySlices { SliceType::SliceBody };

    //TODO copy over all profile data exactly on printtab print button clicked
    QSharedPointer<PrintProfile>    printProfile    { };

    bool isTiled()
    {
        if(_bodyManager)
            return _bodyManager->tiled();

        return false;
    }

    bool hasLayerMoreElements(int checkedImage)
    {
        if (checkedImage+1 == totalLayerCount()) {
        return false;
        }

        if(isTiled())
        {
            if(checkedImage == 0){
              return (tilingCount()>1) ? true : false;
            }else{
              return (checkedImage+1) % tilingCount();
            }

        }else{
         return false;
        }

    }

    WaitBeforeZmoveType getPauseTypeBeforeZmove(int layerNum)
    {
            if(getLayerProfileParams(layerNum).isPumpingEnabled()){
                return WaitBeforeZmoveType::WaitBeforeLift;
            } else {
                return WaitBeforeZmoveType::WaitBeforeProject;
            }
    }

    double getLayerPowerLevel(int layerNum)
    {
        return getLayerProfileParams(layerNum).powerLevel();
    }

    QList<MovementInfo> getMovementAfterLayer(int layerNum)
    {
        QList<MovementInfo> movements;

        if(getLayerProfileParams(layerNum).isPumpingEnabled())
        {
            movements.push_back( { MoveType::Relative,
                                   getLayerProfileParams(layerNum).pumpUpDistance( ),
                                   getLayerProfileParams(layerNum).pumpUpVelocity_Effective( ) } );

            movements.push_back( { getLayerProfileParams(layerNum).pumpUpPause( ) } );

            //TODO check if proper layer number is selected
            movements.push_back( { MoveType::Relative,
                                   -getLayerProfileParams(layerNum).pumpDownDistance_Effective() +
                                       (getLayerThicknessAt(layerNum - tilingCount() + 1) / 1000.0),
                                   getLayerProfileParams(layerNum).pumpDownVelocity_Effective( ) } );

            movements.push_back( { getLayerProfileParams(layerNum).pumpDownPause( ) } );

        }else{
             movements.push_back( { MoveType::Relative,
                                    getLayerThicknessAt(layerNum) / 1000.0,
                                    PrinterDefaultLowSpeed } );
        }



        return movements;

    }




    bool hasBasicControlsEnabled()
    {
        if (isTiled())
            return false;

        return !_advancedControlsEnabled;
    }

    bool hasAdvancedControlsEnabled()
    {
        if (isTiled())
            return false;

        return _advancedControlsEnabled;
    }

    void enableAdvancedControls(bool enable)
    {
        _advancedControlsEnabled = enable;
    }

    bool hasBaseLayers() const
    {
        return baseSlices.layerCount > 0;
    }

    int totalLayerCount() const
    {
        return baseSlices.layerCount + bodySlices.layerCount;
    }

    int slicedBaseLayerCount() const
    {
        return _bodyManager->getSize();
    }

    int baseThickness() const
    {
        return baseSlices.layerCount * baseSlices.layerThickness;
    }

    int baseLayerThickness()
    {
        if (isTiled())
            return hasBaseLayers() ? _baseManager->layerThickNessAt(0) : 0;
        else
            return hasBaseLayers() ? baseSlices.layerThickness : 0;
    }

    int bodyLayerThickness()
    {
        return isTiled()
            ? _bodyManager->layerThickNessAt(0)
            : bodySlices.layerThickness;
    }

    int getLayerThicknessAt(int layerNo)
    {
        if (isTiled()) {
            if (hasBaseLayers()) {
                return layerNo <= _baseManager->getSize()
                    ? _baseManager->layerThickNessAt(layerNo)
                    : _bodyManager->layerThickNessAt(layerNo);
            } else
                return _bodyManager->layerThickNessAt(layerNo);
        } else {
            if (hasBaseLayers()) {
                return layerNo <= baseSlices.layerCount
                    ? baseSlices.layerThickness
                    : bodySlices.layerThickness;
            } else
                return bodySlices.layerThickness;
        }
    }

    int baseLayerStart() const
    {
        return hasBaseLayers() ? 0 : -1;
    }

    int baseLayerEnd() const
    {
        return hasBaseLayers() ? baseSlices.layerCount - 1 : -1;
    }

    int bodyLayerStart() const
    {   
        return hasBaseLayers() ? baseThickness() / bodySlices.layerThickness : 0;
    }

    int bodyLayerEnd() const
    {
        return bodySlices.layerCount + bodyLayerStart() - 1;
    }

    bool isBaseLayer(int const layer) const
    {
        if (!hasBaseLayers())
            return false;

        return (layer >= baseLayerStart()) && (layer <= baseLayerEnd());
    }

    PrintParameters& getLayerProfileParams(int layerNum)
    {
        if(isBaseLayer(layerNum)){
            return printProfile->baseLayerParameters();
        } else {
            return printProfile->bodyLayerParameters();
        }
    }

    QString getLayerDirectory(int const layer) const
    {
        if (directoryMode)
            return directoryPath;

        return isBaseLayer(layer) ? baseSlices.sliceDirectory : bodySlices.sliceDirectory;
    }

    QString getLayerFileName(int const layer) const
    {
        return isBaseLayer(layer)
            ? _baseManager->getElementAt(layer)
            : _bodyManager->getElementAt(bodyLayerStart() + layer - baseSlices.layerCount);
    }

    QString getLayerPath( int const layer ) const {
        return QString("%1/%2").arg(getLayerDirectory(layer)).arg(getLayerFileName(layer));
    }

    double getTimeForElementAt( int position ) {
        if(_bodyManager && isTiled())
            return _bodyManager->getTimeForElementAt( position );

        else return -1.0;
    }

    QSharedPointer<OrderManifestManager> getBaseManager()
    {
        return _baseManager;
    }

    QSharedPointer<OrderManifestManager> getBodyManager()
    {
        return _bodyManager;
    }

    void setBodyManager(QSharedPointer<OrderManifestManager> manager) {
        _bodyManager.swap(manager);
        bodySlices.isPreSliced = true;
        bodySlices.layerCount = _bodyManager->getSize() - bodyLayerStart();

        if (_bodyManager->tiled())
            bodySlices.layerThickness = -1;
    }

    void setBaseManager(QSharedPointer<OrderManifestManager> manager)
    {
        _baseManager.swap(manager);

        if (!_baseManager.isNull()) {
            baseSlices.isPreSliced = true;
            baseSlices.layerCount = std::min(baseSlices.layerCount, _baseManager->getSize());
            if (_baseManager->tiled())
                baseSlices.layerThickness = -1;

        } else {
            baseSlices.sliceDirectory = nullptr;
            baseSlices.isPreSliced = false;
            baseSlices.layerCount = 0;
            baseSlices.layerThickness = -1;
        }
    }

    int tilingCount()
    {
        if(!_bodyManager.isNull() && isTiled())
            return _bodyManager->tilingCount();

        return 0;
    }

private:
    QSharedPointer<OrderManifestManager> _bodyManager {};
    QSharedPointer<OrderManifestManager> _baseManager {};
    bool _advancedControlsEnabled;
};

#endif // __PRINTJOB_H__
