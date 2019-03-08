#ifndef __SVGRENDERER_H__
#define __SVGRENDERER_H__

class ProcessRunner;

class SvgRenderer: public QObject {

    Q_OBJECT

public:

    SvgRenderer( );
    ~SvgRenderer( );

    void startRender( QString const& svgFileName, QString const& outputDirectory );

protected:

private:

    QString        outputDirectory;
    QDomDocument   doc;
    ProcessRunner* pr;

    int currentLayer;
    int totalLayers { 0 };
    int digits;
    int pxWidth;
    int pxHeight;

    void renderLayer( );

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
