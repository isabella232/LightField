#include "pch.h"

#include "commandreader.h"

namespace {

    QRegularExpression const WhitespaceRegex { "\\s+" };
    QChar              const Colon           { ':'    };

}

CommandReader::CommandReader( QObject* parent ): QObject( parent ) {
    _thread = QThread::create( std::bind( &CommandReader::_readCommands, this ) );
    _thread->setParent( this );
    _thread->start( );
}

CommandReader::~CommandReader( ) {
    if ( _thread ) {
        ::close( 0 );
        _thread->wait( );
        _thread->deleteLater( );
        _thread = nullptr;
    }
}

void CommandReader::_readCommands( ) {
    auto standardInput { new QFile };
    if ( !standardInput->open( 0, QFileDevice::ReadOnly, QFileDevice::DontCloseHandle ) ) {
        debug( "+ CommandReader::_readCommands: couldn't reopen stdin\n" );
        standardInput->deleteLater( );
        ::exit( 1 );
        return;
    }

    qint64 bytesRead;
    do {
        char buf[1024];
        bytesRead = standardInput->readLine( buf, std::size( buf ) );
        if ( bytesRead >= 1 ) {
            if ( auto line = QString::fromUtf8( buf, bytesRead ).trimmed( ); !line.isEmpty( ) ) {
                emit commandReceived( line.split( Colon, QString::SkipEmptyParts ) );
                if ( line == "terminate" ) {
                    break;
                }
            }
        }
    } while ( bytesRead > -1 );
    ::exit( 0 );
}
