#ifndef __SVGRENDERER_H__
#define __SVGRENDERER_H__

#include <QtCore>
#include <QtXml>
#include <Magick++.h>
#include "printjob.h"
#include "debug.h"

class LayerRenderTask;

class SvgRenderer: public QObject {
    friend class LayerRenderTask;
    Q_OBJECT

public:

    SvgRenderer( );
    ~SvgRenderer( );

    void render(QString const& svgFileName, QString const& _outputDirectory,
        PrintJob* printJob, QSharedPointer<OrderManifestManager> orderManager);
    void loadSlices ( PrintJob* printJob );

protected:

private:

    QString                 _outputDirectory;
    QDomDocument            _doc;

    QVector<int>            _runningLayers;
    QStringList             _layerList;
    QThreadPool             _threadPool;
    QSharedPointer<OrderManifestManager> _orderManager;

    int                     _currentLayer        { };
    int                     _completedLayers     { };
    int                     _totalLayers         { };
    int                     _digits              { };
    unsigned int                     _pxWidth             { };
    unsigned int                     _pxHeight            { };

    bool                    _isRunning           { };

    //MERGE_TODO check if has to be removed
    //std::recursive_mutex    _layerRenderingLock;

    void _renderLayer( );
    void _cleanUpOneProcessRunner( int const slot );
    void _cleanUpProcessRunners( );

signals:
    void layerCount( int const totalLayers );
    void layerComplete( int const layer );
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

        emit _renderer.layerComplete(_layerNumber);
        debug("+ completed layer %d\n", _layerNumber);
    }

private:
    SvgRenderer &_renderer;
    int _layerNumber;
    const QString _inputPath;
    const QString _outputPath;
};


#endif // __SVGRENDERER_H__
