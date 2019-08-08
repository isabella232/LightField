#include "pch.h"

#include "lightfieldstyle.h"
#include "signalhandler.h"
#include "version.h"
#include "window.h"

AppSettings g_settings;

namespace {

    QCommandLineParser CommandLineParser;
    QFile              PidFile           { QString { "/run/user/%1/lf.pid" }.arg( getuid( ) ) };
    bool               MoveMainWindow    { };

    QList<QCommandLineOption> CommandLineOptions {
        QCommandLineOption { QStringList { "?", "help"  }, "Displays this help."                                                                    },
        QCommandLineOption { QStringList { "l", "light" }, "Selects the \"light\" theme."                                                           },
        QCommandLineOption {               "s",            "Run at 800×480.",                                                                       },
        QCommandLineOption {               "x",            "Offsets the projected image horizontally.",                              "xOffset", "0" },
        QCommandLineOption {               "y",            "Offsets the projected image vertically.",                                "yOffset", "0" },
#if defined _DEBUG
        QCommandLineOption {               "h",            "Positions main window at (0, 0)."                                                       },
        QCommandLineOption {               "i",            "Sets FramelessWindowHint instead of BypassWindowManagerHint on windows."                },
        QCommandLineOption {               "j",            "Pretend printer preparation is complete."                                               },
        QCommandLineOption {               "k",            "Ignore stdio-shepherd failure reports."                                                 },
        QCommandLineOption {               "m",            "Pretend printer is online."                                                             },
        QCommandLineOption {               "n",            "Ignore USB."                                                                            },
#endif // defined _DEBUG
    };

    QList<std::function<void( )>> CommandLineActions {
        [] ( ) { // -? or --help
            ::fputs( CommandLineParser.helpText( ).toUtf8( ).data( ), stdout );
            ::exit( 0 );
        },
        [] ( ) { // -l or --light
            g_settings.theme = Theme::Light;
        },
        [ ] ( ) { // -s
            MainWindowSize           = SmallMainWindowSize;
            MainButtonSize           = SmallMainButtonSize;
            MaximalRightHandPaneSize = SmallMaximalRightHandPaneSize;
        },
        [] ( ) { // -x
            auto value = CommandLineParser.value( CommandLineOptions[3] );

            bool ok = false;
            auto xOffset = value.toInt( &ok, 10 );
            if ( ok ) {
                g_settings.projectorOffset.setX( xOffset );
            } else {
                ::fprintf( stderr, "Invalid value given for -x parameter.\n" );
                ::exit( 1 );
            }
        },
        [] ( ) { // -y
            auto value = CommandLineParser.value( CommandLineOptions[4] );

            bool ok = false;
            auto yOffset = value.toInt( &ok, 10 );
            if ( ok ) {
                g_settings.projectorOffset.setY( yOffset );
            } else {
                ::fprintf( stderr, "Invalid value given for -y parameter.\n" );
                ::exit( 1 );
            }
        },
#if defined _DEBUG
        [] ( ) { // -h
            MoveMainWindow = true;
        },
        [] ( ) { // -i
            g_settings.frameless = true;
        },
        [] ( ) { // -j
            g_settings.pretendPrinterIsPrepared = true;
        },
        [] ( ) { // -k
            g_settings.ignoreShepherdFailures = true;
        },
        [] ( ) { // -m
            g_settings.pretendPrinterIsOnline = true;
        },
        [] ( ) { // -n
            g_settings.ignoreUsb = true;
        },
#endif // defined _DEBUG
    };

}

void App::_parseCommandLine( ) {
    CommandLineParser.setOptionsAfterPositionalArgumentsMode( QCommandLineParser::ParseAsOptions );
    CommandLineParser.setSingleDashWordOptionMode( QCommandLineParser::ParseAsCompactedShortOptions );
    CommandLineParser.addOptions( CommandLineOptions );
    CommandLineParser.process( *this );

    for ( auto i = 0; i < CommandLineOptions.count( ); ++i ) {
        if ( CommandLineParser.isSet( CommandLineOptions[i] ) ) {
            CommandLineActions[i]( );
        }
    }

    if ( MoveMainWindow ) {
        g_settings.mainWindowPosition.setY( 0 );
        g_settings.projectorWindowPosition.setY( MainWindowSize.height( ) );
    }
}

