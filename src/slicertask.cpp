#include <QtCore>
#include "svgrenderer.h"
#include "slicertask.h"

SlicerTask::SlicerTask(PrintJob *printJob, bool reslice, QObject *parent):
    QObject(parent),
    _printJob(printJob),
    _reslice(reslice)
{
}

void SlicerTask::run()
{
    debug("+ SlicerTask::run\n");

    try {
        if (_printJob->hasBaseLayers() && (!_printJob->baseSlices.isPreSliced || _reslice)) {
            /* Need to reslice base layers */
            QDir dir { QDir(_printJob->baseSlices.sliceDirectory) };
            QString output { QString("%1/sliced.svg").arg(dir.path()) };

            debug("  + must reslice base layers into %s\n", output.toUtf8().data());
            emit sliceStatus("Slicing base layers");
            _createDirectory(_printJob->baseSlices);
            _slice(_printJob->modelFileName, output, _printJob->baseSlices.layerThickness);
        }

        if (!_printJob->bodySlices.isPreSliced || _reslice) {
            /* Need to reslice body layers */
            QString output { QString("%1/sliced.svg").arg(_printJob->bodySlices.sliceDirectory) };

            debug("  + must reslice body layers into %s\n", output.toUtf8().data());
            emit sliceStatus("Slicing body layers");
            _createDirectory(_printJob->bodySlices);
            _slice(_printJob->modelFileName, output, _printJob->bodySlices.layerThickness);
        }

        emit sliceStatus("Slicing finished");

        if (_printJob->hasBaseLayers() && (!_printJob->baseSlices.isPreSliced || _reslice)) {
            /* Need to render base layers */

            debug("  + must render base layers\n");
            _render(_printJob->baseSlices);
        }

        if (!_printJob->bodySlices.isPreSliced || _reslice) {
            /* Need to render body layers */

             debug("  + must rendes body layers\n");
            _render(_printJob->bodySlices);
        }
    } catch (const std::exception &ex) {
        debug("  + caught exception: %s\n", ex.what());
        emit done(false);
    }

    debug("  + finished successfully\n");
    emit sliceStatus("Idle");
    emit renderStatus("Idle");
    emit done(true);
}

int SlicerTask::_numThreads()
{
    return 4;
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

void SlicerTask::_render(const SliceInformation &slices)
{
    QSharedPointer<OrderManifestManager> manager { new OrderManifestManager };
    QString sliced;
    QString outputDirectory;
    SvgRenderer renderer;

    switch (slices.type) {
    case SliceType::SliceBase:
        QObject::connect(&renderer, &SvgRenderer::layerCount, this, &SlicerTask::_baseLayerCount);
        QObject::connect(&renderer, &SvgRenderer::layerComplete, this, &SlicerTask::_baseLayerDone);
        sliced = QString("%1/sliced.svg").arg(_printJob->baseSlices.sliceDirectory);
        outputDirectory = _printJob->baseSlices.sliceDirectory;
        break;

    case SliceType::SliceBody:
        QObject::connect(&renderer, &SvgRenderer::layerCount, this, &SlicerTask::_bodyLayerCount);
        QObject::connect(&renderer, &SvgRenderer::layerComplete, this, &SlicerTask::_bodyLayerDone);
        sliced = QString("%1/sliced.svg").arg(_printJob->bodySlices.sliceDirectory);
        outputDirectory = _printJob->bodySlices.sliceDirectory;
        break;
    }

    renderer.render(sliced, outputDirectory, _printJob, manager);

    switch (slices.type) {
    case SliceType::SliceBase:
        _printJob->setBaseManager(manager);
        break;
    case SliceType::SliceBody:
        _printJob->setBodyManager(manager);
        break;
    }
}

void SlicerTask::_createDirectory(const SliceInformation &slices)
{
    QDir workDir { QDir(slices.sliceDirectory) };

    Q_ASSERT(workDir.path().length() > 0);
    Q_ASSERT(workDir.path().startsWith(JobWorkingDirectoryPath));

    workDir.removeRecursively();
    workDir.mkdir(workDir.path());
}

void SlicerTask::_baseLayerCount(int count)
{
    _printJob->baseSlices.layerCount = std::min(_printJob->baseSlices.layerCount, count);
}

void SlicerTask::_baseLayerDone(int layer)
{

}

void SlicerTask::_bodyLayerCount(int count)
{
    _printJob->bodySlices.layerCount = count - _printJob->bodyLayerStart();
    emit layerCount(_printJob->totalLayerCount());
}

void SlicerTask::_bodyLayerDone(int layer)
{

}

