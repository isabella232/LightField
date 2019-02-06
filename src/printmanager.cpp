#include "printmanager.h"

#include "pngdisplayer.h"
#include "shepherd.h"
#include "strings.h"

//
// For each layer:
//
// ✓ 1. Lift up
// ✓ 2. Lift down
// ✓ 3. [deleted]
// ✓ 4. Pause before projection
// ✓ 5. Start projecting: setpower ${brightness}
// ✓ 6. Pause for layer time
// ✓ 7. Stop projection: setpower 0
// ✓ 8. [deleted]
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

    auto SetPowerCommand    = QString { "setpower" };

}

PrintManager::PrintManager( Shepherd* shepherd, QObject* parent ):
    QObject   ( parent   ),
    _shepherd ( shepherd )
{
    /*empty*/
}

PrintManager::~PrintManager( ) {
    /*empty*/
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

    if ( _setPowerProcessConnected_step5 ) {
        _disconnectSetPowerProcess_step5( );
    }

    if ( _setPowerProcessConnected_step7 ) {
        _disconnectSetPowerProcess_step7( );
    }

    if ( _setPowerProcess ) {
        if ( _setPowerProcess->state( ) != QProcess::NotRunning ) {
            _setPowerProcess->terminate( );
            _setPowerProcess->waitForFinished( );
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
    fprintf( stderr, "+ PrintManager::print: new job\n" );
    _printJob = printJob;

    _pngDisplayer = new PngDisplayer( );
    _pngDisplayer->move( { 0, 0 } );
    _pngDisplayer->resize( { 1280, 800 } );
    _pngDisplayer->show( );
    _pngDisplayer->setFullScreen( true );

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
    fprintf( stderr, "+ PrintManager::startNextLayer: moving %f\n", LiftDistance );
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

    QString pngFileName = _printJob->pngFilesPath + QString( "/%1.png" ).arg( _currentLayer, 3, 10, QChar( '0' ) );
    if ( !_pngDisplayer->load( pngFileName ) ) {
        fprintf( stderr, "+ PrintManager::step2_LiftDownComplete: PngDisplayer::load failed for file %s\n", pngFileName.toUtf8( ).data( ) );
    }

    _preProjectionTimer = new QTimer( this );
    _preProjectionTimer->setInterval( PauseBeforeProject );
    _preProjectionTimer->setSingleShot( true );
    _preProjectionTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _preProjectionTimer, &QTimer::timeout, this, &PrintManager::step4_timerExpired );
    _preProjectionTimer->start( );
}

void PrintManager::step4_timerExpired( ) {
    QObject::disconnect( _preProjectionTimer, &QTimer::timeout, this, &PrintManager::step4_timerExpired );

    fprintf( stderr, "+ PrintManager::step4_timerExpired\n" );

    delete _preProjectionTimer;
    _preProjectionTimer = nullptr;

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
    } else if ( QProcess::Crashed == error ) {
        fprintf( stderr, "  + setpower process crashed, but carrying on anyway\n" );
        step5_setPowerProcessFinished( 139, QProcess::CrashExit );
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
        fprintf( stderr, "  + setpower process crashed, but carrying on anyway\n" );
        //_cleanUp( );
        //emit printComplete( false );
        //return;
    }

    _layerProjectionTimer = new QTimer( this );
    _layerProjectionTimer->setInterval( _printJob->exposureTime * 1000.0 );
    _layerProjectionTimer->setSingleShot( true );
    _layerProjectionTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step6_timerExpired );
    _layerProjectionTimer->start( );
}

void PrintManager::step6_timerExpired( ) {
    QObject::disconnect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step6_timerExpired );

    fprintf( stderr, "+ PrintManager::step6_timerExpired\n" );

    delete _layerProjectionTimer;
    _layerProjectionTimer = nullptr;

    _setPowerProcess = new QProcess( this );
    _setPowerProcess->setProgram( SetPowerCommand );
    _setPowerProcess->setArguments( QStringList { "0" } );
    _connectSetPowerProcess_step7( );
    _setPowerProcess->start( );
}

void PrintManager::step7_setPowerProcessErrorOccurred( QProcess::ProcessError error ) {
    _disconnectSetPowerProcess_step7( );

    fprintf( stderr, "+ PrintManager::step7_setPowerProcessErrorOccurred: error %s [%d]\n", ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        fprintf( stderr, "  + setpower process failed to start\n" );
        _cleanUp( );
        emit printComplete( false );
    } else if ( QProcess::Crashed == error ) {
        fprintf( stderr, "  + setpower process crashed, but carrying on anyway\n" );
        step7_setPowerProcessFinished( 139, QProcess::CrashExit );
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
        fprintf( stderr, "  + setpower process crashed, but carrying on anyway\n" );
        //_cleanUp( );
        //emit printComplete( false );
        //return;
    }

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
    _preLiftTimer->start( );
}

void PrintManager::step9_timerExpired( ) {
    QObject::disconnect( _preLiftTimer, &QTimer::timeout, this, &PrintManager::step9_timerExpired );

    delete _preLiftTimer;
    _preLiftTimer = nullptr;

    startNextLayer( );
}
