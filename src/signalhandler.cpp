#include "pch.h"

#include "signalhandler.h"

#include "window.h"

#define _countof(x) (sizeof(x) / sizeof(x[0]))

namespace {

    int const QuitSignalList[] { SIGHUP, SIGINT, SIGQUIT, SIGTERM };

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

    struct sigaction sigact { };
    sigact.sa_handler = &SignalHandler::signalHandler;
    sigact.sa_flags   = SA_RESTART;
    ::sigemptyset( &sigact.sa_mask );

    int failureCount { };
    for ( int signalNumber : QuitSignalList ) {
        if ( ::sigaction( signalNumber, &sigact, nullptr ) ) {
            auto err = errno;
            debug( "+ SignalHandler::`ctor: Signal %d: sigaction failed: %s [%d]", signalNumber, strerror( err ), err );
            failureCount++;
        }
    }

    if ( failureCount == _countof( QuitSignalList ) ) {
        cleanUp( );
        return;
    }
}

SignalHandler::~SignalHandler( ) {
    cleanUp( );
}

void SignalHandler::cleanUp( ) {
    for ( int signum : QuitSignalList ) {
        signal( signum, SIG_DFL );
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

    emit quit( signalNumber );

    signalNotifier->setEnabled( true );
}
