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
    explicit SlicerTask(QSharedPointer<PrintJob> printJob, const QString &basePath, bool sliceBase,
        const QString &bodyPath, bool sliceBody, QObject *parent = nullptr);
    virtual void run() override;

signals:
    void sliceStatus(const QString &status);
    void renderStatus(const QString &status);
    void layerCount(int count);
    void layerDone(int layer, QString path);
    void done(bool success);

protected:
    int _numThreads();
    void _slice(const QString &input, const QString &output, int layerHeight);
    void _render(const QString &directory, bool isBody);
    void _createDirectory(const QString &path);
    void _baseLayerDone(int layer, const QString &path);
    void _bodyLayerDone(int layer, const QString &path);

    QSharedPointer<PrintJob> _printJob;
    const QString &_basePath;
    const QString &_bodyPath;
    bool _sliceBase;
    bool _sliceBody;
};

#endif // SLICERTASK_H
