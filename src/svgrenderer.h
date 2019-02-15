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
    int layerCount { 0 };
    int digits;
    int pxWidth;
    int pxHeight;

    void renderLayer( );

signals:

    void nextLayer( int const layer );
    void done( int const totalLayers );

public slots:

protected slots:

private slots:

    void programSucceeded( );
    void programFailed( QProcess::ProcessError const error );

};

#endif // __SVGRENDERER_H__
