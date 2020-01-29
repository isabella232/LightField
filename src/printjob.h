#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include "coordinate.h"
#include "printprofile.h"

class PrintJob {

public:

    PrintJob( ) {
        /*empty*/
    }

    PrintJob( PrintJob const* value ) {
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
        printProfile            = value->printProfile;
    }

    ~PrintJob( ) {
        /*empty*/
    }

    size_t        vertexCount             {     };
    Coordinate    x                       {     };
    Coordinate    y                       {     };
    Coordinate    z                       {     };
    double        estimatedVolume         {     }; // unit: µL

    QString       modelFileName           {     };
    QString       modelHash               {     };
    QString       jobWorkingDirectory     {     };

    int           layerCount              {     };
    int           layerThickness          { 100 }; // unit: µm
    double        exposureTime            { 1.0 }; // unit: s
    double        exposureTimeScaleFactor { 1.0 }; // for first two layers
    int           powerLevel              { static_cast<int>( ProjectorMaxPowerLevel / 2.0 + 0.5 ) }; // range: 0..ProjectorMaxPowerLevel
    double        printSpeed              { PrinterDefaultLowSpeed                                 }; // unit: mm/min; range: 50-200
    PrintProfile* printProfile            {     };

};

#endif // __PRINTJOB_H__
