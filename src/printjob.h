#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include "coordinate.h"
#include "printprofile.h"

class SliceInformation {

public:

    QString sliceDirectory;

    bool    isPreSliced      { };

    int     layerCount       { };
    int     layerThickness   { }; // unit: µm
    int     firstLayerOffset { }; // unit: µm

    int     startLayer       { -1 };
    int     endLayer         { -1 };

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

    QString          modelFileName;
    QString          modelHash;

    size_t           vertexCount     { };
    Coordinate       x               { };
    Coordinate       y               { };
    Coordinate       z               { };
    double           estimatedVolume { }; // unit: µL

    SliceInformation baseSlices;
    SliceInformation bodySlices;

    int              totalLayerCount { };

    PrintProfile*    printProfile    { };

    bool isBaseLayer( int const layer ) const {
        return ( baseSlices.startLayer != -1 ) && ( layer <= baseSlices.endLayer );
    }

    QString getLayerDirectory( int const layer ) const {
        if ( isBaseLayer( layer ) ) {
            return baseSlices.sliceDirectory;
        } else {
            return bodySlices.sliceDirectory;
        }
    }

    QString getLayerFileName( int const layer ) const {
        auto sliceDirectory { getLayerDirectory( layer ) };
        auto adjustedLayer  { layer                      };

        if ( ( baseSlices.startLayer != -1 ) && !isBaseLayer( layer ) ) {
            adjustedLayer -= ( baseSlices.endLayer - baseSlices.startLayer + 1 );
            adjustedLayer += bodySlices.startLayer;
        }

        return QString { "%1/%2.png" }.arg( sliceDirectory ).arg( adjustedLayer, 6, 10, DigitZero );
    }

};

#endif // __PRINTJOB_H__