bool App::_isAlreadyRunning( ) {
    if ( !PidFile.exists( ) ) {
        debug( "+ App:_isAlreadyRunning: pid file doesn't exist\n" );
        errno = 0;
        return false;
    }

    if ( !PidFile.open( QIODevice::ReadOnly | QIODevice::ExistingOnly ) ) {
        error_t err = errno;
        debug( "+ App::_isAlreadyRunning: couldn't open pid file: %s [%d]\n", strerror( err ), err );
        errno = err;
        return true;
    }

    bool ok = false;
    pid_t pid = PidFile.readAll( ).toInt( &ok );
    PidFile.close( );
    if ( !ok ) {
        debug( "+ App::_isAlreadyRunning: couldn't get pid from file\n" );
        errno = EINVAL;
        return true;
    }

    debug( "+ App::_isAlreadyRunning: pid from pid file is %d\n", pid );

    if ( getpid( ) == pid ) {
        debug( "+ App::_isAlreadyRunning: pid file contains our pid, so no other instance is running\n" );
        PidFile.remove( );
        errno = 0;
        return false;
    }

    sigval_t val;
    val.sival_int = getpid( );
    if ( -1 == sigqueue( pid, SIGUSR2, val ) ) {
        error_t err = errno;
        if ( ESRCH == err ) {
            debug( "+ App::_isAlreadyRunning: process from pid file doesn't exist\n" );
            PidFile.remove( );
            errno = 0;
            return false;
        }

        debug( "+ App::_isAlreadyRunning: couldn't send signal to other instance: %s [%d]\n", strerror( err ), err );
        errno = err;
        return true;
    }

    sigset_t signalsToWaitFor;
    sigemptyset( &signalsToWaitFor );
    sigaddset( &signalsToWaitFor, SIGUSR2 );

    siginfo_t info            { };
    timespec  timeoutDuration { 5L, 0L };

    if ( -1 == sigtimedwait( &signalsToWaitFor, &info, &timeoutDuration ) ) {
        error_t err = errno;
        if ( EAGAIN == err ) {
            debug( "+ App::_isAlreadyRunning: timed out waiting for reply from other instance\n" );
        } else {
            debug( "+ App::_isAlreadyRunning: sigtimedwait failed: %s [%d]\n", strerror( err ), err );
        }
        errno = err;
        return true;
    }

    int major = 0;
    int minor = 0;
    int teeny = 0;
    int build = 0;
    DecodeVersionCode( info.si_value.sival_int, major, minor, teeny, build );
    debug( "+ App::_isAlreadyRunning: reply from other instance: pid %d, version %d.%d.%d.%d\n", info.si_pid, major, minor, teeny, build );

    errno = 0;
    return true;
}

void App::_recordProcessId( ) {
    if ( PidFile.open( QIODevice::WriteOnly | QIODevice::NewOnly ) ) {
        debug( "+ App::_recordProcessId: saving our pid %d to file %s\n", getpid( ), PidFile.fileName( ).toUtf8( ).data( ) );
        PidFile.write( QString { "%1" }.arg( getpid( ) ).toUtf8( ) );
        PidFile.close( );
    } else {
        debug( "+ App::_recordProcessId: couldn't create new pid file\n" );
    }
}

void App::_setTheme( ) {
    setStyle( new LightFieldStyle( "Fusion" ) );

    if ( g_settings.theme != Theme::Dark ) {
        return;
    }

    QPalette darkPalette;
    darkPalette.setColor(                     QPalette::WindowText,      Qt::white               );
    darkPalette.setColor(                     QPalette::Button,          QColor(  53,  53,  53 ) );
    darkPalette.setColor(                     QPalette::Dark,            QColor(  35,  35,  35 ) );
    darkPalette.setColor(                     QPalette::Text,            Qt::white               );
    darkPalette.setColor(                     QPalette::BrightText,      Qt::red                 );
    darkPalette.setColor(                     QPalette::ButtonText,      Qt::white               );
    darkPalette.setColor(                     QPalette::Base,            QColor(  42,  42,  42 ) );
    darkPalette.setColor(                     QPalette::Window,          QColor(  53,  53,  53 ) );
    darkPalette.setColor(                     QPalette::Shadow,          QColor(  20,  20,  20 ) );
    darkPalette.setColor(                     QPalette::Highlight,       QColor(  42, 130, 218 ) );
    darkPalette.setColor(                     QPalette::HighlightedText, Qt::white               );
    darkPalette.setColor(                     QPalette::Link,            Qt::white               );
    darkPalette.setColor(                     QPalette::AlternateBase,   QColor(  66,  66,  66 ) );
    darkPalette.setColor(                     QPalette::ToolTipBase,     Qt::white               );
    darkPalette.setColor(                     QPalette::ToolTipText,     Qt::white               );

    darkPalette.setColor( QPalette::Disabled, QPalette::WindowText,      QColor( 127, 127, 127 ) );
    darkPalette.setColor( QPalette::Disabled, QPalette::Text,            QColor( 127, 127, 127 ) );
    darkPalette.setColor( QPalette::Disabled, QPalette::ButtonText,      QColor( 127, 127, 127 ) );
    darkPalette.setColor( QPalette::Disabled, QPalette::Highlight,       QColor(  80,  80,  80 ) );
    darkPalette.setColor( QPalette::Disabled, QPalette::HighlightedText, QColor( 127, 127, 127 ) );

    setPalette( darkPalette );
}

App::App( int& argc, char* argv[] ): QApplication( argc, argv ) {
    _debugManager = new DebugManager;

    QCoreApplication::setOrganizationName( "Volumetric, Inc." );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "LightField" );
    QCoreApplication::setApplicationVersion( LIGHTFIELD_VERSION_STRING );
    QGuiApplication::setFont( ModifyFont( ModifyFont( QGuiApplication::font( ), "Montserrat" ), NormalFontSize ) );

    _parseCommandLine( );

    if ( !_isAlreadyRunning( ) ) {
        _recordProcessId( );
    } else {
        debug( "+ App::`ctor: there %s an instance of LightField already running. this instance is terminating.\n", ( 0 == errno ) ? "is" : "may be" );
        ::exit( 1 );
    }

    QProcess::startDetached( SetProjectorPowerCommand, { "0" } );

    _setTheme( );

    _window = new Window;
    _window->show( );
}

App::~App( ) {
    QProcess::startDetached( SetProjectorPowerCommand, { "0" } );

    _window->deleteLater( );
    _window = nullptr;

    delete _debugManager;
    _debugManager = nullptr;
}
