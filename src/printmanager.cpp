#include "pch.h"

#include "printmanager.h"

#include "app.h"
#include "pngdisplayer.h"
#include "printjob.h"
#include "processrunner.h"
#include "shepherd.h"
#include "strings.h"

//
// Before printing:
//
// ✓ 0. Home the printer
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
//
// After printing:
//
// ✓ 8. Final lift up
//

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
    debug( "+ construct PrintManager at %p\n", this );

    _setPowerProcess = new ProcessRunner( this );
}

PrintManager::~PrintManager( ) {
    debug( "+ destruct PrintManager at %p\n", this );
}

void PrintManager::_cleanUp( ) {
    QObject::disconnect( this );

    if ( _printJob ) {
        delete _printJob;
        _printJob = nullptr;
    }

    if ( _pngDisplayer ) {
        _pngDisplayer->close( );
        _pngDisplayer->deleteLater( );
        _pngDisplayer = nullptr;
    }

    if ( _preProjectionTimer ) {
        _preProjectionTimer->stop( );
        _preProjectionTimer->deleteLater( );
        _preProjectionTimer = nullptr;
    }

    if ( _setPowerProcess ) {
        if ( _setPowerProcess->state( ) != QProcess::NotRunning ) {
            _setPowerProcess->kill( );
        }
        _setPowerProcess->deleteLater( );
        _setPowerProcess = nullptr;
    }

    if ( _preLiftTimer ) {
        _preLiftTimer->stop( );
        _preLiftTimer->deleteLater( );
        _preLiftTimer = nullptr;
    }
}

void PrintManager::print( PrintJob* printJob ) {
    if ( _printJob ) {
        debug( "+ PrintManager::print: Job submitted while we're busy\n" );
        return;
    }
    debug( "+ PrintManager::print: new job\n" );
    _printJob = printJob;

    _pngDisplayer = new PngDisplayer( );
    _pngDisplayer->setWindowFlags( _pngDisplayer->windowFlags( ) | Qt::BypassWindowManagerHint );
    _pngDisplayer->setFixedSize( 1280, 800 );
    _pngDisplayer->move( { 0, g_settings.startY ? 0 : 480 } );
    _pngDisplayer->show( );

    emit printStarting( );
    _startNextLayer( );
}

void PrintManager::terminate( ) {
    debug( "+ PrintManager::terminate\n" );
    _cleanUp( );
}

void PrintManager::abort( ) {
    debug( "+ PrintManager::abort\n" );

    _cleanUp( );
    if ( _lampOn ) {
        QProcess::startDetached( SetPowerCommand, { "0" } );
        _lampOn = false;
    }
    emit printComplete( false );
}

void PrintManager::_startNextLayer( ) {
    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step1_LiftUpComplete );
    debug( "+ PrintManager::_startNextLayer: moving %f\n", LiftDistance );
    _shepherd->doMove( LiftDistance );
    emit startingLayer( _currentLayer );
}

void PrintManager::step1_LiftUpComplete( bool const success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step1_LiftUpComplete );

    if ( !success ) {
        debug( "+ PrintManager::step1_LiftUpComplete: action failed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
    debug( "+ PrintManager::step1_LiftUpComplete: action succeeded\n" );

    debug( "+ PrintManager::step1_LiftUpComplete: moving %f\n", -LiftDistance + _printJob->layerThickness / 1000.0 );
    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step2_LiftDownComplete );
    _shepherd->doMove( -LiftDistance + _printJob->layerThickness / 1000.0 );
}

