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
// A1. Raise the build platform to maximum Z.
// A2. Prompt user to load recommended volume of print solution.
// A3. Lower to first layer height.
// A4. Wait for four seconds.
//
// For each layer:
//
// B1. Poll printer temperature.
// B2. Start projection: "setpower ${powerLevel}".
// B3. Pause for layer projection time (first two layers: scaled by scale factor)
// B4. Stop projection: "setpower 0".
// B5. Pause before raise.
// B6. Raise the build platform by LiftDistance.
// B7. Lower the build platform by LiftDistance - LayerHeight.
// B8. Pause before projection.
//
// After printing, whether successful or not:
//
// C1. Raise the build platform to maximum Z.
//

namespace {

    auto const PauseAfterPrintSolutionLoad = 4000; // ms
    auto const PauseBeforeProject          = 1000; // ms
    auto const PauseBeforeLift             = 1000; // ms
    auto const LiftDistance                = 2.00; // mm

    char const* PrintResultStrings[] {
        "Failure",
        "Success",
        "Abort",
    };

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
    debug( "+ PrintManager::stepA1_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepA1_completed );

    if ( !success ) {
        _printResult = PrintResult::Failure;
        stepC1_start( );
        return;
    }

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
    debug( "+ PrintManager::stepA3_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepA3_completed );

    if ( !success ) {
        _printResult = PrintResult::Failure;
        stepC1_start( );
        return;
    }

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

    stepB2_start( );
}

// B1. Poll printer temperature.
void PrintManager::stepB1_start( ) {
    debug( "+ PrintManager::stepB1_start: polling printer temperature\n" );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &PrintManager::stepB1_completed );
    _shepherd->doSend( QString { "M105" } );
}

void PrintManager::stepB1_completed( bool const success ) {
    debug( "+ PrintManager::stepB1_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_sendComplete, this, &PrintManager::stepB1_completed );

    stepB2_start( );
}

// B2. Start projection: "setpower ${_printJob->powerLevel}".
void PrintManager::stepB2_start( ) {
    debug( "+ PrintManager::stepB2_start: running 'setpower %d'\n", _printJob->powerLevel );

    QString pngFileName = _printJob->jobWorkingDirectory + QString( "/%1.png" ).arg( _currentLayer, 6, 10, DigitZero );
    if ( !_pngDisplayer->load( pngFileName ) ) {
        debug( "+ PrintManager::stepB2_start: PngDisplayer::load failed for file %s\n", pngFileName.toUtf8( ).data( ) );
        this->abort( );
        return;
    }

    QObject::connect( _setpowerProcess, &ProcessRunner::succeeded, this, &PrintManager::stepB2_completed );
    QObject::connect( _setpowerProcess, &ProcessRunner::failed,    this, &PrintManager::stepB2_failed    );
    _setpowerProcess->start( SetpowerCommand, { QString( "%1" ).arg( _printJob->powerLevel ) } );
}

void PrintManager::stepB2_completed( ) {
    debug( "+ PrintManager::stepB2_completed\n" );

    QObject::disconnect( _setpowerProcess, nullptr, this, nullptr );

    _lampOn = true;
    emit lampStatusChange( true );

    stepB3_start( );
}

void PrintManager::stepB2_failed( QProcess::ProcessError const ) {
    stepB2_completed( );
}

// B3. Pause for layer projection time (first two layers: scaled by scale factor)
void PrintManager::stepB3_start( ) {
    int layerProjectionTime = 1000.0 * _printJob->exposureTime * ( ( _currentLayer < 2 ) ? _printJob->exposureTimeScaleFactor : 1.0 );
    debug( "+ PrintManager::stepB3_start: pausing for %d ms\n", layerProjectionTime );

    _layerProjectionTimer = _makeAndStartTimer( layerProjectionTime, &PrintManager::stepB3_completed );
}

void PrintManager::stepB3_completed( ) {
    debug( "+ PrintManager::stepB3_completed\n" );

    _stopAndCleanUpTimer( _layerProjectionTimer );

    stepB4_start( );
}

// B4. Stop projection: "setpower 0".
void PrintManager::stepB4_start( ) {
    debug( "+ PrintManager::stepB4_start: running 'setpower 0'\n" );

    _pngDisplayer->clear( );

    QObject::connect( _setpowerProcess, &ProcessRunner::succeeded, this, &PrintManager::stepB4_completed );
    QObject::connect( _setpowerProcess, &ProcessRunner::failed,    this, &PrintManager::stepB4_failed    );
    _setpowerProcess->start( SetpowerCommand, { "0" } );
}

void PrintManager::stepB4_completed( ) {
    debug( "+ PrintManager::stepB4_completed\n" );

    QObject::disconnect( _setpowerProcess, nullptr, this, nullptr );

    _lampOn = false;
    emit lampStatusChange( false );

    stepB5_start( );
}

void PrintManager::stepB4_failed( QProcess::ProcessError const error ) {
    stepB4_completed( );
}

// B5. Pause before raise.
void PrintManager::stepB5_start( ) {
    debug( "+ PrintManager::stepB5_start: pausing for %d ms before raising build platform\n", PauseBeforeLift );

    _preLiftTimer = _makeAndStartTimer( PauseBeforeLift, &PrintManager::stepB5_completed );
}

void PrintManager::stepB5_completed( ) {
    debug( "+ PrintManager::stepB5_completed\n" );

    _stopAndCleanUpTimer( _preLiftTimer );

    stepB6_start( );
}

// B6. Raise the build platform by LiftDistance.
void PrintManager::stepB6_start( ) {
    if ( ++_currentLayer == _printJob->layerCount ) {
        debug( "+ PrintManager::stepB6_start: print complete\n" );

        _printResult = PrintResult::Success;

        stepC1_start( );
    } else {
        debug( "+ PrintManager::stepB6_start: raising build platform by %.2f mm\n", LiftDistance );

        QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::stepB6_completed );
        _shepherd->doMove( LiftDistance );

        emit startingLayer( _currentLayer );
    }
}

void PrintManager::stepB6_completed( bool const success ) {
    debug( "+ PrintManager::stepB6_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::stepB6_completed );

    if ( !success ) {
        _printResult = PrintResult::Failure;
        stepC1_start( );
        return;
    }

    stepB7_start( );
}

// B7. Lower the build platform by LiftDistance - LayerHeight.
void PrintManager::stepB7_start( ) {
    auto const moveDistance = -LiftDistance + _printJob->layerThickness / 1000.0;
    debug( "+ PrintManager::stepB7_start: lowering build platform by %.2f mm\n", moveDistance );

    QObject::connect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::stepB7_completed );
    _shepherd->doMove( moveDistance );
}

void PrintManager::stepB7_completed( bool const success ) {
    debug( "+ PrintManager::stepB7_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveComplete, this, &PrintManager::stepB7_completed );

    if ( !success ) {
        _printResult = PrintResult::Failure;
        stepC1_start( );
        return;
    }

    stepB8_start( );
}

// B8. Pause before projection.
void PrintManager::stepB8_start( ) {
    debug( "+ PrintManager::stepB8_start: pausing for %d ms before projecting layer\n", PauseBeforeProject );

    _preProjectionTimer = _makeAndStartTimer( PauseBeforeProject, &PrintManager::stepB8_completed );
}

void PrintManager::stepB8_completed( ) {
    debug( "+ PrintManager::stepB8_completed\n" );

    _stopAndCleanUpTimer( _preProjectionTimer );

    stepB2_start( );
}

// C1. Raise the build platform to maximum Z.
void PrintManager::stepC1_start( ) {
    debug( "+ PrintManager::stepC1_start: raising build platform to maximum Z\n" );

    QObject::connect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepC1_completed );
    _shepherd->doMoveTo( PrinterMaximumZ );
}

void PrintManager::stepC1_completed( bool const success ) {
    debug( "+ PrintManager::stepC1_completed: action %s. print result %s\n", SucceededString( success ), PrintResultStrings[+_printResult] );

    QObject::disconnect( _shepherd, &Shepherd::action_moveToComplete, this, &PrintManager::stepC1_completed );

    if ( PrintResult::Abort == _printResult ) {
        emit printAborted( );
    } else {
        emit printComplete( _printResult == PrintResult::Success );
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
    _cleanUp( );
}

void PrintManager::abort( ) {
    debug( "+ PrintManager::abort\n" );

    _printResult = PrintResult::Abort;

    if ( _lampOn ) {
        QProcess::startDetached( SetpowerCommand, { "0" } );
        _lampOn = false;
    }

    stepC1_start( );
}

void PrintManager::printSolutionLoaded( ) {
    stepA2_completed( );
}
