#include "pch.h"

#include "shepherd.h"
#include "window.h"

#include "strings.h"

namespace {

    char const* ShepherdBaseDirectory = "/home/lumen/Volumetric/fstl/stdio-shepherd";

}

Shepherd::Shepherd( QObject* parent ): QObject( parent ) {
    debug( "+ Shepherd::`ctor: Shepherd base directory: '%s'\n", ShepherdBaseDirectory );

    _process = new QProcess( parent );

    auto env = _process->processEnvironment( );
    if ( env.isEmpty( ) ) {
        env = QProcessEnvironment::systemEnvironment( );
    }
    env.insert( "PYTHONUNBUFFERED", "x" );
    _process->setProcessEnvironment( env );
    _process->setWorkingDirectory( ShepherdBaseDirectory );

    QObject::connect( _process, &QProcess::errorOccurred,           this, &Shepherd::processErrorOccurred           );
    QObject::connect( _process, &QProcess::started,                 this, &Shepherd::processStarted                 );
    QObject::connect( _process, &QProcess::readyReadStandardError,  this, &Shepherd::processReadyReadStandardError  );
    QObject::connect( _process, &QProcess::readyReadStandardOutput, this, &Shepherd::processReadyReadStandardOutput );
    QObject::connect( _process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Shepherd::processFinished );
}

Shepherd::~Shepherd( ) {
    /*empty*/
}

void Shepherd::processErrorOccurred( QProcess::ProcessError error ) {
    debug( "+ Shepherd::processErrorOccurred: error %s [%d]\n", ToString( error ), error );
    emit shepherd_processError( error );
}

void Shepherd::processStarted( ) {
    debug( "+ Shepherd::processStarted\n" );
    emit shepherd_started( );
}

void Shepherd::processReadyReadStandardError( ) {
    _process->setReadChannel( QProcess::StandardError );
    QString input = _process->readAllStandardError( );
    if ( input.length( ) ) {
        fprintf( stderr,
            "+ Shepherd::processReadyReadStandardError: from stderr:\n"
            "%s\n",
            input.toUtf8( ).data( )
        );
    }
}

void Shepherd::processReadyReadStandardOutput( ) {
    _process->setReadChannel( QProcess::StandardOutput );

    QString input = _process->readAllStandardOutput( );
    if ( input.length( ) ) {
        handleInput( input );
    }
}

void Shepherd::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    debug( "+ Shepherd::processFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );
    emit shepherd_finished( exitCode, exitStatus );
}

QStringList Shepherd::splitLine( QString const& line ) {
    QStringList pieces;
    QString piece;
    bool inQuote = false;

    int length = line.length( );
    int index = 0;
    while ( index < length ) {
        QChar ch = line[index];
        switch ( ch.unicode( ) ) {
            case L' ':
                if ( inQuote ) {
                    piece += ch;
                } else {
                    pieces += piece;
                    piece.clear( );
                }
                break;

            case L'"':
                inQuote = !inQuote;
                break;

            case L'\\':
                if ( inQuote ) {
                    if ( ( index + 1 ) < length ) {
                        piece += line[++index];
                    }
                } else {
                    piece += ch;
                }
                break;

            default:
                piece += ch;
                break;
        }
        ++index;
    }
    if ( !piece.isEmpty( ) ) {
        pieces += piece;
    }

    return pieces;
}

void Shepherd::handleFromPrinter( QString const& input ) {
    debug( "+ Shepherd::handleFromPrinter: input='%s' pendingCommand=%s [%d]\n", input.toUtf8( ).data( ), ToString( _pendingCommand ), static_cast<int>( _pendingCommand ) );
    if ( input == "ok" ) {
        switch ( _pendingCommand ) {
            case PendingCommand::move:
                ++_okCount;
                if ( 5 == _okCount ) {
                    _pendingCommand = PendingCommand::none;
                    emit action_moveComplete( true );
                }
                break;

            case PendingCommand::moveTo:
                _pendingCommand = PendingCommand::none;
                emit action_moveToComplete( true );
                break;

            case PendingCommand::home:
                ++_okCount;
                if ( 1 == _okCount ) {
                    _pendingCommand = PendingCommand::none;
                    emit action_homeComplete( true );
                }
                break;

            case PendingCommand::none:
                debug( "+ Shepherd::handleFromPrinter: *no* pending command??\n" );
                break;

            default:
                debug( "+ Shepherd::handleFromPrinter: unknown pending command\n" );
                break;
        }
    }
}