void PrintManager::step2_LiftDownComplete( bool const success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step2_LiftDownComplete );

    if ( !success ) {
        debug( "+ PrintManager::step2_LiftDownComplete: action failed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
    debug( "+ PrintManager::step2_LiftDownComplete: action succeeded\n" );

    QString pngFileName = _printJob->pngFilesPath + QString( "/%1.png" ).arg( _currentLayer, 6, 10, QChar( '0' ) );
    if ( !_pngDisplayer->load( pngFileName ) ) {
        debug( "+ PrintManager::step2_LiftDownComplete: PngDisplayer::load failed for file %s\n", pngFileName.toUtf8( ).data( ) );
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

    debug( "+ PrintManager::step3_preProjectionTimerExpired\n" );

    _preProjectionTimer->deleteLater( );
    _preProjectionTimer = nullptr;

    QObject::connect( _setPowerProcess, &ProcessRunner::succeeded, this, &PrintManager::step4_setPowerCompleted );
    QObject::connect( _setPowerProcess, &ProcessRunner::failed,    this, &PrintManager::step4_setPowerFailed    );
    _setPowerProcess->start( SetPowerCommand, { QString( "%1" ).arg( _printJob->powerLevel ) } );
}

void PrintManager::step4_setPowerCompleted( ) {
    debug( "+ PrintManager::step4_setPowerCompleted\n" );

    QObject::disconnect( _setPowerProcess, nullptr, this, nullptr );

    _lampOn = true;
    emit lampStatusChange( true );

    _layerProjectionTimer = new QTimer( this );
    _layerProjectionTimer->setInterval( 1000.0 * _printJob->exposureTime * ( ( _currentLayer < 2 ) ? _printJob->exposureTimeScaleFactor : 1.0 ) );
    _layerProjectionTimer->setSingleShot( true );
    _layerProjectionTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step5_layerProjectionTimerExpired );
    _layerProjectionTimer->start( );
}

void PrintManager::step4_setPowerFailed( QProcess::ProcessError const error ) {
    debug( "+ PrintManager::step4_setPowerFailed: but that's okay, carrying on\n" );

    if ( QProcess::Crashed == error ) {
        if ( _setPowerProcess->state( ) != QProcess::NotRunning ) {
            _setPowerProcess->kill( );
        }
    }

    step4_setPowerCompleted( );
}

void PrintManager::step5_layerProjectionTimerExpired( ) {
    QObject::disconnect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step5_layerProjectionTimerExpired );

    debug( "+ PrintManager::step5_layerProjectionTimerExpired\n" );

    _layerProjectionTimer->deleteLater( );
    _layerProjectionTimer = nullptr;

    QObject::connect( _setPowerProcess, &ProcessRunner::succeeded, this, &PrintManager::step6_setPowerCompleted );
    QObject::connect( _setPowerProcess, &ProcessRunner::failed,    this, &PrintManager::step6_setPowerFailed    );
    _setPowerProcess->start( SetPowerCommand, { "0" } );
}

void PrintManager::step6_setPowerCompleted( ) {
    debug( "+ PrintManager::step6_setPowerCompleted\n" );

    QObject::disconnect( _setPowerProcess, nullptr, this, nullptr );

    _lampOn = false;
    emit lampStatusChange( false );

    _preLiftTimer = new QTimer( this );
    _preLiftTimer->setInterval( PauseBeforeLift );
    _preLiftTimer->setSingleShot( true );
    _preLiftTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _preLiftTimer, &QTimer::timeout, this, &PrintManager::step7_preLiftTimerExpired );
    _preLiftTimer->start( );
}

void PrintManager::step6_setPowerFailed( QProcess::ProcessError const error ) {
    debug( "+ PrintManager::step6_setPowerFailed: %s [%d], but that's okay, carrying on\n", ToString( error ), error );

    if ( QProcess::Crashed == error ) {
        if ( _setPowerProcess->state( ) != QProcess::NotRunning ) {
            _setPowerProcess->kill( );
        }
    }

    step6_setPowerCompleted( );
}

void PrintManager::step7_preLiftTimerExpired( ) {
    QObject::disconnect( _preLiftTimer, &QTimer::timeout, this, &PrintManager::step7_preLiftTimerExpired );

    _preLiftTimer->deleteLater( );
    _preLiftTimer = nullptr;

    ++_currentLayer;
    if ( _currentLayer == _printJob->layerCount ) {
        QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step8_LiftUpComplete );
        debug( "+ PrintManager::step7_preLiftTimerExpired: moving %f\n", LiftDistance );
        _shepherd->doMove( LiftDistance );
    } else {
        _startNextLayer( );
    }
}

void PrintManager::step8_LiftUpComplete( bool const success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step8_LiftUpComplete );

    debug( "+ PrintManager::step8_LiftUpComplete: action %s\n", success ? "succeeded" : "failed" );
    _cleanUp( );
    emit printComplete( success );
}
