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
// A2. Prompt user to dispense print solution.
// A3. Lower to 5 mm.
// A4. Configure print speed as per job.
// A5. Lower to first layer height.
// A6. Wait for four seconds.
//
// For each layer:
//
// B1. Start projection: "setpower ${powerLevel}".
// B2. Pause for layer projection time (first two layers: scaled by scale factor)
// B3. Stop projection: "setpower 0".
// B4. Pause before raise.
// B5. Raise the build platform by LiftDistance.
// B6. Lower the build platform by LiftDistance - LayerHeight.
// B7. Pause before projection.
//
// After printing, whether successful or not:
//
// C1. Configure print speed to 200 mm/min.
// C2. Raise the build platform to maximum Z.
//

namespace {

    auto const PauseAfterPrintSolutionLoad = 4000; // ms
    auto const PauseBeforeProject          = 1000; // ms
    auto const PauseBeforeLift             = 1000; // ms
    auto const LiftDistance                = 2.00; // mm

    char const* PrintResultStrings[] {
        "None",
        "Failure",
        "Success",
        "Abort",
    };

    bool IsBadPrintResult( PrintResult const printResult ) {
        return printResult < PrintResult::None;
    }

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

QTimer* PrintManager::_makeAndStartTimer( int const interval, void ( PrintManager::*func )( ) ) {
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

    _step = PrintStep::none;

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
    _step = PrintStep::A1;

    debug( "+ PrintManager::stepA1_start: raising build platform to %.2f mm\n", PrinterRaiseToMaxZHeight );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA1_completed );
    _shepherd->doMoveAbsolute( PrinterRaiseToMaxZHeight );
}

