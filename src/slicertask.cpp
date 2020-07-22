#include <sys/sysinfo.h>
#include <QtCore>
#include "svgrenderer.h"
#include "slicertask.h"

SlicerTask::SlicerTask(QString basePath, bool sliceBase,
    QString bodyPath, bool sliceBody, QObject *parent):
    QObject(parent),
    _basePath(basePath),
    _bodyPath(bodyPath),
    _sliceBase(sliceBase),
    _sliceBody(sliceBody)
{
}

void SlicerTask::run()
{
    debug(QString("+ SlicerTask::run %1\n").arg(_basePath).toUtf8().data());

    bool oneHeight = printJob.getSelectedBaseLayerThickness() == printJob.getSelectedBodyLayerThickness();

    if (oneHeight)
        debug("  + base and body layers are the same height\n");

    try {
        if (printJob.hasBaseLayers() && _sliceBase) {
            /* Need to reslice base layers */
            QDir dir { QDir(_basePath) };
            QString output { QString("%1/sliced.svg").arg(dir.path()) };

            debug("  + must reslice base layers into %s\n", output.toUtf8().data());
            emit sliceStatus("base layers");
            _createDirectory(_basePath);
            _slice(printJob.getModelFilename(), output, printJob.getSelectedBaseLayerThickness());
        }

        if (_sliceBody && !oneHeight) {
            /* Need to reslice body layers */
            QString output { QString("%1/sliced.svg").arg(_bodyPath) };

            debug("  + must reslice body layers into %s\n", output.toUtf8().data());
            emit sliceStatus("body layers");
            _createDirectory(_bodyPath);
            _slice(printJob.getModelFilename(), output, printJob.getSelectedBodyLayerThickness());
        }

        emit sliceStatus("finished");

        if (printJob.hasBaseLayers() && _sliceBase) {
            /* Need to render base layers */

            debug("  + must render base layers\n");
            _render(_basePath, false);
        }

        if (oneHeight) {
            printJob.setBodyManager(printJob.getBaseManager());
        } else {
            if (_sliceBody) {
                /* Need to render body layers */

                 debug("  + must rendes body layers\n");
                _render(_bodyPath, true);
            }
        }

        emit layerCount(printJob.totalLayerCount());
    } catch (const std::exception &ex) {
        debug("  + caught exception: %s\n", ex.what());
        emit done(false);
    }

    debug("  + finished successfully\n");
    emit sliceStatus("idle");
    emit renderStatus("idle");
    emit done(true);
}

int SlicerTask::_numThreads()
{
    return get_nprocs();
}

void SlicerTask::_slice(const QString &input, const QString &output, int layerHeight)
{
    QProcess* process = new QProcess();
    QStringList slicerArgs = {
        input,
        "--export-svg",
        "--threads", QString("%1").arg(_numThreads()),
        "--first-layer-height", QString("%1").arg(layerHeight / 1000.0),
        "--layer-height", QString("%1").arg(layerHeight / 1000.0),
        "--output", output
    };

    process->setProcessChannelMode(QProcess::ForwardedChannels);
    process->start("slic3r", slicerArgs);
    process->waitForFinished(-1);

    if (process->exitCode() != 0)
        throw std::runtime_error("Slicer process crashed");
}

void SlicerTask::_render(const QString &directory, bool isBody)
{
    debug(QString("+ SlicerTask::_render %1\n").arg(directory).toUtf8().data());

    QSharedPointer<OrderManifestManager> manager { new OrderManifestManager };
    QString sliced;
    SvgRenderer renderer;

    if (isBody)
        QObject::connect(&renderer, &SvgRenderer::layerComplete, this, &SlicerTask::_bodyLayerDone);
    else
        QObject::connect(&renderer, &SvgRenderer::layerComplete, this, &SlicerTask::_baseLayerDone);

    sliced = QString("%1/sliced.svg").arg(directory);

    renderer.render(sliced, directory, manager);

    if (isBody)
        printJob.setBodyManager(manager);
    else
        printJob.setBaseManager(manager);
}

void SlicerTask::_createDirectory(const QString &path)
{
    QDir workDir { QDir(path) };

    Q_ASSERT(workDir.path().length() > 0);
    Q_ASSERT(workDir.path().startsWith(JobWorkingDirectoryPath));

    workDir.removeRecursively();
    workDir.mkdir(workDir.path());
}

void SlicerTask::_baseLayerDone(int layer, const QString &path)
{
    emit renderStatus(QString("base layer %1").arg(layer));
    emit layerDone(layer, path);
}

void SlicerTask::_bodyLayerDone(int layer, const QString &path)
{
    emit renderStatus(QString("body layer %1").arg(layer));
    emit layerDone(layer, path);
}

