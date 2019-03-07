#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include "coordinate.h"

class PrintJob {

public:

    size_t     vertexCount;
    Coordinate x;
    Coordinate y;
    Coordinate z;
    double     estimatedVolume;                 // µL

    QString    modelFileName;
    QString    slicedSvgFileName;
    QString    jobWorkingDirectory;

    int        layerCount              {     };
    int        layerThickness          { 100 }; // µm
    double     exposureTime            { 1.0 }; // s
    double     exposureTimeScaleFactor { 1.0 }; // for first two layers
    int        powerLevel              { 127 }; // 0..255

};

#endif // __PRINTJOB_H__
