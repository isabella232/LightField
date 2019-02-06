#ifndef __PRINT_MANAGER_H__
#define __PRINT_MANAGER_H__

#include <QProcess>
#include <QTimer>

#include "printjob.h"

class PngDisplayer;
class Shepherd;

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
    PngDisplayer*          _pngDisplayer                   { };

    int                    _currentLayer                   { };

    QProcess*              _setPowerProcess                { };
    bool                   _setPowerProcessConnected_step4 { false };
    bool                   _setPowerProcessConnected_step6 { false };

    QTimer*                _preProjectionTimer             { };
    QTimer*                _layerProjectionTimer           { };
    QTimer*                _preLiftTimer                   { };

    void _cleanUp( );

    void _connectSetPowerProcess_step4( );
    void _disconnectSetPowerProcess_step4( );

    void _connectSetPowerProcess_step6( );
    void _disconnectSetPowerProcess_step6( );

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

    void step3_preProjectionTimerExpired( );

    void step4_setPowerProcessErrorOccurred( QProcess::ProcessError error );
    void step4_setPowerProcessStarted( );
    void step4_setPowerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void step5_layerProjectionTimerExpired( );

    void step6_setPowerProcessErrorOccurred( QProcess::ProcessError error );
    void step6_setPowerProcessStarted( );
    void step6_setPowerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void step7_preLiftTimerExpired( );

    void step8_LiftUpComplete( bool success );

};

#endif // __PRINT_MANAGER_H__
