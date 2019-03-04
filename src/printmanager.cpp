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
// ✗ A1. Raise the build platform to maximum Z.
// ✗ A2. Prompt user to load recommended volume of print solution.
// ✗ A3. Lower to first layer height.
// ✗ A4. Wait for four seconds.
//
// For each layer:
//
// ✓ B1. Start projection: setpower ${powerLevel} .
// ✓ B2. Pause for layer projection time (first two layers: scaled by scale factor)
// ✓ B3. Stop projection: setpower 0 .
// ✓ B4. Pause before raise.
// ✓ B5. Raise the build platform by LiftDistance.
// ✓ B6. Lower the build platform by LiftDistance - LayerHeight.
// ✓ B7. Pause before projection.
//
// After printing, whether successful or not:
//
// ✗ C1. Raise the build platform to maximum Z.
//

namespace {

    auto const PauseBeforeProject = 1000; // ms
    auto const PauseBeforeLift    = 1000; // ms
    auto const LiftDistance       = 2.00; // mm

    auto const SetpowerCommand    = QString { "setpower" };

}

PrintManager::PrintManager( Shepherd* shepherd, QObject* parent ):
    QObject   ( parent   ),
    _shepherd ( shepherd )
{
    debug( "+ construct PrintManager at %p\n", this );

    _setpowerProcess = new ProcessRunner( this );
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

    if ( _setpowerProcess ) {
        if ( _setpowerProcess->state( ) != QProcess::NotRunning ) {
            _setpowerProcess->kill( );
        }
        _setpowerProcess->deleteLater( );
        _setpowerProcess = nullptr;
    }

    if ( _preLiftTimer ) {
        _preLiftTimer->stop( );
        _preLiftTimer->deleteLater( );
        _preLiftTimer = nullptr;
    }
}

void PrintManager::_startNextLayer( ) {
    debug( "+ PrintManager::_startNextLayer: moving %f\n", LiftDistance );
    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step1_liftUpComplete );
    _shepherd->doMove( LiftDistance );
    emit startingLayer( _currentLayer );
}

void PrintManager::step1_liftUpComplete( bool const success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step1_liftUpComplete );

    if ( !success ) {
        debug( "+ PrintManager::step1_liftUpComplete: action failed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
    
    auto const moveDistance = -LiftDistance + _printJob->layerThickness / 1000.0;
    debug( "+ PrintManager::step1_liftUpComplete: action succeeded, moving %f\n", moveDistance );
    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step2_liftDownComplete );
    _shepherd->doMove( moveDistance );
}

void PrintManager::step2_liftDownComplete( bool const success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::step2_liftDownComplete );

    if ( !success ) {
        debug( "+ PrintManager::step2_liftDownComplete: action failed\n" );
        _cleanUp( );
        emit printComplete( false );
        return;
    }
    debug( "+ PrintManager::step2_liftDownComplete: action succeeded\n" );

    QString pngFileName = _printJob->jobWorkingDirectory + QString( "/%1.png" ).arg( _currentLayer, 6, 10, DigitZero );
    if ( !_pngDisplayer->load( pngFileName ) ) {
        debug( "+ PrintManager::step2_liftDownComplete: PngDisplayer::load failed for file %s\n", pngFileName.toUtf8( ).data( ) );
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

    QObject::connect( _setpowerProcess, &ProcessRunner::succeeded, this, &PrintManager::step4_setPowerCompleted );
    QObject::connect( _setpowerProcess, &ProcessRunner::failed,    this, &PrintManager::step4_setPowerFailed    );
    _setpowerProcess->start( SetpowerCommand, { QString( "%1" ).arg( _printJob->powerLevel ) } );
}

void PrintManager::step4_setPowerCompleted( ) {
    debug( "+ PrintManager::step4_setPowerCompleted\n" );

    QObject::disconnect( _setpowerProcess, nullptr, this, nullptr );

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

    step4_setPowerCompleted( );
}

void PrintManager::step5_layerProjectionTimerExpired( ) {
    QObject::disconnect( _layerProjectionTimer, &QTimer::timeout, this, &PrintManager::step5_layerProjectionTimerExpired );

    debug( "+ PrintManager::step5_layerProjectionTimerExpired\n" );

    _layerProjectionTimer->deleteLater( );
    _layerProjectionTimer = nullptr;

    QObject::connect( _setpowerProcess, &ProcessRunner::succeeded, this, &PrintManager::step6_setPowerCompleted );
    QObject::connect( _setpowerProcess, &ProcessRunner::failed,    this, &PrintManager::step6_setPowerFailed    );
    _setpowerProcess->start( SetpowerCommand, { "0" } );
}

void PrintManager::step6_setPowerCompleted( ) {
    debug( "+ PrintManager::step6_setPowerCompleted\n" );

    QObject::disconnect( _setpowerProcess, nullptr, this, nullptr );

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

    step6_setPowerCompleted( );
}

void PrintManager::step7_preLiftTimerExpired( ) {
    QObject::disconnect( _preLiftTimer, &QTimer::timeout, this, &PrintManager::step7_preLiftTimerExpired );

    _preLiftTimer->deleteLater( );
    _preLiftTimer = nullptr;

    ++_currentLayer;
    if ( _currentLayer == _printJob->layerCount ) {
        QObject::connect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::step8_liftUpComplete );
        debug( "+ PrintManager::step7_preLiftTimerExpired: raising build platform\n" );
        _shepherd->doMoveTo( PrinterMaximumZ );
    } else {
        _startNextLayer( );
    }
}

void PrintManager::step8_liftUpComplete( bool const success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::step8_liftUpComplete );

    debug( "+ PrintManager::step8_liftUpComplete: action %s\n", success ? "succeeded" : "failed" );
    _cleanUp( );
    emit printComplete( success );
}

void PrintManager::abort_liftUpComplete( bool const success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::abort_liftUpComplete );

    debug( "+ PrintManager::abort_liftUpComplete: action %s\n", success ? "succeeded" : "failed" );
    _cleanUp( );
    emit printAborted( );
}

void PrintManager::print( PrintJob* printJob ) {
    if ( _printJob ) {
        debug( "+ PrintManager::print: Job submitted while we're busy\n" );
        return;
    }
    debug( "+ PrintManager::print: new job\n" );
    _printJob = printJob;

    _pngDisplayer = new PngDisplayer( );
    _pngDisplayer->setFixedSize( PngDisplayWindowSize );
    _pngDisplayer->move( g_settings.pngDisplayWindowPosition );
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

    if ( _lampOn ) {
        QProcess::startDetached( SetpowerCommand, { "0" } );
        _lampOn = false;
    }

    debug( "+ PrintManager::abort: raising build platform\n" );
    QObject::connect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::abort_liftUpComplete );
    _shepherd->doMoveTo( PrinterMaximumZ );
}
