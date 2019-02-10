#include "pch.h"

#include "printmanager.h"

#include "pngdisplayer.h"
#include "shepherd.h"
#include "strings.h"

//
// For each layer:
//
// ✓ 1. Lift up
// ✓ 2. Lift down
// ✓ 3. Pause before projection
// ✓ 4. Start projecting: setpower ${powerLevel}
// ✓ 5. Pause for layer time
// ✓ 6. Stop projection: setpower 0
// ✓ 7. Pause before lift
// ✓ 8. Lift up after final layer complete
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

    auto SetPowerCommand    = QString { "setpower" };

}

PrintManager::PrintManager( Shepherd* shepherd, QObject* parent ):
    QObject   ( parent   ),
    _shepherd ( shepherd )
{
    fprintf( stderr, "+ construct PrintManager at %p\n", this );
}

PrintManager::~PrintManager( ) {
    fprintf( stderr, "+ destruct PrintManager at %p\n", this );
}

void PrintManager::_cleanUp( ) {
    if ( _printJob ) {
        delete _printJob;
        _printJob = nullptr;
    }

    if ( _pngDisplayer ) {
        _pngDisplayer->close( );
        delete _pngDisplayer;
        _pngDisplayer = nullptr;
    }

    if ( _preProjectionTimer ) {
        _preProjectionTimer->stop( );
        delete _preProjectionTimer;
        _preProjectionTimer = nullptr;
    }

    if ( _setPowerProcessConnected_step4 ) {
        _disconnectSetPowerProcess_step4( );
    }

    if ( _setPowerProcessConnected_step6 ) {
        _disconnectSetPowerProcess_step6( );
    }

    if ( _setPowerProcess ) {
        if ( _setPowerProcess->state( ) != QProcess::NotRunning ) {
            _setPowerProcess->kill( );
        }
        delete _setPowerProcess;
        _setPowerProcess = nullptr;
    }

    if ( _preLiftTimer ) {
        _preLiftTimer->stop( );
        delete _preLiftTimer;
        _preLiftTimer = nullptr;
    }
}

void PrintManager::_connectSetPowerProcess_step4( ) {
    QObject::connect( _setPowerProcess, &QProcess::errorOccurred, this, &PrintManager::step4_setPowerProcessErrorOccurred );
    QObject::connect( _setPowerProcess, &QProcess::started,       this, &PrintManager::step4_setPowerProcessStarted       );
    QObject::connect( _setPowerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step4_setPowerProcessFinished );
    _setPowerProcessConnected_step4 = true;
}

void PrintManager::_disconnectSetPowerProcess_step4( ) {
    QObject::disconnect( _setPowerProcess, &QProcess::errorOccurred, this, &PrintManager::step4_setPowerProcessErrorOccurred );
    QObject::disconnect( _setPowerProcess, &QProcess::started,       this, &PrintManager::step4_setPowerProcessStarted       );
    QObject::disconnect( _setPowerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step4_setPowerProcessFinished );
    _setPowerProcessConnected_step4 = false;
}

void PrintManager::_connectSetPowerProcess_step6( ) {
    QObject::connect( _setPowerProcess, &QProcess::errorOccurred, this, &PrintManager::step6_setPowerProcessErrorOccurred );
    QObject::connect( _setPowerProcess, &QProcess::started,       this, &PrintManager::step6_setPowerProcessStarted       );
    QObject::connect( _setPowerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step6_setPowerProcessFinished );
    _setPowerProcessConnected_step6 = true;
}

void PrintManager::_disconnectSetPowerProcess_step6( ) {
    QObject::disconnect( _setPowerProcess, &QProcess::errorOccurred, this, &PrintManager::step6_setPowerProcessErrorOccurred );
    QObject::disconnect( _setPowerProcess, &QProcess::started,       this, &PrintManager::step6_setPowerProcessStarted       );
    QObject::disconnect( _setPowerProcess, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &PrintManager::step6_setPowerProcessFinished );
    _setPowerProcessConnected_step6 = false;
}

void PrintManager::print( PrintJob* printJob ) {
    if ( _printJob ) {
        fprintf( stderr, "+ PrintManager::print: Job submitted while we're busy\n" );
        return;
    }
    fprintf( stderr, "+ PrintManager::print: new job\n" );
    _printJob = printJob;

    _pngDisplayer = new PngDisplayer( );
    _pngDisplayer->move( { 0, 0 } );
    _pngDisplayer->resize( { 1280, 800 } );
    _pngDisplayer->showFullScreen( );

    emit printStarting( );
    QObject::connect( _shepherd, &Shepherd::action_homeComplete, this, &PrintManager::initialHomeComplete );
    _shepherd->doHome( );
}

void PrintManager::terminate( ) {
    fprintf( stderr, "+ PrintManager::terminate\n" );
    _cleanUp( );
}

void PrintManager::abortJob( ) {
    fprintf( stderr, "+ PrintManager::abortJob\n" );
    // TODO abort command if possible
    // TODO home printer
    _cleanUp( );
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

    _startNextLayer( );
}

void PrintManager::_startNextLayer( ) {
    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step1_LiftUpComplete );
    fprintf( stderr, "+ PrintManager::_startNextLayer: moving %f\n", LiftDistance );
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
    fprintf( stderr, "+ PrintManager::step1_LiftUpComplete: moving %f\n", -LiftDistance + _printJob->layerThickness / 1000.0 );
    _shepherd->doMove( -LiftDistance + _printJob->layerThickness / 1000.0 );
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

    QString pngFileName = _printJob->pngFilesPath + QString( "/%1.png" ).arg( _currentLayer, 6, 10, QChar( '0' ) );
    if ( !_pngDisplayer->load( pngFileName ) ) {
        fprintf( stderr, "+ PrintManager::step2_LiftDownComplete: PngDisplayer::load failed for file %s\n", pngFileName.toUtf8( ).data( ) );
    }

    _preProjectionTimer = new QTimer( this );
    _preProjectionTimer->setInterval( PauseBeforeProject );
    _preProjectionTimer->setSingleShot( true );
    _preProjectionTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _preProjectionTimer, &QTimer::timeout, this, &PrintManager::step3_preProjectionTimerExpired );
    _preProjectionTimer->start( );
}

