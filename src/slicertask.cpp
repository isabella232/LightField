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
    try {
        if (_printJob->hasBaseLayers() && (!_printJob->baseSlices.isPreSliced || _reslice)) {
            /* Need to reslice base layers */
            QString output { QString("%1/sliced.svg").arg(_printJob->baseSlices.sliceDirectory) };
            _slice(_printJob->modelFileName, output, _printJob->baseSlices.layerThickness);
        }

        if (!_printJob->bodySlices.isPreSliced || _reslice) {
            /* Need to reslice body layers */
            QString output { QString("%1/sliced.svg").arg(_printJob->bodySlices.sliceDirectory) };
            _slice(_printJob->modelFileName, output, _printJob->bodySlices.layerThickness);
        }

        if (_printJob->hasBaseLayers() && (!_printJob->baseSlices.isPreSliced || _reslice)) {
            /* Need to render base layers */
            _render(_printJob->baseSlices);
        }

        if (!_printJob->bodySlices.isPreSliced || _reslice) {
            /* Need to render body layers */
            _render(_printJob->bodySlices);
        }
    } catch (const std::exception &ex) {

    }
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
    QString sliced { QString("%1/sliced.svg").arg(_printJob->bodySlices.sliceDirectory) };
    QString outputDirectory;
    SvgRenderer renderer;

    switch (slices.type) {
    case SliceType::SliceBase:
        QObject::connect(&renderer, &SvgRenderer::layerCount, this, &SlicerTask::_baseLayerCount);
        QObject::connect(&renderer, &SvgRenderer::layerComplete, this, &SlicerTask::_baseLayerDone);
        _printJob->setBaseManager(manager);
        outputDirectory = _printJob->baseSlices.sliceDirectory;
        break;

    case SliceType::SliceBody:
        QObject::connect(&renderer, &SvgRenderer::layerCount, this, &SlicerTask::_bodyLayerCount);
        QObject::connect(&renderer, &SvgRenderer::layerComplete, this, &SlicerTask::_bodyLayerDone);
        _printJob->setBodyManager(manager);
        outputDirectory = _printJob->bodySlices.sliceDirectory;
        break;
    }

    renderer.render(sliced, outputDirectory, _printJob, manager);
}

void SlicerTask::_baseLayerCount(int count)
{

}

void SlicerTask::_baseLayerDone(int layer)
{

}

void SlicerTask::_bodyLayerCount(int count)
{

}

void SlicerTask::_bodyLayerDone(int layer)
{

}