void PrintManager::stepA1_completed( bool const success ) {
    debug( "+ PrintManager::stepA1_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA1_completed );

    if ( !success && !IsBadPrintResult( _printResult ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepA2_start( );
}

// A2. Prompt user to dispense recommended volume of print solution.
void PrintManager::stepA2_start( ) {
    _step = PrintStep::A2;

    debug( "+ PrintManager::stepA2_start: waiting for user to dispense print solution\n" );

    emit requestDispensePrintSolution( );
}

void PrintManager::stepA2_completed( ) {
    debug( "+ PrintManager::stepA2_completed\n" );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    stepA3_start( );
}

// A3. Lower to 50 mm.
void PrintManager::stepA3_start( ) {
    _step = PrintStep::A3;

    debug( "+ PrintManager::stepA3_start: lowering build platform to 5 mm\n" );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA3_completed );
    _shepherd->doMoveAbsolute( 5.00 );
}

void PrintManager::stepA3_completed( bool const success ) {
    debug( "+ PrintManager::stepA3_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA3_completed );

    if ( !success && !IsBadPrintResult( _printResult ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepA4_start( );
}

// A4. Configure print speed.
void PrintManager::stepA4_start( ) {
    _step = PrintStep::A4;

    debug( "+ PrintManager::stepA4_start: Setting print speed to %d mm/min\n", _printJob->printSpeed );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &PrintManager::stepA4_completed );
    _shepherd->doSend( QString { "G0 F%1" }.arg( _printJob->printSpeed ) );
}

void PrintManager::stepA4_completed( bool const success ) {
    debug( "+ PrintManager::stepA4_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_sendComplete, this, &PrintManager::stepA4_completed );

    if ( !success && !IsBadPrintResult( _printResult ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepA5_start( );
}

// A5. Lower to first layer height.
void PrintManager::stepA5_start( ) {
    _step = PrintStep::A5;

    auto firstLayerHeight = std::max( 100, _printJob->layerThickness ) / 1000.0;

    debug( "+ PrintManager::stepA5_start: lowering build platform to %.2f mm (layer thickness: %d µm)\n", firstLayerHeight, _printJob->layerThickness );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA5_completed );
    _shepherd->doMoveAbsolute( firstLayerHeight );
}

void PrintManager::stepA5_completed( bool const success ) {
    debug( "+ PrintManager::stepA5_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA5_completed );

    if ( !success && !IsBadPrintResult( _printResult ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepA6_start( );
}

// A6. Wait for four seconds.
void PrintManager::stepA6_start( ) {
    _step = PrintStep::A6;

    debug( "+ PrintManager::stepA6_start: pausing for %d ms\n", PauseAfterPrintSolutionLoad );

    _preProjectionTimer = _makeAndStartTimer( PauseAfterPrintSolutionLoad, &PrintManager::stepA6_completed );
}

void PrintManager::stepA6_completed( ) {
    debug( "+ PrintManager::stepA6_completed\n" );

    _stopAndCleanUpTimer( _preProjectionTimer );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    stepB1_start( );
}

// B1. Start projection: "setpower ${_printJob->powerLevel}".
void PrintManager::stepB1_start( ) {
    _step = PrintStep::B1;

    debug( "+ PrintManager::stepB1_start: running 'setpower %d'\n", _printJob->powerLevel );

    QString pngFileName = _printJob->jobWorkingDirectory + QString( "/%1.png" ).arg( _currentLayer, 6, 10, DigitZero );
    if ( !_pngDisplayer->setImageFileName( pngFileName ) ) {
        debug( "+ PrintManager::stepB1_start: PngDisplayer::setImageFileName failed for file %s\n", pngFileName.toUtf8( ).data( ) );
        this->abort( );
        return;
    }

    QObject::connect( _setpowerProcess, &ProcessRunner::succeeded, this, &PrintManager::stepB1_completed );
    QObject::connect( _setpowerProcess, &ProcessRunner::failed,    this, &PrintManager::stepB1_failed    );
    _setpowerProcess->start( SetpowerCommand, { QString( "%1" ).arg( _printJob->powerLevel ) } );

    emit startingLayer( _currentLayer );
}

void PrintManager::stepB1_completed( ) {
    debug( "+ PrintManager::stepB1_completed\n" );

    QObject::disconnect( _setpowerProcess, nullptr, this, nullptr );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    _lampOn = true;
    emit lampStatusChange( true );

    stepB2_start( );
}

void PrintManager::stepB1_failed( QProcess::ProcessError const ) {
    stepB1_completed( );
}

// B2. Pause for layer projection time (first two layers: scaled by scale factor)
void PrintManager::stepB2_start( ) {
    _step = PrintStep::B2;

    int layerProjectionTime = 1000.0 * _printJob->exposureTime * ( ( _currentLayer < 2 ) ? _printJob->exposureTimeScaleFactor : 1.0 );
    debug( "+ PrintManager::stepB2_start: pausing for %d ms\n", layerProjectionTime );

    _layerProjectionTimer = _makeAndStartTimer( layerProjectionTime, &PrintManager::stepB2_completed );
}

void PrintManager::stepB2_completed( ) {
    debug( "+ PrintManager::stepB2_completed\n" );

    _stopAndCleanUpTimer( _layerProjectionTimer );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    stepB3_start( );
}

// B3. Stop projection: "setpower 0".
void PrintManager::stepB3_start( ) {
    _step = PrintStep::B3;

    debug( "+ PrintManager::stepB3_start: running 'setpower 0'\n" );

    _pngDisplayer->clear( );

    QObject::connect( _setpowerProcess, &ProcessRunner::succeeded, this, &PrintManager::stepB3_completed );
    QObject::connect( _setpowerProcess, &ProcessRunner::failed,    this, &PrintManager::stepB3_failed    );
    _setpowerProcess->start( SetpowerCommand, { "0" } );
}

void PrintManager::stepB3_completed( ) {
    debug( "+ PrintManager::stepB3_completed\n" );

    QObject::disconnect( _setpowerProcess, nullptr, this, nullptr );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    _lampOn = false;
    emit lampStatusChange( false );

    stepB4_start( );
}

void PrintManager::stepB3_failed( QProcess::ProcessError const error ) {
    stepB3_completed( );
}

// B4. Pause before raise.
void PrintManager::stepB4_start( ) {
    _step = PrintStep::B4;

    debug( "+ PrintManager::stepB4_start: pausing for %d ms before raising build platform\n", PauseBeforeLift );

    _preLiftTimer = _makeAndStartTimer( PauseBeforeLift, &PrintManager::stepB4_completed );
}

void PrintManager::stepB4_completed( ) {
    debug( "+ PrintManager::stepB4_completed\n" );

    _stopAndCleanUpTimer( _preLiftTimer );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    stepB5_start( );
}

// B5. Raise the build platform by LiftDistance.
void PrintManager::stepB5_start( ) {
    _step = PrintStep::B5;

    if ( ++_currentLayer == _printJob->layerCount ) {
        debug( "+ PrintManager::stepB5_start: print complete\n" );

        _printResult = PrintResult::Success;

        stepC1_start( );
    } else {
        debug( "+ PrintManager::stepB5_start: raising build platform by %.2f mm\n", LiftDistance );

        QObject::connect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &PrintManager::stepB5_completed );
        _shepherd->doMoveRelative( LiftDistance );
    }
}

void PrintManager::stepB5_completed( bool const success ) {
    debug( "+ PrintManager::stepB5_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &PrintManager::stepB5_completed );

    if ( !success && !IsBadPrintResult( _printResult ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepB6_start( );
}

// B6. Lower the build platform by LiftDistance - LayerHeight.
void PrintManager::stepB6_start( ) {
    _step = PrintStep::B6;

    auto const moveDistance = -LiftDistance + _printJob->layerThickness / 1000.0;
    debug( "+ PrintManager::stepB6_start: lowering build platform by %.2f mm\n", moveDistance );

    QObject::connect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &PrintManager::stepB6_completed );
    _shepherd->doMoveRelative( moveDistance );
}

void PrintManager::stepB6_completed( bool const success ) {
    debug( "+ PrintManager::stepB6_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &PrintManager::stepB6_completed );

    if ( !success && !IsBadPrintResult( _printResult ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepB7_start( );
}

// B7. Pause before projection.
void PrintManager::stepB7_start( ) {
    _step = PrintStep::B7;

    debug( "+ PrintManager::stepB7_start: pausing for %d ms before projecting layer\n", PauseBeforeProject );

    _preProjectionTimer = _makeAndStartTimer( PauseBeforeProject, &PrintManager::stepB7_completed );
}

void PrintManager::stepB7_completed( ) {
    debug( "+ PrintManager::stepB7_completed\n" );

    _stopAndCleanUpTimer( _preProjectionTimer );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    stepB1_start( );
}

// C1. Configure print speed.
void PrintManager::stepC1_start( ) {
    _step = PrintStep::C1;

    if ( _lampOn ) {
        debug( "+ PrintManager::stepC1_start: Turning off lamp\n" );
        QProcess::startDetached( SetpowerCommand, { "0" } );
        _lampOn = false;
        emit lampStatusChange( false );
    }

    debug( "+ PrintManager::stepC1_start: Setting print speed to 200 mm/min\n" );

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &PrintManager::stepC1_completed );
    _shepherd->doSend( QString { "G0 F200" } );
}

void PrintManager::stepC1_completed( bool const success ) {
    debug( "+ PrintManager::stepC1_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_sendComplete, this, &PrintManager::stepC1_completed );

    if ( success ) {
        stepC2_start( );
    } else {
        _printResult = PrintResult::Failure;
        stepC2_completed( false );
    }

    stepC2_start( );
}

// C2. Raise the build platform to maximum Z.
void PrintManager::stepC2_start( ) {
    _step = PrintStep::C2;

    debug( "+ PrintManager::stepC2_start: raising build platform to maximum Z\n" );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepC2_completed );
    _shepherd->doMoveAbsolute( PrinterRaiseToMaxZHeight );
}

void PrintManager::stepC2_completed( bool const success ) {
    debug( "+ PrintManager::stepC2_completed: action %s. print result %s\n", SucceededString( success ), PrintResultStrings[+_printResult] );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepC2_completed );

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
    _printResult = PrintResult::None;
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
    if ( _step == PrintStep::A2 ) {
        debug( "  + Aborting print solution load prompt\n" );
        stepA2_completed( );
        return;
    }
    if ( _step == PrintStep::none ) {
        debug( "  + Going directly to step C1\n" );
        stepC1_start( );
    } else {
        debug( "  + Waiting on current step to stop\n" );
    }
}

void PrintManager::printSolutionLoaded( ) {
    stepA2_completed( );
}