void Shepherd::handleInput( QString const& input ) {
    _buffer += input;

    auto lines = _buffer.split( "\n", QString::SplitBehavior::KeepEmptyParts );

    // if the buffer doesn't end with a newline character, then the last line is
    // not yet complete. put it back in the buffer and forget about it for now.
    if ( !_buffer.endsWith( "\n" ) ) {
        _buffer = lines.last( );
        lines.removeLast( );
    } else {
        _buffer.clear( );
    }

    for ( auto line : lines ) {
        if ( line.endsWith( "\r" ) ) {
            line.resize( line.length( ) - 1 );
        }
        if ( line.isEmpty( ) ) {
            continue;
        }

        auto pieces = splitLine( line );
        debug( "+ Shepherd::handleInput:\n" );
        if ( pieces[0] == "ok" ) {
            debug( "  + ok %s\n", pieces[1].toUtf8( ).data( ) );
        } else if ( pieces[0] == "fail" ) {
            debug( "  + FAIL %s\n", pieces[1].toUtf8( ).data( ) );
        } else if ( pieces[0] == "warning" ) {
            debug( "  + warning from shepherd: %s\n", pieces[1].toUtf8( ).data( ) );
        } else if ( pieces[0] == "warning" ) {
            debug( "  + info from shepherd about '%s': %s\n", pieces[1].toUtf8( ).data( ), pieces[2].toUtf8( ).data( ) );
        } else if ( pieces[0] == "from_printer" ) {
            debug( "<<< '%s'\n", pieces[1].toUtf8( ).data( ) );
            handleFromPrinter( pieces[1] );
        } else if ( pieces[0] == "to_printer" ) {
            debug( ">>> '%s'\n", pieces[1].toUtf8( ).data( ) );
        } else if ( pieces[0] == "printer_online" ) {
            emit printer_online( );
        } else if ( pieces[0] == "printer_offline" ) {
            emit printer_offline( );
        }
    }

    if ( !_buffer.isEmpty( ) ) {
        debug( "  + left over in buffer: '%s'\n", _buffer.toUtf8( ).data( ) );
    }
}

void Shepherd::start( ) {
    if ( _process->state( ) == QProcess::NotRunning ) {
        _process->start( "./stdio-shepherd.py" );
    } else {
        debug( "+ Shepherd::start: already running\n" );
    }
}

void Shepherd::doMove( float arg ) {
    if ( _pendingCommand != PendingCommand::none ) {
        debug( "+ Shepherd::doMove: command already in progress" );
        return;
    }
    _pendingCommand = PendingCommand::move;
    _okCount = 0;
    _process->write( QString( "move %1\n" ).arg( arg ).toUtf8( ) );
}

void Shepherd::doMoveTo( float arg ) {
    if ( _pendingCommand != PendingCommand::none ) {
        debug( "+ Shepherd::doMoveTo: command already in progress" );
        return;
    }
    _pendingCommand = PendingCommand::moveTo;
    _okCount = 0;
    _process->write( QString( "moveTo %1\n" ).arg( arg ).toUtf8( ) );
}

void Shepherd::doHome( ) {
    if ( _pendingCommand != PendingCommand::none ) {
        debug( "+ Shepherd::doHome: command already in progress" );
        return;
    }
    _pendingCommand = PendingCommand::home;
    _okCount = 0;
    _process->write( "home\n" );
}

void Shepherd::doSend( char const* arg ) {
    _process->write( QString( "send \"%1\"\n" ).arg( QString( arg ).replace( "\\", "\\\\" ).replace( "\"", "\\\"" ) ).toUtf8( ) );
}

void Shepherd::doTerminate( ) {
    _process->write( "terminate\n" );
    _process->waitForFinished( );
}
