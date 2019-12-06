#include "pch.h"

#include "printmanager.h"

#include "pngdisplayer.h"
#include "printjob.h"
#include "processrunner.h"
#include "shepherd.h"
#include "timinglogger.h"

//
// Before printing:
//
// A1. Raise the build platform to maximum Z at high speed.
// A2. Prompt user to dispense print solution.
// A3. Lower to high-speed threshold Z at high speed.
// A4. Lower to first layer height at low speed.
// A5. Wait for four seconds.
//
// For each layer:
//
// B1. Start projection: "set-projector-power ${powerLevel}".
// --- no-pause region starts
// B2. Pause for layer projection time (first two layers: scaled by scale factor)
// B3. Stop projection: "set-projector-power 0".
// B4. Pause before raise.
// B5. Raise the build platform by LiftDistance at low speed.
// --- no-pause region ends
// B6. Lower the build platform by LiftDistance - LayerHeight at low speed.
// B7. Pause before projection.
//
// After printing, whether successful or not:
//
// C1. Raise the build platform to maximum Z.
//

namespace {

    auto const PauseAfterPrintSolutionDispensed = 4000; // ms
    auto const PauseBeforeProject               = 4000; // ms
    auto const PauseBeforeLift                  = 2000; // ms
    auto const LiftDistance                     = 2.00; // mm

    char const* PrintStepStrings[] {
        "none",
        "A1", "A2", "A3", "A4", "A5",
        "B1", "B2", "B3", "B4", "B5", "B6", "B7",
        "C1", "C2",
        "D1", "D2", "D3", "D4",
    };

    bool IsBadPrintResult( PrintResult const printResult ) {
        return printResult < PrintResult::None;
    }

    char const* ToString( PrintStep const value ) {
#if defined _DEBUG
        if ( ( value >= PrintStep::none ) && ( value <= PrintStep::C1 ) ) {
#endif
            return PrintStepStrings[static_cast<int>( value )];
#if defined _DEBUG
        } else {
            return nullptr;
        }
#endif
    }

}

PrintManager::PrintManager( Shepherd* shepherd, QObject* parent ):
    QObject   ( parent   ),
    _shepherd ( shepherd )
{
    _setProjectorPowerProcess = new ProcessRunner( this );
}

PrintManager::~PrintManager( ) {
    /*empty*/
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

    if ( _setProjectorPowerProcess ) {
        if ( _setProjectorPowerProcess->state( ) != QProcess::NotRunning ) {
            _setProjectorPowerProcess->kill( );
        }
        _setProjectorPowerProcess->deleteLater( );
        _setProjectorPowerProcess = nullptr;
    }
}

// A1. Raise the build platform to maximum Z at high speed.
void PrintManager::stepA1_start( ) {
    _step = PrintStep::A1;

    debug( "+ PrintManager::stepA1_start: raising build platform to %.2f mm\n", PrinterRaiseToMaximumZ );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA1_completed );
    _shepherd->doMoveAbsolute( PrinterRaiseToMaximumZ, PrinterDefaultHighSpeed );
}

