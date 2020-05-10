#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include <QtCore>
#include "constants.h"
#include "coordinate.h"
#include "printprofile.h"
#include "ordermanifestmanager.h"

enum class SliceType
{
    SliceBase,
    SliceBody,
};

class SliceInformation
{
public:
    SliceInformation(SliceType t): type(t)
    {
    }

    SliceType type;
    QString sliceDirectory;
    bool isPreSliced;
    int layerCount;
    int layerThickness;
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
    //MERGE_TODO to be removed?
    double          exposureTime            { 1.0 }; // unit: s
    double          exposureTimeScaleFactor { 1.0 }; // for first two layers
    int             firstLayerOffset;
    bool            directoryMode;

    SliceInformation        baseSlices { SliceType::SliceBase };
    SliceInformation        bodySlices { SliceType::SliceBody };

    PrintProfile*    printProfile    { };

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
        return hasBaseLayers() ? baseSlices.layerCount : 0;
    }

    int bodyLayerEnd() const
    {
        return baseSlices.layerCount + bodySlices.layerCount - 1;
    }

    bool isBaseLayer(int const layer) const
    {
        if (!hasBaseLayers())
            return false;

        return (layer >= baseLayerStart()) && (layer <= baseLayerEnd());
    }

    QString getLayerDirectory(int const layer) const
    {
        return isBaseLayer(layer) ? baseSlices.sliceDirectory :  bodySlices.sliceDirectory;
    }

    QString getLayerFileName(int const layer) const
    {
        return isBaseLayer(layer)
            ? _baseManager->getElementAt(layer)
            : _bodyManager->getElementAt(layer - baseSlices.layerCount);
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
        bodySlices.layerCount = _bodyManager->getSize() - baseSlices.layerCount;

        if(_bodyManager->tiled())
            bodySlices.layerThickness = _bodyManager->layerThickNessAt(0);

    }

    void setBaseManager(QSharedPointer<OrderManifestManager> manager)
    {
        _baseManager.swap(manager);

        if(!_baseManager.isNull()) {
            baseSlices.isPreSliced = true;
            baseSlices.layerCount = std::min(baseSlices.layerCount, _baseManager->getSize());
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
};

#endif // __PRINTJOB_H__
