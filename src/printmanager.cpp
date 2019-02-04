#include "printmanager.h"
#include "printmanager.h"
#include <QProcess>

#include "printmanager.h"
#include "shepherd.h"
#include "strings.h"

//
// For each layer:
//
// ✓ 1. Lift up
// ✓ 2. Lift down
// ✓ 3. Start feh
// ✓ 4. Pause before projection
// ✓ 5. Start projecting: setpower ${brightness}
// ✓ 6. Pause for layer time
// ✓ 7. Stop projection: setpower 0
// ✓ 8. Stop feh
// ✓ 9. Pause before lift
//

/*
Bad command startup
===================
+ ProcWin::processStateChanged: new state Starting [1]
+ ProcWin::processStateChanged: new state NotRunning [0]
+ ProcWin::processErrorOccurred: error FailedToStart [0]

When a process exits normally
=============================
+ ProcWin::processStateChanged: new state Starting [1]
+ ProcWin::processStateChanged: new state Running [2]
+ ProcWin::processStarted
[time passes]
+ ProcWin::processStateChanged: new state NotRunning [0]
+ ProcWin::processFinished: exitCode: 0, exitStatus: NormalExit [0]

When a process exits abnormally
===============================
+ ProcWin::processStateChanged: new state Starting [1]
+ ProcWin::processStateChanged: new state Running [2]
+ ProcWin::processStarted
[time passes]
+ ProcWin::processErrorOccurred: error Crashed [1]
+ ProcWin::processStateChanged: new state NotRunning [0]
+ ProcWin::processFinished: exitCode: 15, exitStatus: CrashExit [1]
*/

namespace {

    auto PauseBeforeProject = 1000;
    auto PauseBeforeLift    = 1000;
    auto LiftDistance       = 2.0;

    auto FehCommand         = QString { "feh"      };
    auto SetPowerCommand    = QString { "setpower" };

}

PrintManager::PrintManager( Shepherd* shepherd, QObject* parent ):
    QObject   ( parent   ),
    _shepherd ( shepherd )
{
    /*empty*/
}

PrintManager::~PrintManager( ) {
    if ( _fehProcess->state( ) != QProcess::NotRunning ) {
        _fehProcess->terminate( );
    }
}

void PrintManager::_cleanUp( ) {
    if ( _printJob ) {
        delete _printJob;
        _printJob = nullptr;
    }

    if ( _fehProcessConnected ) {
        _disconnectFehProcess( );
    }

    if ( _fehProcess ) {
        delete _fehProcess;
        _fehProcess = nullptr;
    }

    if ( _preProjectionTimer ) {
        _preProjectionTimer->stop( );
        delete _preProjectionTimer;
        _preProjectionTimer = nullptr;
    }

    if ( _setPowerProcessConnected_step5 ) {
        _disconnectSetPowerProcess_step5( );
    }

    if ( _setPowerProcessConnected_step7 ) {
        _disconnectSetPowerProcess_step7( );
    }

    if ( _setPowerProcess ) {
        delete _setPowerProcess;
        _setPowerProcess = nullptr;
    }

    if ( _preLiftTimer ) {
        _preLiftTimer->stop( );
        delete _preLiftTimer;
        _preLiftTimer = nullptr;
    }
}

void PrintManager::_connectFehProcess( ) {
    QObject::connect( _fehProcess, &QProcess::errorOccurred, this, &PrintManager::step3_fehProcessErrorOccurred );
    QObject::connect( _fehProcess, &QProcess::started,       this, &PrintManager::step3_fehProcessStarted       );
    QObject::connect( _fehProcess, &QProcess::stateChanged,  this, &PrintManager::step3_fehProcessStateChanged  );
    QObject::connect( _fehProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step3_fehProcessFinished );
    _fehProcessConnected = true;
}

void PrintManager::_disconnectFehProcess( ) {
    QObject::disconnect( _fehProcess, &QProcess::errorOccurred, this, &PrintManager::step3_fehProcessErrorOccurred );
    QObject::disconnect( _fehProcess, &QProcess::started,       this, &PrintManager::step3_fehProcessStarted       );
    QObject::disconnect( _fehProcess, &QProcess::stateChanged,  this, &PrintManager::step3_fehProcessStateChanged  );
    QObject::disconnect( _fehProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step3_fehProcessFinished );
    _fehProcessConnected = false;
}

void PrintManager::_connectSetPowerProcess_step5( ) {
    QObject::connect( _setPowerProcess, &QProcess::errorOccurred, this, &PrintManager::step5_setPowerProcessErrorOccurred );
    QObject::connect( _setPowerProcess, &QProcess::started,       this, &PrintManager::step5_setPowerProcessStarted       );
    QObject::connect( _setPowerProcess, &QProcess::stateChanged,  this, &PrintManager::step5_setPowerProcessStateChanged  );
    QObject::connect( _setPowerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step5_setPowerProcessFinished );
    _setPowerProcessConnected_step5 = true;
}