void PrintManager::step3_preProjectionTimerExpired( ) {
    QObject::disconnect( _preProjectionTimer, &QTimer::timeout, this, &PrintManager::step3_preProjectionTimerExpired );

    fprintf( stderr, "+ PrintManager::step3_preProjectionTimerExpired\n" );

    delete _preProjectionTimer;
    _preProjectionTimer = nullptr;

    _setPowerProcess = new QProcess( this );
    _setPowerProcess->setProgram( SetPowerCommand );
    _setPowerProcess->setArguments( QStringList {
        QString( "%1" ).arg( _printJob->powerLevel )
    } );
    _connectSetPowerProcess_step4( );
    _setPowerProcess->start( );
}

void PrintManager::step4_setPowerProcessErrorOccurred( QProcess::ProcessError error ) {
    fprintf( stderr, "+ PrintManager::step4_setPowerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        fprintf( stderr, "  + setpower process failed to start\n" );
    } else if ( QProcess::Crashed == error ) {
        fprintf( stderr, "  + setpower process crashed?\n" );
        if ( _setPowerProcess->state( ) != QProcess::NotRunning ) {
            _setPowerProcess->kill( );
            fprintf( stderr, "  + setpower terminated\n" );
        }
    }
}

void PrintManager::step4_setPowerProcessStarted( ) {
    fprintf( stderr, "+ PrintManager::step4_setPowerProcessStarted\n" );
}

void PrintManager::step4_setPowerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    _disconnectSetPowerProcess_step4( );

    fprintf( stderr, "+ PrintManager::step4_setPowerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    delete _setPowerProcess;
    _setPowerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        fprintf( stderr, "  + setpower process crashed, but that's okay, carrying on\n" );
    }

    emit lampStatusChange( true );

    _layerProjectionTimer = new QTimer( this );
    if ( _currentLayer < 2 ) {
        _layerProjectionTimer->setInterval( _printJob->exposureTimeScaleFactor * _printJob->exposureTime * 1000.0 );
    } else {
        _layerProjectionTimer->setInterval( _printJob->exposureTime * 1000.0 );
    }
    _layerProjectionTimer->setSingleShot( true );
    _layerProjectionTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step5_layerProjectionTimerExpired );
    _layerProjectionTimer->start( );
}

void PrintManager::step5_layerProjectionTimerExpired( ) {
    QObject::disconnect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step5_layerProjectionTimerExpired );

    fprintf( stderr, "+ PrintManager::step5_layerProjectionTimerExpired\n" );

    delete _layerProjectionTimer;
    _layerProjectionTimer = nullptr;

    _setPowerProcess = new QProcess( this );
    _setPowerProcess->setProgram( SetPowerCommand );
    _setPowerProcess->setArguments( QStringList { "0" } );
    _connectSetPowerProcess_step6( );
    _setPowerProcess->start( );
}

void PrintManager::step6_setPowerProcessErrorOccurred( QProcess::ProcessError error ) {
    fprintf( stderr, "+ PrintManager::step6_setPowerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        fprintf( stderr, "  + setpower process failed to start\n" );
    } else if ( QProcess::Crashed == error ) {
        fprintf( stderr, "  + setpower process crashed?\n" );
        if ( _setPowerProcess->state( ) != QProcess::NotRunning ) {
            _setPowerProcess->kill( );
            fprintf( stderr, "  + setpower terminated\n" );
        }
    }
}

void PrintManager::step6_setPowerProcessStarted( ) {
    fprintf( stderr, "+ PrintManager::step6_setPowerProcessStarted\n" );
}

void PrintManager::step6_setPowerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    _disconnectSetPowerProcess_step6( );

    fprintf( stderr, "+ PrintManager::step6_setPowerProcessFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );

    delete _setPowerProcess;
    _setPowerProcess = nullptr;

    if ( exitStatus == QProcess::CrashExit ) {
        fprintf( stderr, "  + setpower process crashed, but that's okay, carrying on\n" );
    }

    emit lampStatusChange( false );

    _preLiftTimer = new QTimer( this );
    _preLiftTimer->setInterval( PauseBeforeLift );
    _preLiftTimer->setSingleShot( true );
    _preLiftTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _preLiftTimer, &QTimer::timeout, this, &PrintManager::step7_preLiftTimerExpired );
    _preLiftTimer->start( );
}

void PrintManager::step7_preLiftTimerExpired( ) {
    QObject::disconnect( _preLiftTimer, &QTimer::timeout, this, &PrintManager::step7_preLiftTimerExpired );

    delete _preLiftTimer;
    _preLiftTimer = nullptr;

    ++_currentLayer;
    if ( _currentLayer == _printJob->layerCount ) {
        QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step8_LiftUpComplete );
        fprintf( stderr, "+ PrintManager::step7_preLiftTimerExpired: moving %f\n", LiftDistance );
        _shepherd->doMove( LiftDistance );
    } else {
        _startNextLayer( );
    }
}

void PrintManager::step8_LiftUpComplete( bool success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step8_LiftUpComplete );

    fprintf( stderr, "+ PrintManager::step8_LiftUpComplete: action %s\n", success ? "succeeded" : "failed" );
    _cleanUp( );
    emit printComplete( success );
}
