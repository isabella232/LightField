#include "pch.h"

#include "movementsequencer.h"
#include "firmwarecontroller.h"

MovementSequencer::MovementSequencer(FirmwareController* controller, QObject* parent):
    QObject   { parent },
    _firmwareController { controller },
    _timer    { new QTimer(this) }
{
    _timer->setSingleShot( true );
    _timer->setTimerType( Qt::PreciseTimer );

    QObject::connect(_firmwareController, &FirmwareController::printerMoveAbsoluteCompleted, this,
        &MovementSequencer::moveAbsoluteComplete);
    QObject::connect(_firmwareController, &FirmwareController::printerMoveRelativeCompleted, this,
        &MovementSequencer::moveRelativeComplete);
    QObject::connect(_timer, &QTimer::timeout, this, &MovementSequencer::timer_timeout);
}

MovementSequencer::~MovementSequencer() {
    if (_firmwareController) {
        QObject::disconnect(_firmwareController, nullptr, this, nullptr );
        _firmwareController = nullptr;
    }

    if ( _timer ) {
        QObject::disconnect( _timer, nullptr, this, nullptr );
        _timer->stop( );
        _timer->deleteLater( );
        _timer = nullptr;
    }
}

void MovementSequencer::_startNextMovement( ) {
    if ( _aborting ) {
        debug( "+ MovementSequencer::_startNextMovement: aborting sequence\n" );

        _active = false;
        emit movementComplete( false );
        return;
    }

    if ( _movements.isEmpty( ) ) {
        debug( "+ MovementSequencer::_startNextMovement: done\n" );

        _active = false;
        emit movementComplete( true );
        return;
    }

    auto movement { _movements.takeFirst( ) };
    switch ( movement.type ) {
        case MovementInfo::moveAbsolute:
            debug( "+ MovementSequencer::_startNextMovement: starting Move Absolute: distance %.2f mm, speed %.2f mm/min\n", movement.distance, movement.speed );

            _firmwareController->moveAbsolute(movement.distance, movement.speed);
            break;

        case MovementInfo::moveRelative:
            debug( "+ MovementSequencer::_startNextMovement: starting Move Relative: distance %.2f mm, speed %.2f mm/min\n", movement.distance, movement.speed );

            _firmwareController->moveRelative(movement.distance, movement.speed);
            break;

        case MovementInfo::delay:
            debug( "+ MovementSequencer::_startNextMovement: starting Delay: duration %d ms\n", movement.distance, movement.speed );

            _timer->stop( );
            _timer->setInterval( movement.duration );
            _timer->start( );
            break;

        default:
            debug( "+ MovementSequencer::_startNextMovement: unknown movement type, aborting\n" );

            abort( );
            break;
    }
}

void MovementSequencer::abort( ) {
    debug( "+ MovementSequencer::abort: aborting sequence; active? %s; aborting already? %s; timer active? %s\n", YesNoString( _active ), YesNoString( _aborting ), YesNoString( _timer->isActive( ) ) );

    _aborting = true;

    if ( !_active ) {
        emit movementComplete( false );
        return;
    }

    if ( _timer->isActive( ) ) {
        _timer->stop( );

        _active = false;
        emit movementComplete( false );
    }
}

void MovementSequencer::moveAbsoluteComplete( bool const success ) {
    debug( "+ MovementSequencer::moveAbsoluteComplete: success? %s\n", YesNoString( success ) );

    if ( success ) {
        _startNextMovement( );
    } else {
        abort( );
    }
}

void MovementSequencer::moveRelativeComplete( bool const success ) {
    debug( "+ MovementSequencer::moveRelativeComplete: success? %s\n", YesNoString( success ) );

    if ( success ) {
        _startNextMovement( );
    } else {
        abort( );
    }
}

void MovementSequencer::timer_timeout( ) {
    debug( "+ MovementSequencer::timer_timeout\n" );

    _timer->stop( );
    _startNextMovement( );
}