void PrintManager::_disconnectSetPowerProcess_step5( ) {
    QObject::disconnect( _setPowerProcess, &QProcess::errorOccurred, this, &PrintManager::step5_setPowerProcessErrorOccurred );
    QObject::disconnect( _setPowerProcess, &QProcess::started,       this, &PrintManager::step5_setPowerProcessStarted       );
    QObject::disconnect( _setPowerProcess, &QProcess::stateChanged,  this, &PrintManager::step5_setPowerProcessStateChanged  );
    QObject::disconnect( _setPowerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step5_setPowerProcessFinished );
    _setPowerProcessConnected_step5 = false;
}

void PrintManager::_connectSetPowerProcess_step7( ) {
    QObject::connect( _setPowerProcess, &QProcess::errorOccurred, this, &PrintManager::step7_setPowerProcessErrorOccurred );
    QObject::connect( _setPowerProcess, &QProcess::started,       this, &PrintManager::step7_setPowerProcessStarted       );
    QObject::connect( _setPowerProcess, &QProcess::stateChanged,  this, &PrintManager::step7_setPowerProcessStateChanged  );
    QObject::connect( _setPowerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step7_setPowerProcessFinished );
    _setPowerProcessConnected_step7 = true;
}

void PrintManager::_disconnectSetPowerProcess_step7( ) {
    QObject::disconnect( _setPowerProcess, &QProcess::errorOccurred, this, &PrintManager::step7_setPowerProcessErrorOccurred );
    QObject::disconnect( _setPowerProcess, &QProcess::started,       this, &PrintManager::step7_setPowerProcessStarted       );
    QObject::disconnect( _setPowerProcess, &QProcess::stateChanged,  this, &PrintManager::step7_setPowerProcessStateChanged  );
    QObject::disconnect( _setPowerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step7_setPowerProcessFinished );
    _setPowerProcessConnected_step7 = false;
}

void PrintManager::print( PrintJob* printJob ) {
    if ( _printJob ) {
        fprintf( stderr, "+ PrintManager::print: Job submitted while we're busy\n" );
        return;
    }
    _printJob = printJob;

    QObject::connect( _shepherd, &Shepherd::action_homeComplete, this, &PrintManager::initialHomeComplete );
    _shepherd->doHome( );
}

void PrintManager::initialHomeComplete( bool success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_homeComplete, this, &PrintManager::initialHomeComplete );

    if ( !success ) {
        fprintf( stderr, "+ PrintManager::initialHomeComplete: action failed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
    fprintf( stderr, "+ PrintManager::initialHomeComplete: action succeeded\n" );

    startNextLayer( );
}

void PrintManager::startNextLayer( ) {
    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step1_LiftUpComplete );
    _shepherd->doMove( LiftDistance );
    emit startingLayer( _currentLayer );
}

void PrintManager::step1_LiftUpComplete( bool success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step1_LiftUpComplete );

    if ( !success ) {
        fprintf( stderr, "+ PrintManager::step1_LiftUpComplete: action failed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
    fprintf( stderr, "+ PrintManager::step1_LiftUpComplete: action succeeded\n" );

    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step2_LiftDownComplete );
    _shepherd->doMove( -LiftDistance + _printJob->layerThickness );
}

