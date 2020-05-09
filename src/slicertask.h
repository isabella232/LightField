#ifndef SLICERTASK_H
#define SLICERTASK_H

#include <QtCore>
#include "printjob.h"

class Slicer
{

};

class SlicerTask: public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit SlicerTask(PrintJob *printJob, bool reslice, QObject *parent = nullptr);
    virtual void run() override;

signals:
    void sliceStatus(const QString &status);
    void renderStatus(const QString &status);
    void layerCount(int count);
    void layerDone(int layer);
    void done(bool success);

protected:
    int _numThreads();
    void _slice(const QString &input, const QString &output, int layerHeight);
    void _render(const SliceInformation &slices);
    void _baseLayerCount(int count);
    void _baseLayerDone(int layer);
    void _bodyLayerCount(int count);
    void _bodyLayerDone(int layer);

    PrintJob* _printJob;
    bool _reslice;
};

#endif // SLICERTASK_H
