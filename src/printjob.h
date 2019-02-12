#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

class PrintJob {

public:

    QString modelFileName;
    QString slicedSvgFileName;
    QString pngFilesPath;
    int     layerCount              {     };
    int     layerThickness          { 100 }; // µm
    double  exposureTime            { 1.0 }; // s
    double  exposureTimeScaleFactor { 1.0 }; // for first two layers
    int     powerLevel              { 127 }; // 0..255

};

#endif // __PRINTJOB_H__