void PrintManager::step2_LiftDownComplete( bool success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step2_LiftDownComplete );

    if ( !success ) {
        fprintf( stderr, "+ PrintManager::step2_LiftDownComplete: action failed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
    fprintf( stderr, "+ PrintManager::step2_LiftDownComplete: action succeeded\n" );

    _fehProcess = new QProcess( this );
    _fehProcess->setProgram( FehCommand );
    _fehProcess->setArguments( QStringList {
        "-x",
        "--geometry",
        "1280x800+800+0",
        QString( _printJob->pngFilesPath + QString( "/%1.png" ) ).arg( _currentLayer, 3, QChar( '0' ) )
    } );
    _connectFehProcess( );
    _fehProcess->start( );
}

void PrintManager::step3_fehProcessErrorOccurred( QProcess::ProcessError error ) {
    _disconnectFehProcess( );

    fprintf( stderr, "+ PrintManager::step3_fehProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        fprintf( stderr, "  + feh process failed to start\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
}

void PrintManager::step3_fehProcessStarted( ) {
    fprintf( stderr, "+ PrintManager::step3_fehProcessStarted\n" );

    _preProjectionTimer = new QTimer( this );
    _preProjectionTimer->setInterval( PauseBeforeProject );
    _preProjectionTimer->setSingleShot( true );
    _preProjectionTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _preProjectionTimer, &QTimer::timeout, this, &PrintManager::step4_timerExpired );
}

void PrintManager::step3_fehProcessStateChanged( QProcess::ProcessState newState ) {
    fprintf( stderr, "+ PrintManager::step3_fehProcessStateChanged: new state %s [%d]\n", ToString( newState ), newState );
    _fehProcessState = _fehProcess->state( );
}

void PrintManager::step3_fehProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    _disconnectFehProcess( );

    fprintf( stderr, "+ PrintManager::step3_fehProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    delete _fehProcess;
    _fehProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        fprintf( stderr, "  + feh process crashed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    } else {
        step8_fehProcessTerminated( );
    }
}

void PrintManager::step4_timerExpired( ) {
    QObject::disconnect( _preProjectionTimer, &QTimer::timeout, this, &PrintManager::step4_timerExpired );

    fprintf( stderr, "+ PrintManager::step4_timerExpired\n" );

    delete _preProjectionTimer;

    _setPowerProcess = new QProcess( this );
    _setPowerProcess->setProgram( SetPowerCommand );
    _setPowerProcess->setArguments( QStringList {
        QString( "%1" ).arg( _printJob->brightness )
    } );
    _connectSetPowerProcess_step5( );
    _setPowerProcess->start( );
}

void PrintManager::step5_setPowerProcessErrorOccurred( QProcess::ProcessError error ) {
    _disconnectSetPowerProcess_step5( );

    fprintf( stderr, "+ PrintManager::step5_setPowerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        fprintf( stderr, "  + setpower process failed to start\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
}

void PrintManager::step5_setPowerProcessStarted( ) {
    fprintf( stderr, "+ PrintManager::step5_setPowerProcessStarted\n" );
}

void PrintManager::step5_setPowerProcessStateChanged( QProcess::ProcessState newState ) {
    fprintf( stderr, "+ PrintManager::step5_setPowerProcessStateChanged: new state %s [%d]\n", ToString( newState ), newState );
    _setPowerProcessState = _setPowerProcess->state( );
}

void PrintManager::step5_setPowerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    _disconnectSetPowerProcess_step5( );

    fprintf( stderr, "+ PrintManager::step5_setPowerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    delete _setPowerProcess;
    _setPowerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        fprintf( stderr, "  + setpower process crashed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }

    _layerProjectionTimer = new QTimer( this );
    _layerProjectionTimer->setInterval( _printJob->exposureTime );
    _layerProjectionTimer->setSingleShot( true );
    _layerProjectionTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step6_timerExpired );
}

void PrintManager::step6_timerExpired( ) {
    QObject::disconnect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step6_timerExpired );

    fprintf( stderr, "+ PrintManager::step6_timerExpired\n" );

    delete _layerProjectionTimer;

    _setPowerProcess = new QProcess( this );
    _setPowerProcess->setProgram( SetPowerCommand );
    _setPowerProcess->setArguments( QStringList { "0" } );
    _connectSetPowerProcess_step5( );
    _setPowerProcess->start( );
}

void PrintManager::step7_setPowerProcessErrorOccurred( QProcess::ProcessError error ) {
    _disconnectSetPowerProcess_step7( );

    fprintf( stderr, "+ PrintManager::step7_setPowerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        fprintf( stderr, "  + setpower process failed to start\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
}

void PrintManager::step7_setPowerProcessStarted( ) {
    fprintf( stderr, "+ PrintManager::step7_setPowerProcessStarted\n" );
}

void PrintManager::step7_setPowerProcessStateChanged( QProcess::ProcessState newState ) {
    fprintf( stderr, "+ PrintManager::step7_setPowerProcessStateChanged: new state %s [%d]\n", ToString( newState ), newState );
    _setPowerProcessState = _setPowerProcess->state( );
}

void PrintManager::step7_setPowerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    _disconnectSetPowerProcess_step7( );

    fprintf( stderr, "+ PrintManager::step7_setPowerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    delete _setPowerProcess;
    _setPowerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        fprintf( stderr, "  + setpower process crashed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }

    _fehProcess->terminate( );
}

void PrintManager::step8_fehProcessTerminated( ) {
    _disconnectFehProcess( );

    delete _fehProcess;
    _fehProcess = nullptr;

    ++_currentLayer;
    if ( _currentLayer == _printJob->layerCount ) {
        _cleanUp( );
        emit printComplete( true );
        return;
    }

    _preLiftTimer = new QTimer( this );
    _preLiftTimer->setInterval( PauseBeforeLift );
    _preLiftTimer->setSingleShot( true );
    _preLiftTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _preLiftTimer, &QTimer::timeout, this, &PrintManager::step9_timerExpired );
}

void PrintManager::step9_timerExpired( ) {
    QObject::disconnect( _preLiftTimer, &QTimer::timeout, this, &PrintManager::step9_timerExpired );

    delete _preLiftTimer;

    startNextLayer( );
}
