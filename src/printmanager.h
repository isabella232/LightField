#ifndef __PRINT_MANAGER_H__
#define __PRINT_MANAGER_H__

#include <QProcess>
#include <QTimer>

#include "printjob.h"

class Shepherd;
class QProcess;

class PrintManager: public QObject {

    Q_OBJECT

public:

    PrintManager( Shepherd* shepherd, QObject* parent = 0 );
    virtual ~PrintManager( ) override;

    void print( PrintJob* printJob );

protected:

private:

    Shepherd*              _shepherd                       { };
    PrintJob*              _printJob                       { };

    int                    _currentLayer                   { };

    QProcess*              _fehProcess                     { };
    bool                   _fehProcessConnected            { false };
    QProcess::ProcessState _fehProcessState                { QProcess::NotRunning };

    QProcess*              _setPowerProcess                { };
    bool                   _setPowerProcessConnected_step5 { false };
    bool                   _setPowerProcessConnected_step7 { false };
    QProcess::ProcessState _setPowerProcessState           { QProcess::NotRunning };

    QTimer*                _preProjectionTimer             { };
    QTimer*                _layerProjectionTimer           { };
    QTimer*                _preLiftTimer                   { };

    void _cleanUp( );

    void _connectFehProcess( );
    void _disconnectFehProcess( );

    void _connectSetPowerProcess_step5( );
    void _disconnectSetPowerProcess_step5( );

    void _connectSetPowerProcess_step7( );
    void _disconnectSetPowerProcess_step7( );

    void startNextLayer( );

signals:

    void printComplete( bool success );
    void startingLayer( int layer );

public slots:

protected slots:

private slots:

    void initialHomeComplete( bool success );

    void step1_LiftUpComplete( bool success );

    void step2_LiftDownComplete( bool success );

    void step3_fehProcessErrorOccurred( QProcess::ProcessError error );
    void step3_fehProcessStarted( );
    void step3_fehProcessStateChanged( QProcess::ProcessState newState );
    void step3_fehProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void step4_timerExpired( );

    void step5_setPowerProcessErrorOccurred( QProcess::ProcessError error );
    void step5_setPowerProcessStarted( );
    void step5_setPowerProcessStateChanged( QProcess::ProcessState newState );
    void step5_setPowerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void step6_timerExpired( );

    void step7_setPowerProcessErrorOccurred( QProcess::ProcessError error );
    void step7_setPowerProcessStarted( );
    void step7_setPowerProcessStateChanged( QProcess::ProcessState newState );
    void step7_setPowerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void step8_fehProcessTerminated( );

    void step9_timerExpired( );

};

#endif // __PRINT_MANAGER_H__
