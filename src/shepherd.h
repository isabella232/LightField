#ifndef __SHEPHERD_H__
#define __SHEPHERD_H__

#include <QProcess>

class Shepherd: public QObject {

    Q_OBJECT

public:

    Shepherd( QObject* parent );
    virtual ~Shepherd( ) override;

    void start( );
    void doMove( float arg );
    void doMoveTo( float arg );
    void doHome( );
    void doLift( float arg1, float arg2 );
    void doAskTemp( );
    void doSend( char const* arg );

    void terminate( );

private slots:

    void processErrorOccurred( QProcess::ProcessError error );
    void processStarted( );
    void processStateChanged( QProcess::ProcessState newState );
    void processReadyRead( );
    void processFinished( int exitCode, QProcess::ExitStatus exitStatus );

signals:

    void shepherd_Started( );
    void shepherd_Finished( int exitCode, QProcess::ExitStatus exitStatus );
    void shepherd_ProcessError( QProcess::ProcessError error );

    void printer_Online( );
    void printer_Offline( );
    void printer_Position( float position );
    void printer_Temperature( char const* temperatureInfo );

    void printProcess_ShowImage( char const* fileName, char const* brightness, char const* index, char const* total );
    void printProcess_HideImage( );
    void printProcess_StartedPrinting( );
    void printProcess_FinishedPrinting( );

private:

    QProcess* _process;
    QString _buffer;

};

#endif // __SHEPHERD_H__
