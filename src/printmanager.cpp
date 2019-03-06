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

    auto const PauseAfterPrintSolutionLoad = 4000; // ms
    auto const PauseBeforeProject          = 1000; // ms
    auto const PauseBeforeLift             = 1000; // ms
    auto const LiftDistance                = 2.00; // mm

    auto const SetpowerCommand             = QString { "setpower" };

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

QTimer* PrintManager::_makeAndStartTimer( int const interval, TimerExpiryFunc func ) {
    auto timer = new QTimer( this );
    QObject::connect( timer, &QTimer::timeout, this, func );
    timer->setInterval( interval );
    timer->setSingleShot( true );
    timer->setTimerType( Qt::PreciseTimer );
    timer->start( );
    return timer;
}

void PrintManager::_stopAndCleanUpTimer( QTimer*& timer ) {
    if ( !timer ) {
        return;
    }

    QObject::disconnect( timer, nullptr, this, nullptr );
    timer->stop( );
    timer->deleteLater( );
    timer = nullptr;
}

void PrintManager::_cleanUp( ) {
    QObject::disconnect( this );

    _stopAndCleanUpTimer( _preProjectionTimer   );
    _stopAndCleanUpTimer( _layerProjectionTimer );
    _stopAndCleanUpTimer( _preLiftTimer         );

    if ( _printJob ) {
        delete _printJob;
        _printJob = nullptr;
    }

    if ( _pngDisplayer ) {
        _pngDisplayer->close( );
        _pngDisplayer->deleteLater( );
        _pngDisplayer = nullptr;
    }

    if ( _setpowerProcess ) {
        if ( _setpowerProcess->state( ) != QProcess::NotRunning ) {
            _setpowerProcess->kill( );
        }
        _setpowerProcess->deleteLater( );
        _setpowerProcess = nullptr;
    }
}

// A1. Raise the build platform to maximum Z.
void PrintManager::stepA1_start( ) {
    debug( "+ PrintManager::stepA1_start: raising build platform to %.2f mm\n", PrinterMaximumZ );

    QObject::connect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepA1_completed );
    _shepherd->doMoveTo( PrinterMaximumZ );
}

void PrintManager::stepA1_completed( bool const success ) {
    debug( "+ PrintManager::stepA1_completed: action %s\n", success ? "succeeded" : "failed" );

    QObject::disconnect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepA1_completed );

    stepA2_start( );
}

// A2. Prompt user to load recommended volume of print solution.
void PrintManager::stepA2_start( ) {
    debug( "+ PrintManager::stepA2_start: waiting for user to load print solution\n" );

    emit requestLoadPrintSolution( );
}

void PrintManager::stepA2_completed( ) {
    debug( "+ PrintManager::stepA2_completed\n" );

    stepA3_start( );
}

// A3. Lower to first layer height.
void PrintManager::stepA3_start( ) {
    auto firstLayerHeight = std::max( 100, _printJob->layerThickness ) / 1000.0;

    debug( "+ PrintManager::stepA3_start: lowering build platform to %.2f mm (layer thickness: %d µm)\n", firstLayerHeight, _printJob->layerThickness );

    QObject::connect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepA3_completed );
    _shepherd->doMoveTo( firstLayerHeight );
}

void PrintManager::stepA3_completed( bool const success ) {
    debug( "+ PrintManager::stepA3_completed: action %s\n", success ? "succeeded" : "failed" );

    QObject::disconnect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepA3_completed );

    stepA4_start( );
}

// A4. Wait for four seconds.
void PrintManager::stepA4_start( ) {
    debug( "+ PrintManager::stepA4_start: pausing for %d ms\n", PauseAfterPrintSolutionLoad );

    _preProjectionTimer = _makeAndStartTimer( PauseAfterPrintSolutionLoad, &PrintManager::stepA4_completed );
}

void PrintManager::stepA4_completed( ) {
    debug( "+ PrintManager::stepA4_completed\n" );

    _stopAndCleanUpTimer( _preProjectionTimer );

    emit startingLayer( _currentLayer );

    stepB1_start( );
}

// B1. Start projection: "setpower ${_printJob->powerLevel}".
void PrintManager::stepB1_start( ) {
    debug( "+ PrintManager::stepB1_start: running 'setpower %d'\n", _printJob->powerLevel );

    QString pngFileName = _printJob->jobWorkingDirectory + QString( "/%1.png" ).arg( _currentLayer, 6, 10, DigitZero );
    if ( !_pngDisplayer->load( pngFileName ) ) {
        debug( "+ PrintManager::stepB1_start: PngDisplayer::load failed for file %s\n", pngFileName.toUtf8( ).data( ) );
        this->abort( );
        return;
    }

    QObject::connect( _setpowerProcess, &ProcessRunner::succeeded, this, &PrintManager::stepB1_completed );
    QObject::connect( _setpowerProcess, &ProcessRunner::failed,    this, &PrintManager::stepB1_failed    );
    _setpowerProcess->start( SetpowerCommand, { QString( "%1" ).arg( _printJob->powerLevel ) } );
}

void PrintManager::stepB1_completed( ) {
    debug( "+ PrintManager::stepB1_completed\n" );

    QObject::disconnect( _setpowerProcess, nullptr, this, nullptr );

    _lampOn = true;
    emit lampStatusChange( true );

    stepB2_start( );
}

void PrintManager::stepB1_failed( QProcess::ProcessError const ) {
    debug( "+ PrintManager::stepB1_failed: but that's okay, carrying on\n" );

    stepB1_completed( );
}

