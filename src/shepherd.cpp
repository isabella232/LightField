#include "pch.h"

#include "shepherd.h"

#include "app.h"
#include "processrunner.h"
#include "strings.h"
#include "window.h"

namespace {

    char const* ShepherdBaseDirectory = "/usr/share/lightfield/libexec/stdio-shepherd";

    QRegularExpression PositionReportMatcher     { "^X:(-?\\d+\\.\\d\\d) Y:(-?\\d+\\.\\d\\d) Z:(-?\\d+\\.\\d\\d) E:(-?\\d+\\.\\d\\d) Count X:(-?\\d+) Y:(-?\\d+) Z:(-?\\d+)", QRegularExpression::CaseInsensitiveOption };
    QRegularExpression TemperatureReportMatcher1 { "^T:(-?\\d+\\.\\d\\d)\\s*/(-?\\d+\\.\\d\\d) B:(-?\\d+\\.\\d\\d)\\s*/(-?\\d+\\.\\d\\d) @:(-?\\d+) B@:(-?\\d+)",             QRegularExpression::CaseInsensitiveOption };
    QRegularExpression TemperatureReportMatcher2 { "^T:(-?\\d+\\.\\d\\d)\\s*/(-?\\d+\\.\\d\\d) @:(-?\\d+)",                                                                   QRegularExpression::CaseInsensitiveOption };

}

Shepherd::Shepherd( QObject* parent ): QObject( parent ) {
    debug( "+ Shepherd::`ctor: Shepherd base directory: '%s'\n", ShepherdBaseDirectory );
}

Shepherd::~Shepherd( ) {
    /*empty*/
}

void Shepherd::process_errorOccurred( QProcess::ProcessError error ) {
    debug( "+ Shepherd::process_errorOccurred: error %s [%d]\n", ToString( error ), error );
    if ( error == QProcess::FailedToStart ) {
        debug( "+ Shepherd::process_errorOccurred: process failed to start\n" );
        emit shepherd_startFailed( );
    }
}

void Shepherd::process_started( ) {
    debug( "+ Shepherd::process_started\n" );
    emit shepherd_started( );

#if defined _DEBUG
    if ( g_settings.pretendPrinterIsOnline ) {
        emit printer_online( );
    }
#endif // defined _DEBUG
}

void Shepherd::process_readyReadStandardError( ) {
    _process->setReadChannel( QProcess::StandardError );

    QString input = _process->readAllStandardError( );
    if ( input.length( ) ) {
        debug(
            "+ Shepherd::process_readyReadStandardError: from stderr:\n"
            "%s\n",
            input.toUtf8( ).data( )
        );
    }
}

void Shepherd::process_readyReadStandardOutput( ) {
    _process->setReadChannel( QProcess::StandardOutput );

    QString input = _process->readAllStandardOutput( );
    if ( input.length( ) ) {
        handleInput( input );
    }
}

