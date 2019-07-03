#include "pch.h"

#include <bitset>

#include "signalhandler.h"

namespace {

    std::bitset<_NSIG> subscribedSignals;

    int signalFds[2] { };

}

SignalHandler::SignalHandler( QObject* parent ): QObject( parent ) {
    if ( ::socketpair( AF_UNIX, SOCK_DGRAM, 0, signalFds ) ) {
        auto err = errno;
        debug( "+ SignalHandler::`ctor: socketpair failed: %s [%d]", strerror( err ), err );
        return;
    }

    _signalNotifier = new QSocketNotifier( signalFds[1], QSocketNotifier::Read, this );
    QObject::connect( _signalNotifier, &QSocketNotifier::activated, this, &SignalHandler::dispatchSignal );
}

SignalHandler::~SignalHandler( ) {
    _cleanUp( );
}

void SignalHandler::subscribe( int const signalNumber ) {
    if ( subscribedSignals[signalNumber] ) {
        return;
    }

    struct sigaction sigact { };
    sigact.sa_sigaction = &SignalHandler::_signalAction;
    sigact.sa_flags     = SA_SIGINFO | SA_RESTART;
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

void SignalHandler::_cleanUp( ) {
    for ( int signum = 0; signum < _NSIG; ++signum ) {
        unsubscribe( signum );
    }

    QObject::disconnect( _signalNotifier );
    _signalNotifier->deleteLater( );
    _signalNotifier = nullptr;

    ::close( signalFds[0] );
    ::close( signalFds[1] );
}

void SignalHandler::_signalAction( int signalNumber, siginfo_t* info, void* ucontext ) {
    auto rc = ::write( signalFds[0], info, sizeof( siginfo_t ) );
    if ( -1 == rc ) {
        error_t err = errno;
        debug( "+ SignalHandler::_signalAction: error calling write: %s [%d]\n", strerror( err ), err );
    } else if ( 0 == rc ) {
        debug( "+ SignalHandler::_signalAction: end of file?\n" );
    } else if ( static_cast<size_t>( rc ) != sizeof( siginfo_t ) ) {
        debug( "+ SignalHandler::_signalAction: wrong message write length? expected %zu bytes, got %ld\n", sizeof( siginfo_t ), rc );
    }
}

void SignalHandler::dispatchSignal( int ) {
    _signalNotifier->setEnabled( false );

    siginfo_t info;
    auto rc = ::read( signalFds[1], &info, sizeof( siginfo_t ) );
    if ( -1 == rc ) {
        error_t err = errno;
        debug( "+ SignalHandler::dispatchSignal: error calling read: %s [%d]\n", strerror( err ), err );
    } else if ( 0 == rc ) {
        debug( "+ SignalHandler::dispatchSignal: end of file?\n" );
    } else if ( static_cast<size_t>( rc ) != sizeof( siginfo_t ) ) {
        debug( "+ SignalHandler::dispatchSignal: wrong message length? expected %zu bytes, got %ld\n", sizeof( siginfo_t ), rc );
    } else {
        emit signalReceived( info );
    }

    _signalNotifier->setEnabled( true );
}
