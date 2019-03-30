#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include "coordinate.h"

class PrintJob {

public:

    PrintJob( ) {
        debug( "+ PrintJob::`ctor1: %p\n", this );
    }

    PrintJob( PrintJob const* value ) {
        debug( "+ PrintJob::`ctor2: %p\n", this );

        vertexCount             = value->vertexCount;
        x                       = value->x;
        y                       = value->y;
        z                       = value->z;
        estimatedVolume         = value->estimatedVolume;

        modelFileName           = value->modelFileName;
        modelHash               = value->modelHash;
        jobWorkingDirectory     = value->jobWorkingDirectory;

        layerCount              = value->layerCount;
        layerThickness          = value->layerThickness;
        exposureTime            = value->exposureTime;
        exposureTimeScaleFactor = value->exposureTimeScaleFactor;
        powerLevel              = value->powerLevel;
        printSpeed              = value->printSpeed;
    }

    ~PrintJob( ) {
        debug( "+ PrintJob::`dtor: %p\n", this );
    }

    size_t     vertexCount             {     };
    Coordinate x                       {     };
    Coordinate y                       {     };
    Coordinate z                       {     };
    double     estimatedVolume         {     }; // unit: µL

    QString    modelFileName           {     };
    QString    modelHash               {     };
    QString    jobWorkingDirectory     {     };

    int        layerCount              {     };
    int        layerThickness          { 100 }; // unit: µm
    double     exposureTime            { 1.0 }; // unit: s
    double     exposureTimeScaleFactor { 1.0 }; // for first two layers
    int        powerLevel              { 128 }; // range: 0..255
    int        printSpeed              { 100 }; // unit: mm/min; range: 50-200

};

#endif // __PRINTJOB_H__
