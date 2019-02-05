#include "shepherd.h"
#include "window.h"
#include "strings.h"

namespace {

    char const* ShepherdBaseDirectory = "/home/lumen/Volumetric/fstl/stdio-shepherd";

}

Shepherd::Shepherd( QObject* parent ): QObject( parent ) {
    fprintf( stderr, "+ Shepherd::`ctor: Shepherd base directory: '%s'\n", ShepherdBaseDirectory );

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
    QObject::connect( _process, &QProcess::stateChanged,            this, &Shepherd::processStateChanged            );
    QObject::connect( _process, &QProcess::readyReadStandardError,  this, &Shepherd::processReadyReadStandardError  );
    QObject::connect( _process, &QProcess::readyReadStandardOutput, this, &Shepherd::processReadyReadStandardOutput );
    QObject::connect( _process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Shepherd::processFinished );
}

Shepherd::~Shepherd( ) {
    /*empty*/
}

void Shepherd::processErrorOccurred( QProcess::ProcessError error ) {
    fprintf( stderr, "+ Shepherd::processErrorOccurred: error %s [%d]\n", ToString( error ), error );
    emit shepherd_ProcessError( error );
}

void Shepherd::processStarted( ) {
    fprintf( stderr, "+ Shepherd::processStarted\n" );
    emit shepherd_Started( );
}

void Shepherd::processStateChanged( QProcess::ProcessState newState ) {
    fprintf( stderr, "+ Shepherd::processStateChanged: new state %s [%d]\n", ToString( newState ), newState );
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
    fprintf( stderr, "+ Shepherd::processFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );
    emit shepherd_Finished( exitCode, exitStatus );
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

void Shepherd::handlePrinterOutput( QString const& input ) {
    fprintf( stderr, "+ Shepherd::handlePrinterOutput: input='%s' pendingCommand=%s [%d]\n", input.toUtf8( ).data( ), ToString( _pendingCommand ), static_cast<int>( _pendingCommand ) );
    if ( input == "ok" ) {
        switch ( _pendingCommand ) {
            case PendingCommand::move:
                _pendingCommand = PendingCommand::none;
                emit action_moveComplete( true );
                break;

            case PendingCommand::moveTo:
                _pendingCommand = PendingCommand::none;
                emit action_moveToComplete( true );
                break;

            case PendingCommand::home:
                _pendingCommand = PendingCommand::none;
                emit action_homeComplete( true );
                break;

            case PendingCommand::lift:
                _pendingCommand = PendingCommand::none;
                emit action_liftComplete( true );
                break;

            default:
                fprintf( stderr, "+ Shepherd::handlePrinterOutput: unknown pending command\n" );
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

        fprintf( stderr, "+ Shepherd::handleInput: line '%s'\n", line.toUtf8( ).data( ) );
        auto pieces = splitLine( line );
        fprintf( stderr, "  + first piece: '%s'\n", pieces[0].toUtf8( ).data( ) );
        if ( pieces[0] == "printer_output" ) {
            fprintf( stderr, "  + got printer_output\n" );
            handlePrinterOutput( pieces[1] );
        } else if ( pieces[0] == "printer_online" ) {
            emit printer_Online( );
        } else if ( pieces[0] == "printer_offline" ) {
            emit printer_Offline( );
        } else if ( pieces[0] == "printer_position" ) {
            emit printer_Position( QLocale( ).toDouble( pieces[1] ) );
        } else if ( pieces[0] == "printer_temperature" ) {
            emit printer_Temperature( pieces[1] );
        } else if ( pieces[0] == "printProcess_showImage" ) {
            emit printProcess_ShowImage( pieces[1], pieces[2], pieces[3], pieces[4] );
        } else if ( pieces[0] == "printProcess_hideImage" ) {
            emit printProcess_HideImage( );
        } else if ( pieces[0] == "printProcess_startedPrinting" ) {
            emit printProcess_StartedPrinting( );
        } else if ( pieces[0] == "printProcess_finishedPrinting" ) {
            emit printProcess_FinishedPrinting( );
        } else if ( pieces[0] == "ok" ) {
            fprintf( stderr, "  + got ok for %s\n", pieces[1].toUtf8( ).data( ) );
        } else if ( pieces[0] == "fail" ) {
            fprintf( stderr, "  + got fail for %s\n", pieces[1].toUtf8( ).data( ) );
        }
    }

    fprintf( stderr, "  + left over in buffer: '%s'\n", _buffer.toUtf8( ).data( ) );
}

void Shepherd::start( ) {
    if ( _process->state( ) == QProcess::NotRunning ) {
        _process->start( "./stdio-shepherd.py" );
    }
}

void Shepherd::doMove( float arg ) {
    if ( _pendingCommand != PendingCommand::none ) {
        fprintf( stderr, "Shepherd::doMove: command already in progress" );
        return;
    }
    _pendingCommand = PendingCommand::move;
    _process->write( QString( "move %1\n" ).arg( arg ).toUtf8( ) );
}

void Shepherd::doMoveTo( float arg ) {
    if ( _pendingCommand != PendingCommand::none ) {
        fprintf( stderr, "Shepherd::doMoveTo: command already in progress" );
        return;
    }
    _pendingCommand = PendingCommand::moveTo;
    _process->write( QString( "moveTo %1\n" ).arg( arg ).toUtf8( ) );
}

void Shepherd::doHome( ) {
    if ( _pendingCommand != PendingCommand::none ) {
        fprintf( stderr, "Shepherd::doHome: command already in progress" );
        return;
    }
    _pendingCommand = PendingCommand::home;
    _process->write( "home\n" );
}

void Shepherd::doLift( float arg1, float arg2 ) {
    if ( _pendingCommand != PendingCommand::none ) {
        fprintf( stderr, "Shepherd::doLift: command already in progress" );
        return;
    }
    _pendingCommand = PendingCommand::lift;
    _process->write( QString( "lift %1 %2\n" ).arg( arg1 ).arg( arg2 ).toUtf8( ) );
}

void Shepherd::doAskTemp( ) {
    _process->write( "askTemp\n" );
}

void Shepherd::doSend( char const* arg ) {
    _process->write( QString( "send \"%1\"\n" ).arg( QString( arg ).replace( "\\", "\\\\" ).replace( "\"", "\\\"" ) ).toUtf8( ) );
}

void Shepherd::doTerminate( ) {
    _process->write( "terminate\n" );
    _process->waitForFinished( );
}
