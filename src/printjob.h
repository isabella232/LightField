#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include "coordinate.h"

class PrintJob {

public:

    size_t     vertexCount;
    Coordinate x;
    Coordinate y;
    Coordinate z;
    double     estimatedVolume;                 // unit: µL

    QString    modelFileName;
    QString    jobWorkingDirectory;

    int        layerCount              {     };
    int        layerThickness          { 100 }; // unit: µm
    double     exposureTime            { 1.0 }; // unit: s
    double     exposureTimeScaleFactor { 1.0 }; // for first two layers
    int        powerLevel              { 128 }; // range: 0..255
    int        printSpeed              { 100 }; // unit: mm/min; range: 50-200

};

#endif // __PRINTJOB_H__
