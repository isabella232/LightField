#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__


#include "constants.h"
#include "coordinate.h"
#include "printprofile.h"
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



class SliceInformation
{
public:
    SliceInformation(SliceType t):
        type(t), layerCount(0), layerThickness(0)
    {
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

    QSharedPointer<PrintProfile>    printProfile    { };

    bool isTiled()
    {
        if(_bodyManager)
            return _bodyManager->tiled();

        return false;
    }

    bool hasBaseLayers() const
    {
        return baseSlices.layerCount > 0;
    }

    int totalLayerCount() const
    {
        return baseSlices.layerCount + bodySlices.layerCount;
    }

    int baseThickness() const
    {
        return baseSlices.layerCount * baseSlices.layerThickness;
    }

    int baseLayerThickness()
    {
        if(isTiled()){
            if(hasBaseLayers()){
                return _baseManager->layerThickNessAt(0);
            } else {
                return 0;
            }
        } else {
            if(hasBaseLayers()){
                return baseSlices.layerThickness;
            }else{
                return 0;
            }
        }
    }

    int bodyLayerThickness(){
        if(isTiled()){
            return _bodyManager->layerThickNessAt(0);
        } else {
            return bodySlices.layerThickness;
        }
    }

    int getLayerThicknessAt(int layerNo){
        if(isTiled()){
            if(hasBaseLayers()){
                if(layerNo <= _baseManager->getSize()){
                    return _baseManager->layerThickNessAt(layerNo);
                } else {
                    return _bodyManager->layerThickNessAt(layerNo);
                }
            } else {
                return _bodyManager->layerThickNessAt(layerNo);
            }
        } else {
            if(hasBaseLayers()){
                if(layerNo <= baseSlices.layerCount) {
                    return baseSlices.layerThickness;
                } else {
                    return bodySlices.layerThickness;
                }
            } else {
                return bodySlices.layerThickness;
            }
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

    void setBodyManager(QSharedPointer<OrderManifestManager> manager) {
        _bodyManager.swap(manager);
        bodySlices.isPreSliced = true;
        bodySlices.layerCount = _bodyManager->getSize() - bodyLayerStart();

        if(_bodyManager->tiled()){
            bodySlices.layerThickness = _bodyManager->layerThickNessAt(0);
        }
        updateProfileLayersInfo();

    }

    void setBaseManager(QSharedPointer<OrderManifestManager> manager)
    {
        _baseManager.swap(manager);

        if(!_baseManager.isNull()) {
            baseSlices.isPreSliced = true;
            baseSlices.layerCount = std::min(baseSlices.layerCount, _baseManager->getSize());
            if(_baseManager->tiled()){
                baseSlices.layerThickness = _baseManager->layerThickNessAt(0);
            }
        } else {
            baseSlices.sliceDirectory = nullptr;
            baseSlices.isPreSliced = false;
            baseSlices.layerCount = 0;
            baseSlices.layerThickness = -1;
        }
        updateProfileLayersInfo();
    }

    int tilingCount()
    {
        if(!_bodyManager.isNull() && isTiled())
            return _bodyManager->tilingCount();

        return 0;
    }

    void updateProfileLayersInfo(){
        printProfile->baseLayerParameters().setLayerThickness(baseLayerThickness());
        printProfile->bodyLayerParameters().setLayerThickness(bodyLayerThickness());
        printProfile->setBaseLayerCount(baseSlices.layerCount);
    }

    bool isProfileLayerInfoSync(){
        if(baseLayerThickness() != printProfile->baseLayerParameters().layerThickness()) return false;
        if(bodyLayerThickness() != printProfile->bodyLayerParameters().layerThickness()) return false;
        if(baseSlices.layerCount != printProfile->baseLayerCount()) return false;
        return true;
    }

private:
    QSharedPointer<OrderManifestManager> _bodyManager {};
    QSharedPointer<OrderManifestManager> _baseManager {};
};

#endif // __PRINTJOB_H__
