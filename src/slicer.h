#ifndef __SLICER_H__
#define __SLICER_H__

class Slicer: public QObject {

    Q_OBJECT

public:

    Slicer( QString const& fileName, QString const& outputPath, float layerThickness, QObject* parent = 0 );
    virtual ~Slicer( ) override;

    void start( );

protected:

private:

    QProcess* _process;

signals:

    void starting( );
    void progress( int currentFrame, int totalFrames );
    void error( );
    void finished( );

public slots:

protected slots:

private slots:

    void processErrorOccurred( QProcess::ProcessError error );
    void processStarted( );
    void processStateChanged( QProcess::ProcessState newState );
    void processReadyRead( );
    void processFinished( int exitCode, QProcess::ExitStatus exitStatus );

};

#endif // __SLICER_H__