// B2. Pause for layer projection time (first two layers: scaled by scale factor)
void PrintManager::stepB2_start( ) {
    int layerProjectionTime = 1000.0 * _printJob->exposureTime * ( ( _currentLayer < 2 ) ? _printJob->exposureTimeScaleFactor : 1.0 );
    debug( "+ PrintManager::stepB2_start: pausing for %d ms\n", layerProjectionTime );

    _layerProjectionTimer = _makeAndStartTimer( layerProjectionTime, &PrintManager::stepB2_completed );
}

void PrintManager::stepB2_completed( ) {
    debug( "+ PrintManager::stepB2_completed\n" );

    _stopAndCleanUpTimer( _layerProjectionTimer );

    stepB3_start( );
}

// B3. Stop projection: "setpower 0".
void PrintManager::stepB3_start( ) {
    debug( "+ PrintManager::stepB3_start: running 'setpower 0'\n" );

    _pngDisplayer->clear( );

    QObject::connect( _setpowerProcess, &ProcessRunner::succeeded, this, &PrintManager::stepB3_completed );
    QObject::connect( _setpowerProcess, &ProcessRunner::failed,    this, &PrintManager::stepB3_failed    );
    _setpowerProcess->start( SetpowerCommand, { "0" } );
}

void PrintManager::stepB3_completed( ) {
    debug( "+ PrintManager::stepB3_completed\n" );

    QObject::disconnect( _setpowerProcess, nullptr, this, nullptr );

    _lampOn = false;
    emit lampStatusChange( false );

    stepB4_start( );
}

void PrintManager::stepB3_failed( QProcess::ProcessError const error ) {
    debug( "+ PrintManager::stepB3_failed: but that's okay, carrying on\n" );

    stepB3_completed( );
}

// B4. Pause before raise.
void PrintManager::stepB4_start( ) {
    debug( "+ PrintManager::stepB4_start: pausing for %d ms before raising build platform\n", PauseBeforeLift );

    _preLiftTimer = _makeAndStartTimer( PauseBeforeLift, &PrintManager::stepB4_completed );
}

void PrintManager::stepB4_completed( ) {
    debug( "+ PrintManager::stepB4_completed\n" );

    _stopAndCleanUpTimer( _preLiftTimer );

    stepB5_start( );
}

// B5. Raise the build platform by LiftDistance.
void PrintManager::stepB5_start( ) {
    if ( ++_currentLayer == _printJob->layerCount ) {
        debug( "+ PrintManager::stepB5_start: print complete\n" );

        _aborting = false;
        _success  = true;

        stepC1_start( );
    } else {
        debug( "+ PrintManager::stepB5_start: raising build platform by %.2f mm\n", LiftDistance );

        QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::stepB5_completed );
        _shepherd->doMove( LiftDistance );

        emit startingLayer( _currentLayer );
    }
}

void PrintManager::stepB5_completed( bool const success ) {
    debug( "+ PrintManager::stepB5_completed\n" );

    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::stepB5_completed );

    stepB6_start( );
}

// B6. Lower the build platform by LiftDistance - LayerHeight.
void PrintManager::stepB6_start( ) {
    auto const moveDistance = -LiftDistance + _printJob->layerThickness / 1000.0;
    debug( "+ PrintManager::stepB6_start: lowering build platform by %.2f mm\n", moveDistance );

    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::stepB6_completed );
    _shepherd->doMove( moveDistance );
}

void PrintManager::stepB6_completed( bool const success ) {
    debug( "+ PrintManager::stepB6_completed\n" );

    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::stepB6_completed );

    stepB7_start( );
}

// B7. Pause before projection.
void PrintManager::stepB7_start( ) {
    debug( "+ PrintManager::stepB7_start: pausing for %d ms before projecting layer\n", PauseBeforeProject );

    _preProjectionTimer = _makeAndStartTimer( PauseBeforeProject, &PrintManager::stepB7_completed );
}

void PrintManager::stepB7_completed( ) {
    debug( "+ PrintManager::stepB7_completed\n" );

    _stopAndCleanUpTimer( _preProjectionTimer );

    stepB1_start( );
}

// C1. Raise the build platform to maximum Z.
void PrintManager::stepC1_start( ) {
    debug( "+ PrintManager::stepC1_start: raising build platform to maximum Z\n" );

    QObject::connect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepC1_completed );
    _shepherd->doMoveTo( PrinterMaximumZ );
}

void PrintManager::stepC1_completed( bool const success ) {
    debug( "+ PrintManager::stepC1_completed: %s. aborting? %s success? %s\n", success ? "success" : "failure", _aborting ? "yes" : "no", _success ? "yes" : "no" );

    QObject::disconnect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepC1_completed );

    if ( _aborting ) {
        emit printAborted( );
    } else {
        emit printComplete( _success );
    }
    _cleanUp( );
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

    debug( "+ PrintManager::print: emitting printStarting()\n" );
    emit printStarting( );
    stepA1_start( );
}

void PrintManager::terminate( ) {
    debug( "+ PrintManager::terminate\n" );
    _aborting = false;
    _cleanUp( );
}

void PrintManager::abort( ) {
    debug( "+ PrintManager::abort\n" );

    _aborting = true;

    if ( _lampOn ) {
        QProcess::startDetached( SetpowerCommand, { "0" } );
        _lampOn = false;
    }

    stepC1_start( );
}

void PrintManager::printSolutionLoaded( ) {
    stepA2_completed( );
}
