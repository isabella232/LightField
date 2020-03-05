#ifndef __SVGRENDERER_H__
#define __SVGRENDERER_H__

#include "ordermanifestmanager.h"

class ProcessRunner;

class SvgRenderer: public QObject {

    Q_OBJECT

public:

    SvgRenderer( );
    ~SvgRenderer( );

    void startRender( QString const& svgFileName, QString const& _outputDirectory );
    void loadSlices ( OrderManifestManager manifestManager );
protected:

private:

    QString                 _outputDirectory;
    QDomDocument            _doc;

    QVector<ProcessRunner*> _processRunners;
    QVector<int>            _runningLayers;
    int                     _processesRunning { };

    int                     _currentLayer     { };
    int                     _completedLayers  { };
    int                     _totalLayers      { };
    int                     _digits           { };
    int                     _pxWidth          { };
    int                     _pxHeight         { };

    bool                    _isRunning        { };

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

};

#endif // __SVGRENDERER_H__