void PrintManager::stepA1_completed( bool const success ) {
    debug( "+ PrintManager::stepA1_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA1_completed );

    if ( ( _printResult != PrintResult::Abort ) && !success ) {
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

// A3. Lower to high-speed threshold Z at high speed.
void PrintManager::stepA3_start( ) {
    _step = PrintStep::A3;

    debug( "+ PrintManager::stepA3_start: lowering build platform to %.2f mm\n", PrinterHighSpeedThresholdZ );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA3_completed );
    _shepherd->doMoveAbsolute( PrinterHighSpeedThresholdZ, PrinterDefaultHighSpeed );
}

void PrintManager::stepA3_completed( bool const success ) {
    debug( "+ PrintManager::stepA3_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA3_completed );

    if ( !success && ( _printResult != PrintResult::Abort ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepA4_start( );
}

// A4. Lower to first layer height at low speed.
void PrintManager::stepA4_start( ) {
    _step = PrintStep::A4;

    auto firstLayerHeight = ( std::max( 100, _printJob->layerThickness ) + g_settings.buildPlatformOffset ) / 1000.0;

    debug( "+ PrintManager::stepA4_start: lowering build platform to %.2f mm (layer thickness: %d µm)\n", firstLayerHeight, _printJob->layerThickness );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA4_completed );
    _shepherd->doMoveAbsolute( firstLayerHeight, _printJob->printSpeed );
}

void PrintManager::stepA4_completed( bool const success ) {
    debug( "+ PrintManager::stepA4_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepA4_completed );

    if ( !success && ( _printResult != PrintResult::Abort ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepA5_start( );
}

// A5. Wait for four seconds.
void PrintManager::stepA5_start( ) {
    _step = PrintStep::A5;

    debug( "+ PrintManager::stepA5_start: pausing for %d ms\n", PauseAfterPrintSolutionDispensed );

    _preProjectionTimer = _makeAndStartTimer( PauseAfterPrintSolutionDispensed, &PrintManager::stepA5_completed );
}

void PrintManager::stepA5_completed( ) {
    debug( "+ PrintManager::stepA5_completed\n" );

    _stopAndCleanUpTimer( _preProjectionTimer );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    emit printPausable( true );

    stepB1_start( );
}

// B1. Start projection: "set-projector-power ${_printJob->powerLevel}".
void PrintManager::stepB1_start( ) {
    _step = PrintStep::B1;
    if ( _paused ) {
        emit printPaused( );
        return;
    }

    debug( "+ PrintManager::stepB1_start: running 'set-projector-power %d'\n", _printJob->powerLevel );

    QString pngFileName = _printJob->jobWorkingDirectory + QString( "/%1.png" ).arg( _currentLayer, 6, 10, DigitZero );
    if ( !_pngDisplayer->loadImageFile( pngFileName ) ) {
        debug( "+ PrintManager::stepB1_start: PngDisplayer::loadImageFile failed for file %s\n", pngFileName.toUtf8( ).data( ) );
        this->abort( );
        return;
    }

    QObject::connect( _setProjectorPowerProcess, &ProcessRunner::succeeded, this, &PrintManager::stepB1_completed );
    QObject::connect( _setProjectorPowerProcess, &ProcessRunner::failed,    this, &PrintManager::stepB1_failed    );
    _setProjectorPowerProcess->start( SetProjectorPowerCommand, { QString( "%1" ).arg( _printJob->powerLevel ) } );

    emit startingLayer( _currentLayer );
}

void PrintManager::stepB1_completed( ) {
    debug( "+ PrintManager::stepB1_completed\n" );

    QObject::disconnect( _setProjectorPowerProcess, nullptr, this, nullptr );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    _lampOn = true;
    emit lampStatusChange( true );

    stepB2_start( );
}

void PrintManager::stepB1_failed( int const, QProcess::ProcessError const ) {
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

// B3. Stop projection: "set-projector-power 0".
void PrintManager::stepB3_start( ) {
    _step = PrintStep::B3;

    debug( "+ PrintManager::stepB3_start: running 'set-projector-power 0'\n" );

    _pngDisplayer->clear( );

    QObject::connect( _setProjectorPowerProcess, &ProcessRunner::succeeded, this, &PrintManager::stepB3_completed );
    QObject::connect( _setProjectorPowerProcess, &ProcessRunner::failed,    this, &PrintManager::stepB3_failed    );
    _setProjectorPowerProcess->start( SetProjectorPowerCommand, { "0" } );
}

void PrintManager::stepB3_completed( ) {
    debug( "+ PrintManager::stepB3_completed\n" );

    QObject::disconnect( _setProjectorPowerProcess, nullptr, this, nullptr );

    if ( _printResult == PrintResult::Abort ) {
        stepC1_start( );
        return;
    }

    _lampOn = false;
    emit lampStatusChange( false );

    stepB4_start( );
}

void PrintManager::stepB3_failed( int const, QProcess::ProcessError const ) {
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

// B5. Raise the build platform by LiftDistance at low speed.
void PrintManager::stepB5_start( ) {
    _step = PrintStep::B5;

    if ( ++_currentLayer == _printJob->layerCount ) {
        debug( "+ PrintManager::stepB5_start: print complete\n" );

        _printResult = PrintResult::Success;

        stepC1_start( );
    } else {
        debug( "+ PrintManager::stepB5_start: raising build platform by %.2f mm\n", LiftDistance );

        QObject::connect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &PrintManager::stepB5_completed );
        _shepherd->doMoveRelative( LiftDistance, _printJob->printSpeed );
    }
}

void PrintManager::stepB5_completed( bool const success ) {
    debug( "+ PrintManager::stepB5_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &PrintManager::stepB5_completed );

    if ( !success && ( _printResult != PrintResult::Abort ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( IsBadPrintResult( _printResult ) ) {
        stepC1_start( );
        return;
    }

    stepB6_start( );
}

// B6. Lower the build platform by LiftDistance - LayerHeight at low speed.
void PrintManager::stepB6_start( ) {
    _step = PrintStep::B6;
    if ( _paused ) {
        emit printPaused( );
        return;
    }

    auto const moveDistance = -LiftDistance + _printJob->layerThickness / 1000.0;
    debug( "+ PrintManager::stepB6_start: lowering build platform by %.2f mm\n", moveDistance );

    QObject::connect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &PrintManager::stepB6_completed );
    _shepherd->doMoveRelative( moveDistance, _printJob->printSpeed );
}

void PrintManager::stepB6_completed( bool const success ) {
    debug( "+ PrintManager::stepB6_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &PrintManager::stepB6_completed );

    if ( !success && ( _printResult != PrintResult::Abort ) ) {
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
    if ( _paused ) {
        emit printPaused( );
        return;
    }

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

// C1. Raise the build platform to high-speed threshold Z at low speed.
void PrintManager::stepC1_start( ) {
    _step = PrintStep::C1;
    emit printPausable( false );

    if ( _lampOn ) {
        debug( "+ PrintManager::stepC1_start: Turning off lamp\n" );

        QProcess::startDetached( SetProjectorPowerCommand, { "0" } );
        _lampOn = false;
        emit lampStatusChange( false );
    }

    double threshold = std::min( PrinterRaiseToMaximumZ, PrinterHighSpeedThresholdZ + _printJob->layerCount * _printJob->layerThickness / 1000.0 );
    debug( "+ PrintManager::stepC1_start: raising build platform to threshold Z, %.2f mm\n", threshold );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepC1_completed );
    _shepherd->doMoveAbsolute( threshold, PrinterDefaultHighSpeed );
}

void PrintManager::stepC1_completed( bool const success ) {
    debug( "+ PrintManager::stepC1_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepC1_completed );

    if ( !success && ( _printResult != PrintResult::Abort ) ) {
        _printResult = PrintResult::Failure;
    }
    if ( !success ) {
        if ( PrintResult::Abort == _printResult ) {
            emit printAborted( );
        } else {
            emit printComplete( _printResult == PrintResult::Success );
        }
        return;
    }

    stepC2_start( );
}

void PrintManager::stepC2_start( ) {
    _step = PrintStep::C1;

    debug( "+ PrintManager::stepC2_start: raising build platform to maximum Z\n" );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepC2_completed );
    _shepherd->doMoveAbsolute( PrinterRaiseToMaximumZ, PrinterDefaultHighSpeed );
}

void PrintManager::stepC2_completed( bool const success ) {
    TimingLogger::stopTiming( TimingId::Printing );
    debug( "+ PrintManager::stepC2_completed: action %s\n", SucceededString( success ) );

    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintManager::stepC2_completed );

    if ( PrintResult::None == _printResult ) {
        _printResult = PrintResult::Success;
    }

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

    debug( "+ PrintManager::print: new job %p\n", printJob );
    _printJob = printJob;

    _pngDisplayer->clear( );

    TimingLogger::startTiming( TimingId::Printing, GetFileBaseName( _printJob->modelFileName ) );
    debug( "+ PrintManager::print: emitting printStarting()\n" );
    _printResult = PrintResult::None;
    emit printStarting( );
    stepA1_start( );
}

void PrintManager::pause( ) {
    debug( "+ PrintManager::pause\n" );
    if ( _paused ) {
        return;
    }

    _paused = true;
}

void PrintManager::resume( ) {
    debug( "+ PrintManager::resume\n" );
    if ( !_paused ) {
        return;
    }

    _paused = false;
    switch ( _step ) {
        case PrintStep::B1: stepB1_start( ); break;
        case PrintStep::B6: stepB6_start( ); break;
        case PrintStep::B7: stepB7_start( ); break;
        default:
            debug( "+ PrintManager::resume: paused at invalid step %s\n", ToString( _step ) );
            this->abort( );
            return;
    }
    emit printResumed( );
}

void PrintManager::terminate( ) {
    debug( "+ PrintManager::terminate\n" );
    _cleanUp( );
}

void PrintManager::abort( ) {
    debug( "+ PrintManager::abort: current step is %s; paused? %s\n", ToString( _step ), YesNoString( _paused ) );

    _printResult = PrintResult::Abort;

    if ( _paused ) {
        debug( "  + Printer is paused, going directly to step C1\n" );
        stepC1_start( );
    }

    switch ( _step ) {
        case PrintStep::none:
            debug( "  + Going directly to step C1\n" );
            stepC1_start( );
            break;

        case PrintStep::A2:
            debug( "  + Aborting print solution dispense prompt\n" );
            stepA2_completed( );
            break;

        case PrintStep::A5:
            debug( "  + Interrupting initial pre-projection timer\n" );
            _stopAndCleanUpTimer( _preProjectionTimer );
            stepA5_completed( );
            break;

        case PrintStep::B4:
            debug( "  + Interrupting pre-lift timer\n" );
            _stopAndCleanUpTimer( _preLiftTimer );
            stepB4_completed( );
            break;

        case PrintStep::B7:
            debug( "  + Interrupting pre-projection timer\n" );
            _stopAndCleanUpTimer( _preProjectionTimer );
            stepB7_completed( );
            break;

        default:
            debug( "  + Waiting on current step to stop\n" );
            break;
    }
}

void PrintManager::setPngDisplayer( PngDisplayer* pngDisplayer ) {
    _pngDisplayer = pngDisplayer;
}

void PrintManager::printSolutionDispensed( ) {
    stepA2_completed( );
}
