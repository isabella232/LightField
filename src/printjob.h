#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

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

    void setSliceDirectory(QString sliceDirectory) {
        this->_sliceDirectory = sliceDirectory;
    }

    QString sliceDirectory() const {
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
        if(bodyManager) {
            return bodyManager->tiled();
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

        if(bodyManager != nullptr) {
            if( layer < baseCount ) {
                return baseManager->getElementAt( layer );
            } else if ( layer - baseCount < bodyCount ) {
                return bodyManager->getElementAt( layer - baseCount );
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
        if(bodyManager && isTiled()) {
            return bodyManager->getTimeForElementAt( position );
        }

        else return -1.0;
    }

    void setBodyManager(OrderManifestManager* bodyManager) {
        if(bodyManager != nullptr)
            delete bodyManager;

        bodySlices.setSliceDirectory( bodyManager->path() );
        bodySlices.isPreSliced = true;
        bodySlices.layerCount = bodyManager->getSize();

        bodySlices.startLayer = baseManager == nullptr ? 0 : baseManager->getSize();
        bodySlices.endLayer = baseManager == nullptr ? bodyManager->getSize() : baseManager->getSize() + bodyManager->getSize();


        if(bodyManager->tiled()) {
            bodySlices.layerThickness = bodyManager->layerThickNessAt(0);
            bodySlices.firstLayerOffset = bodyManager->firstLayerOffset();
        }

        this->bodyManager = bodyManager;
        totalLayerCount = bodySlices.layerCount + baseSlices.layerCount;
    }

    void setBaseManager(OrderManifestManager* baseManager) {
        if(baseManager)
            delete bodyManager;

        if(baseManager != nullptr)
            this->baseManager = baseManager;
        else
            baseManager = nullptr;

        if(baseManager != nullptr) {
            baseSlices.setSliceDirectory( baseManager->path() );
            baseSlices.isPreSliced = true;
            baseSlices.layerCount = baseManager->getSize();
            baseSlices.layerThickness = baseManager->layerThickNessAt(0);
            baseSlices.firstLayerOffset = baseManager->firstLayerOffset();
            baseSlices.startLayer = 0;
            baseSlices.endLayer = baseManager->getSize() - 1;
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
        if(bodyManager!=nullptr && isTiled()) {
            return bodyManager->tilingCount();
        }

        return 0;
    }

private:

    OrderManifestManager* bodyManager = nullptr;
    OrderManifestManager* baseManager = nullptr;
};

#endif // __PRINTJOB_H__
