#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include <QtCore>
#include "constants.h"
#include "coordinate.h"
#include "printprofile.h"
#include "ordermanifestmanager.h"

class SliceInformation {

public:



    bool    isPreSliced      { };

    int     layerCount       { };
    int     layerThickness   { }; // unit: µm
    int     firstLayerOffset { }; // unit: µm

    int     startLayer       { -1 };
    int     endLayer         { -1 };

    void setSliceDirectory(const QString &sliceDirectory) {
        this->_sliceDirectory = sliceDirectory;
    }

    const QString& sliceDirectory() const {
        return _sliceDirectory;
    }

private:
   QString _sliceDirectory;
};

class PrintJob {

public:

    PrintJob( ) = default;

    PrintJob( PrintJob const* value ) {
        modelFileName   = value->modelFileName;
        modelHash       = value->modelHash;

        vertexCount     = value->vertexCount;
        x               = value->x;
        y               = value->y;
        z               = value->z;
        estimatedVolume = value->estimatedVolume;

        baseSlices      = value->baseSlices;
        bodySlices      = value->bodySlices;
        totalLayerCount = value->totalLayerCount;

        printProfile    = value->printProfile;
    }

    QString                 modelFileName;
    QString                 modelHash;
    QString                 currentImageFile        {     };
    
    size_t                  vertexCount     { };
    Coordinate              x               { };
    Coordinate              y               { };
    Coordinate              z               { };
    double                  estimatedVolume { }; // unit: µL
    //MERGE_TODO to be removed?
    double          exposureTime            { 1.0 }; // unit: s
    double          exposureTimeScaleFactor { 1.0 }; // for first two layers

    SliceInformation        baseSlices;
    SliceInformation        bodySlices;
    int                     firstLayerOffset{ 0 };
    int                     totalLayerCount { };

    PrintProfile*    printProfile    { };

    bool isTiled() {
        if(_bodyManager) {
            return _bodyManager->tiled();
        }

        return false;
    }

    bool isBaseLayer( int const layer ) const {
        return ( baseSlices.startLayer != -1 ) && ( layer <= baseSlices.endLayer );
    }

    QString getLayerDirectory( int const layer ) const {
        if ( isBaseLayer( layer ) ) {
            return baseSlices.sliceDirectory();
        } else {
            return bodySlices.sliceDirectory();
        }
    }

    QString getLayerFileName( int const layer ) const {
        int baseCount = baseSlices.layerCount;
        int bodyCount = bodySlices.layerCount;

        if(!_bodyManager.isNull()) {
            if( layer < baseCount ) {
                return _baseManager->getElementAt( layer );
            } else if ( layer - baseCount < bodyCount ) {
                return _bodyManager->getElementAt( layer - baseCount );
            }
        } else {
            auto sliceDirectory { getLayerDirectory( layer ) };
            auto adjustedLayer  { layer                      };

            if ( ( baseSlices.startLayer != -1 ) && !isBaseLayer( layer ) ) {
                adjustedLayer -= ( baseSlices.endLayer - baseSlices.startLayer + 1 );
                adjustedLayer += bodySlices.startLayer;
            }

            return QString { "%1/%2.png" }.arg( sliceDirectory ).arg( adjustedLayer, 6, 10, DigitZero );
        }
    }

    QString getLayerPath( int const layer ) const {
        return getLayerDirectory(layer) % Slash % getLayerFileName(layer);
    }

    double getTimeForElementAt( int position ) {
        if(_bodyManager && isTiled()) {
            return _bodyManager->getTimeForElementAt( position );
        }

        else return -1.0;
    }

    void setBodyManager(QSharedPointer<OrderManifestManager> manager) {
        _bodyManager.swap(manager);
        bodySlices.isPreSliced = true;
        bodySlices.layerCount = _bodyManager->getSize();

        bodySlices.startLayer = _baseManager.isNull() ? 0 : _baseManager->getSize();
        bodySlices.endLayer = _baseManager.isNull() ? _bodyManager->getSize() : _baseManager->getSize() + _bodyManager->getSize();


        if(_bodyManager->tiled()) {
            bodySlices.layerThickness = _bodyManager->layerThickNessAt(0);
            bodySlices.firstLayerOffset = _bodyManager->firstLayerOffset();
        }

        totalLayerCount = bodySlices.layerCount + baseSlices.layerCount;
    }

    void setBaseManager(QSharedPointer<OrderManifestManager> manager) {

        _baseManager.swap(manager);

        if(!_baseManager.isNull()) {
            baseSlices.isPreSliced = true;
            baseSlices.layerCount = _baseManager->getSize();
            baseSlices.layerThickness = _baseManager->layerThickNessAt(0);
            baseSlices.firstLayerOffset = _baseManager->firstLayerOffset();
            baseSlices.startLayer = 0;
            baseSlices.endLayer = _baseManager->getSize() - 1;
        } else {
            baseSlices.setSliceDirectory( nullptr );
            baseSlices.isPreSliced = false;
            baseSlices.layerCount = 0;
            baseSlices.layerThickness = -1;
            baseSlices.firstLayerOffset = 0;
            baseSlices.startLayer = 0;
            baseSlices.endLayer = 0;
        }
    }

    int tilingCount() {
        if(!_bodyManager.isNull() && isTiled()) {
            return _bodyManager->tilingCount();
        }

        return 0;
    }

private:
    QSharedPointer<OrderManifestManager> _bodyManager {};
    QSharedPointer<OrderManifestManager> _baseManager {};
};

#endif // __PRINTJOB_H__