void Shepherd::process_finished( int exitCode, QProcess::ExitStatus exitStatus ) {
    debug( "+ Shepherd::process_finished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ToString( exitStatus ), exitStatus );
    if ( ( exitStatus == QProcess::CrashExit ) || ( exitCode != 0 ) ) {
        debug( "+ Shepherd::process_finished: process failed: exit status: %s [%d]; exit code: %d\n", ToString( exitStatus ), exitStatus, exitCode );
        emit shepherd_terminated( _isTerminationExpected, false );
    } else {
        emit shepherd_terminated( _isTerminationExpected, true );
    }
}

void Shepherd::processRunner_succeeded( ) {
    debug( "+ Shepherd::processRunner_succeeded\n" );

    if ( _processRunner ) {
        _processRunner->deleteLater( );
        _processRunner = nullptr;
    }

    if ( _process ) {
        _process->kill( );
        _process->deleteLater( );
        _process = nullptr;
    }

    _process = new QProcess( parent( ) );
    QObject::connect( _process, &QProcess::errorOccurred,                                        this, &Shepherd::process_errorOccurred           );
    QObject::connect( _process, &QProcess::started,                                              this, &Shepherd::process_started                 );
    QObject::connect( _process, &QProcess::readyReadStandardError,                               this, &Shepherd::process_readyReadStandardError  );
    QObject::connect( _process, &QProcess::readyReadStandardOutput,                              this, &Shepherd::process_readyReadStandardOutput );
    QObject::connect( _process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Shepherd::process_finished                );
    _process->setWorkingDirectory( ShepherdBaseDirectory );
    _process->start( "./stdio-shepherd.py" );
}

void Shepherd::processRunner_failed( QProcess::ProcessError const ) {
    processRunner_succeeded( );
}

bool Shepherd::getReady( char const* functionName, PendingCommand const pendingCommand, int const expectedOkCount ) {
    if ( _pendingCommand != PendingCommand::none ) {
        debug( "+ Shepherd::%s: command %s already in progress\n", functionName, ToString( pendingCommand ) );
        return false;
    }

    _pendingCommand  = pendingCommand;
    _expectedOkCount = expectedOkCount;
    _okCount         = 0;
    return true;
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
    debug(
        "+ Shepherd::handleFromPrinter: input: '%s'; pendingCommand: %s [%d]; expectedOkCount: %d; okCount: %d\n",
        input.toUtf8( ).data( ),
        ToString( _pendingCommand ), static_cast<int>( _pendingCommand ),
        _expectedOkCount,
        _okCount
    );
    if ( input == "ok" ) {
        if ( ++_okCount == _expectedOkCount ) {
            debug( "+ Shepherd::handleFromPrinter: got final expected ok, dispatching completion notification\n" );
            auto pending = _pendingCommand;
            _pendingCommand = PendingCommand::none;
            switch ( pending ) {
                case PendingCommand::moveRelative:
                    emit action_moveRelativeComplete( true );
                    break;

                case PendingCommand::moveAbsolute:
                    emit action_moveAbsoluteComplete( true );
                    break;

                case PendingCommand::home:
                    emit action_homeComplete( true );
                    break;

                case PendingCommand::send:
                    emit action_sendComplete( true );
                    break;

                case PendingCommand::none:
                    debug( "+ Shepherd::handleFromPrinter: no pending command\n" );
                    break;

                default:
                    debug( "+ Shepherd::handleFromPrinter: unknown pending command\n" );
                    break;
            }
        }
    } else if ( auto match = PositionReportMatcher.match( input ); match.hasMatch( ) ) {
        auto px = match.captured( 1 ).toDouble( );
        auto py = match.captured( 2 ).toDouble( );
        auto pz = match.captured( 3 ).toDouble( );
        auto pe = match.captured( 4 ).toDouble( );
        auto cx = match.captured( 5 ).toDouble( );
        auto cy = match.captured( 6 ).toDouble( );
        auto cz = match.captured( 7 ).toDouble( );
        debug( "+ Shepherd::handleFromPrinter: position report: XYZ (%.2f,%.2f,%.2f) E %.2f; counts: XYZ (%.0f,%.0f,%.0f)\n", px, py, pz, pe, cx, cy, cz );
        emit printer_positionReport( px, py, pz, pe, cx, cy, cz );
    } else if ( auto match = TemperatureReportMatcher1.match( input ); match.hasMatch( ) ) {
        auto bedCurrentTemperature = match.captured( 3 ).toDouble( );
        auto bedTargetTemperature  = match.captured( 4 ).toDouble( );
        auto bedPwm                = match.captured( 6 ).toInt( );
        debug( "+ Shepherd::handleFromPrinter: temperature report (type 1): current %.2f °C, target %.2f °C, PWM %d\n", bedCurrentTemperature, bedTargetTemperature, bedPwm );
        emit printer_temperatureReport( bedCurrentTemperature, bedTargetTemperature, bedPwm );
    } else if ( auto match = TemperatureReportMatcher2.match( input ); match.hasMatch( ) ) {
        auto bedCurrentTemperature = match.captured( 1 ).toDouble( );
        auto bedTargetTemperature  = match.captured( 2 ).toDouble( );
        auto bedPwm                = match.captured( 3 ).toInt( );
        debug( "+ Shepherd::handleFromPrinter: temperature report (type 2): current %.2f °C, target %.2f °C, PWM %d\n", bedCurrentTemperature, bedTargetTemperature, bedPwm );
        emit printer_temperatureReport( bedCurrentTemperature, bedTargetTemperature, bedPwm );
    }
}

void Shepherd::handleCommandFail( QStringList const& input ) {
    debug( "+ Shepherd::handleCommandFail: input='%s' pendingCommand=%s [%d]\n", input.join( Space ).toUtf8( ).data( ), ToString( _pendingCommand ), _pendingCommand );

    auto pending = _pendingCommand;
    _pendingCommand = PendingCommand::none;
    switch ( pending ) {
        case PendingCommand::moveRelative:
            emit action_moveRelativeComplete( false );
            break;

        case PendingCommand::moveAbsolute:
            emit action_moveAbsoluteComplete( false );
            break;

        case PendingCommand::home:
            emit action_homeComplete( false );
            break;

        case PendingCommand::send:
            emit action_sendComplete( false );
            break;

        case PendingCommand::none:
            debug( "+ Shepherd::handleCommandFail: no pending command\n" );
            break;

        default:
            debug( "+ Shepherd::handleCommandFail: unknown pending command\n" );
            break;
    }
}

#if defined _DEBUG
void Shepherd::handleCommandFailAlternate( QStringList const& input ) {
    debug( "+ Shepherd::handleCommandFailAlternate: input='%s' pendingCommand=%s [%d]\n", input.join( Space ).toUtf8( ).data( ), ToString( _pendingCommand ), _pendingCommand );

    auto pending = _pendingCommand;
    _pendingCommand = PendingCommand::none;
    switch ( pending ) {
        case PendingCommand::moveRelative:
            emit action_moveRelativeComplete( true );
            break;

        case PendingCommand::moveAbsolute:
            emit action_moveAbsoluteComplete( true );
            break;

        case PendingCommand::home:
            emit action_homeComplete( true );
            break;

        case PendingCommand::send:
            emit action_sendComplete( true );
            break;

        case PendingCommand::none:
            debug( "+ Shepherd::handleCommandFailAlternate: no pending command\n" );
            break;

        default:
            debug( "+ Shepherd::handleCommandFailAlternate: unknown pending command\n" );
            break;
    }
}
#endif // defined _DEBUG

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
        debug( "+ Shepherd::handleInput: '%s' [%d]\n", pieces[0].toUtf8( ).data( ), pieces.count( ) );
        if ( pieces[0] == "ok" ) {
            if ( pieces.count( ) > 1 ) {
                debug( "  + ok %s\n", pieces[1].toUtf8( ).data( ) );
            } else {
                debug( "  + ok\n" );
            }
        } else if ( pieces[0] == "fail" ) {
            debug( "  + FAIL %s\n", pieces[1].toUtf8( ).data( ) );
#if defined _DEBUG
            if ( g_settings.ignoreShepherdFailures ) {
                handleCommandFailAlternate( pieces );
            } else {
                handleCommandFail( pieces );
            }
#else
            handleCommandFail( pieces );
#endif // defined _DEBUG
        } else if ( pieces[0] == "warning" ) {
            debug( "  + warning from shepherd: %s\n", pieces[1].toUtf8( ).data( ) );
        } else if ( pieces[0] == "info" ) {
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
    debug( "+ Shepherd::start\n" );

    _processRunner = new ProcessRunner( this );
    QObject::connect( _processRunner, &ProcessRunner::succeeded, this, &Shepherd::processRunner_succeeded );
    QObject::connect( _processRunner, &ProcessRunner::failed,    this, &Shepherd::processRunner_failed    );
    _processRunner->start( "/usr/share/lightfield/lightfield/reset-lumen-arduino-port", { } );
}

void Shepherd::doMoveRelative( float const relativeDistance ) {
    if ( getReady( "doMoveRelative", PendingCommand::moveRelative, 4 ) ) {
        _process->write( QString( "move %1\n" ).arg( relativeDistance, 0, 'f', 2 ).toUtf8( ) );
    }
}

void Shepherd::doMoveAbsolute( float const absolutePosition ) {
    if ( getReady( "doMoveAbsolute", PendingCommand::moveAbsolute, 4 ) ) {
        _process->write( QString( "moveTo %1\n" ).arg( absolutePosition, 0, 'f', 2 ).toUtf8( ) );
    }
}

void Shepherd::doHome( ) {
    if ( getReady( "doHome", PendingCommand::home, 3 ) ) {
        _process->write( "home\n" );
    }
}

void Shepherd::doSend( QString cmd ) {
    if ( getReady( "doSend", PendingCommand::send, 1 ) ) {
        doSendOne( cmd );
    }
}

void Shepherd::doSend( QStringList cmds ) {
    if ( getReady( "doSend", PendingCommand::send, cmds.count( ) ) ) {
        for ( auto& cmd : cmds ) {
            doSendOne( cmd );
        }
    }
}

void Shepherd::doSendOne( QString& cmd ) {
    _process->write( QString( "send \"%1\"\n" ).arg( cmd.replace( "\\", "\\\\" ).replace( "\"", "\\\"" ) ).toUtf8( ) );
}

void Shepherd::doTerminate( ) {
    _isTerminationExpected = true;
    _process->write( "terminate\n" );
    _process->waitForFinished( );
}
