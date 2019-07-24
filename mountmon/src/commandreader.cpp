#include "pch.h"

#include "commandreader.h"

namespace {

    QRegularExpression const WhitespaceRegex { "\\s+" };
    QChar              const FieldSeparator  { ':'    };

}

CommandReader::CommandReader( QObject* parent ): QObject( parent ) {
    (void) QObject::connect( this, &CommandReader::_commandReceived, this, &CommandReader::thread_commandReceived, Qt::QueuedConnection );

    _thread = QThread::create( [this] ( ) {
        auto standardInput { new QFile };
        if ( !standardInput->open( 0, QFileDevice::ReadOnly, QFileDevice::DontCloseHandle ) ) {
            debug( "+ CommandReader::`ctor: couldn't reopen stdin\n" );
            standardInput->deleteLater( );
            qApp->exit( 1 );
            return;
        }

        qint64 bytesRead;
        char   buf[4096];
        do {
            bytesRead = standardInput->readLine( buf, 4096 );
            if ( bytesRead >= 1 ) {
				if ( '\n' == buf[bytesRead - 1] ) {
					--bytesRead;
					buf[bytesRead] = '\0';
				}
                if ( bytesRead > 0 ) {
                    emit _commandReceived( buf );
                }
                if ( 0 == strcmp( buf, "terminate" ) ) {
                    break;
                }
            }
        } while ( bytesRead > -1 );
    } );
    _thread->setParent( this );
    _thread->start( );
}

CommandReader::~CommandReader( ) {
    if ( _thread ) {
        _thread->deleteLater( );
        _thread = nullptr;
    }
}

void CommandReader::thread_commandReceived( QString const command ) {
    auto tokens = command.split( FieldSeparator, QString::SkipEmptyParts );
    emit commandReceived( tokens );
}
