#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__

#include <QString>

class PrintJob {

public:

    QString modelFileName;
    QString slicedSvgFileName;
    QString pngFilesPath;
    int layerCount;
    double layerThickness;
    int exposureTime;
    int brightness;

};

#endif // __PRINTJOB_H__
