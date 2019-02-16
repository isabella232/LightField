#include "pch.h"

#include <bitset>

#include "signalhandler.h"

#include "window.h"

#define _countof(x) (sizeof(x) / sizeof(x[0]))

namespace {

    std::bitset<_NSIG> subscribedSignals;

    int signalFds[2] { };

}

SignalHandler* g_signalHandler;

SignalHandler::SignalHandler( QObject* parent ): QObject( parent ) {
    if ( ::socketpair( AF_UNIX, SOCK_STREAM, 0, signalFds ) ) {
        auto err = errno;
        debug( "+ SignalHandler::`ctor: socketpair failed: %s [%d]", strerror( err ), err );
        return;
    }

    signalNotifier = new QSocketNotifier( signalFds[1], QSocketNotifier::Read, this );
    QObject::connect( signalNotifier, &QSocketNotifier::activated, this, &SignalHandler::dispatchSignal );
}

SignalHandler::~SignalHandler( ) {
    cleanUp( );
}

void SignalHandler::subscribe( int const signalNumber ) {
    if ( subscribedSignals[signalNumber] ) {
        return;
    }

    struct sigaction sigact { };
    sigact.sa_handler = &SignalHandler::signalHandler;
    sigact.sa_flags   = SA_RESTART;
    ::sigemptyset( &sigact.sa_mask );

    if ( ::sigaction( signalNumber, &sigact, nullptr ) ) {
        auto err = errno;
        debug( "+ SignalHandler::subscribe: Signal %d: sigaction failed: %s [%d]", signalNumber, strerror( err ), err );
    } else {
        subscribedSignals[signalNumber] = true;
    }
}

void SignalHandler::subscribe( std::initializer_list<int> signums ) {
    for ( auto signum : signums ) {
        subscribe( signum );
    }
}

void SignalHandler::unsubscribe( int const signalNumber ) {
    if ( !subscribedSignals[signalNumber] ) {
        return;
    }

    signal( signalNumber, SIG_DFL );
    subscribedSignals[signalNumber] = false;
}

void SignalHandler::unsubscribe( std::initializer_list<int> signums ) {
    for ( auto signum : signums ) {
        unsubscribe( signum );
    }
}

void SignalHandler::cleanUp( ) {
    for ( int signum = 0; signum < _NSIG; ++signum ) {
        unsubscribe( signum );
    }

    QObject::disconnect( signalNotifier, nullptr, this, nullptr );
    signalNotifier->deleteLater( );
    signalNotifier = nullptr;

    ::close( signalFds[0] );
    ::close( signalFds[1] );
}

void SignalHandler::signalHandler( int signum ) {
    char signalNumber { static_cast<char>( signum ) };
    ::write( signalFds[0], &signalNumber, 1 );
}

void SignalHandler::dispatchSignal( int ) {
    signalNotifier->setEnabled( false );

    char signalNumber;
    ::read( signalFds[1], &signalNumber, 1 );

    emit signalReceived( signalNumber );

    signalNotifier->setEnabled( true );
}
