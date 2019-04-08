#ifndef __SVGRENDERER_H__
#define __SVGRENDERER_H__

class ProcessRunner;

class SvgRenderer: public QObject {

    Q_OBJECT

public:

    SvgRenderer( );
    ~SvgRenderer( );

    void startRender( QString const& svgFileName, QString const& _outputDirectory );

protected:

private:

    QString              _outputDirectory;
    QDomDocument         _doc;
    ProcessRunner*       _processRunner  { };

    int                  _currentLayer   { };
    int                  _totalLayers    { };
    int                  _digits         { };
    int                  _pxWidth        { };
    int                  _pxHeight       { };

    void _renderLayer( );

signals:

    void layerCount( int const totalLayers );
    void layerComplete( int const layer );
    void done( bool const success );

public slots:

protected slots:

private slots:

    void programSucceeded( );
    void programFailed( QProcess::ProcessError const error );

};

#endif // __SVGRENDERER_H__
