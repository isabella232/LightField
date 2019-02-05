#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include <QString>

class PrintJob {

public:

    QString modelFileName;
    QString slicedSvgFileName;
    QString pngFilesPath;
    int layerCount;
    int layerThickness;  // µm
    double exposureTime; // s
    int brightness;      // 0..255

};

#endif // __PRINTJOB_H__
