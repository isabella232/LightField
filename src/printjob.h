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
    double     exposureTimeScaleFactor { 1.0 }; // unit: for first two layers
    int        powerLevel              { 127 }; // unit: 0..255

};

#endif // __PRINTJOB_H__
