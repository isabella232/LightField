#ifndef __SVGRENDERER_H__
#define __SVGRENDERER_H__

#include "printjob.h"

class ProcessRunner;

class SvgRenderer: public QObject {

    Q_OBJECT

public:

    SvgRenderer( );
    ~SvgRenderer( );

    void startRender( QString const& svgFileName, QString const& _outputDirectory, PrintJob* printJob );
    void loadSlices ( PrintJob* printJob );

protected:

private:

    QString                 _outputDirectory;
    QDomDocument            _doc;

    QVector<ProcessRunner*> _processRunners;
    QVector<int>            _runningLayers;
    QStringList             _layerList;
    int                     _processesRunning { };


    int                     _currentLayer        { };
    int                     _completedLayers     { };
    int                     _totalLayers         { };
    int                     _digits              { };
    int                     _pxWidth             { };
    int                     _pxHeight            { };

    bool                    _isRunning           { };

    //MERGE_TODO check if has to be removed
    //std::recursive_mutex    _layerRenderingLock;

    void _renderLayer( );
    void _cleanUpOneProcessRunner( int const slot );
    void _cleanUpProcessRunners( );

signals:
    ;

    void layerCount( int const totalLayers );
    void layerComplete( int const layer );
    void done( bool const success );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

//    void renderLayerProcess_succeeded( int const slot );
//    void renderLayerProcess_failed( int const slot, int const exitCode, QProcess::ProcessError const error );

};

#endif // __SVGRENDERER_H__
