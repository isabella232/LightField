#ifndef __SVGRENDERER_H__
#define __SVGRENDERER_H__

#include <QtCore>
#include <QtXml>
#include <Magick++.h>
#include "printjob.h"
#include "debug.h"

class LayerRenderTask;

class SvgRenderer: public QObject
{
    friend class LayerRenderTask;
    Q_OBJECT

public:
    SvgRenderer() = default;
    virtual ~SvgRenderer() = default;

    void render(const QString& svgFileName, const QString& outputDirectory,
        QSharedPointer<OrderManifestManager> orderManager);

protected:
    QString _outputDirectory;
    QDomDocument _doc;
    QThreadPool _threadPool;
    QSharedPointer<OrderManifestManager> _orderManager;

    int                     _currentLayer        { };
    int                     _completedLayers     { };
    int                     _totalLayers         { };
    int                     _digits              { };
    unsigned int                     _pxWidth             { };
    unsigned int                     _pxHeight            { };

    bool                    _isRunning           { };

    void _renderLayer( );

signals:
    void layerCount(int const totalLayers);
    void layerComplete(int const layer, QString path);
};

class LayerRenderTask: public QRunnable
{
public:
    LayerRenderTask(SvgRenderer &renderer, int num, const QString input,
        const QString output):
        _renderer(renderer),
        _layerNumber(num),
        _inputPath(input),
        _outputPath(output)
    {
    }

    virtual void run() override
    {
        Magick::Image image;

        debug("+ processing layer %d\n", _layerNumber);

        /* GraphicsMagick needs normalized locale */
        (void) setlocale(LC_ALL,"");
        (void) setlocale(LC_NUMERIC, "C");
        Magick::InitializeMagick(nullptr);

        image.quiet(false);

        try {
            image.antiAlias(true);
            image.backgroundColor(Magick::Color(0, 0, 0));
            image.density(Magick::Geometry(400, 400));
            image.size(Magick::Geometry(_renderer._pxWidth, _renderer._pxHeight));
            image.read(_inputPath.toStdString());
            image.write(_outputPath.toStdString());
        } catch (const std::exception &ex) {
            debug("+ layer error: %s\n", ex.what());
            return;
        }

        _renderer._completedLayers++;

        emit _renderer.layerComplete(_layerNumber, _outputPath);
        debug("+ completed layer %d\n", _layerNumber);
    }

private:
    SvgRenderer &_renderer;
    int _layerNumber;
    const QString _inputPath;
    const QString _outputPath;
};


#endif // __SVGRENDERER_H__
