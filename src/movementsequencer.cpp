#include "pch.h"

#include "movementsequencer.h"
#include "shepherd.h"

MovementSequencer::MovementSequencer( Shepherd* shepherd, QObject* parent ):
    QObject   { parent             },
    _shepherd { shepherd           },
    _timer    { new QTimer( this ) }
{
    _timer->setSingleShot( true );
    _timer->setTimerType( Qt::PreciseTimer );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &MovementSequencer::shepherd_moveAbsoluteComplete );
    QObject::connect( _shepherd, &Shepherd::action_moveRelativeComplete, this, &MovementSequencer::shepherd_moveRelativeComplete );
    QObject::connect( _timer,    &QTimer::timeout,                       this, &MovementSequencer::timer_timeout                 );
}

MovementSequencer::~MovementSequencer( ) {
    if ( _shepherd ) {
        QObject::disconnect( _shepherd, nullptr, this, nullptr );
        _shepherd = nullptr;
    }

    if ( _timer ) {
        QObject::disconnect( _timer, nullptr, this, nullptr );
        _timer->stop( );
        _timer->deleteLater( );
        _timer = nullptr;
    }
}

void MovementSequencer::_startNextMovement( ) {
    if ( _movements.isEmpty( ) ) {
        debug( "+ MovementSequencer::_startNextMovement: done\n" );
        emit movementComplete( true );
        return;
    }

    auto movement { _movements.takeFirst( ) };
    switch ( movement.type ) {
        case MovementInfo::moveAbsolute:
            debug( "+ MovementSequencer::_startNextMovement: starting Move Absolute: distance %.2f mm, speed %.2f mm/min\n", movement.distance, movement.speed );

            _shepherd->doMoveAbsolute( movement.distance, movement.speed );
            break;

        case MovementInfo::moveRelative:
            debug( "+ MovementSequencer::_startNextMovement: starting Move Relative: distance %.2f mm, speed %.2f mm/min\n", movement.distance, movement.speed );

            _shepherd->doMoveRelative( movement.distance, movement.speed );
            break;

        case MovementInfo::delay:
            debug( "+ MovementSequencer::_startNextMovement: starting Delay: duration %d ms\n", movement.distance, movement.speed );

            _timer->setInterval( movement.duration );
            _timer->start( );
            break;
    }
}

void MovementSequencer::shepherd_moveAbsoluteComplete( bool const success ) {
    debug( "+ MovementSequencer::shepherd_moveAbsoluteComplete: success? %s\n", YesNoString( success ) );

    if ( success ) {
        _startNextMovement( );
    } else {
        emit movementComplete( false );
    }
}

void MovementSequencer::shepherd_moveRelativeComplete( bool const success ) {
    debug( "+ MovementSequencer::shepherd_moveRelativeComplete: success? %s\n", YesNoString( success ) );

    if ( success ) {
        _startNextMovement( );
    } else {
        emit movementComplete( false );
    }
}

void MovementSequencer::timer_timeout( ) {
    debug( "+ MovementSequencer::timer_timeout\n" );
    _timer->stop( );

    _startNextMovement( );
}

void MovementSequencer::addMovement( MoveType const type, double const distance, double const speed ) {
    _movements.push_back( { type, distance, speed } );
}

void MovementSequencer::addDelay( int const duration ) {
    _movements.push_back( { duration } );
}

void MovementSequencer::execute( ) {
    _startNextMovement( );
}

void MovementSequencer::setMovements( MovementCollection const& movements ) {
    _movements.clear( );
    _movements.append( movements );
}
