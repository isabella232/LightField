#include "pch.h"

#include "app.h"

#include "signalhandler.h"
#include "utils.h"
#include "window.h"

AppSettings g_settings;

namespace {

    QList<QCommandLineOption> commandLineOptions {
        QCommandLineOption { QStringList { "?", "help"  }, "Displays this help."                                                     },
#if defined _DEBUG
        QCommandLineOption {               "h",            "Positions main window at (0, 0)."                                        },
        QCommandLineOption {               "i",            "Sets FramelessWindowHint instead of BypassWindowManagerHint on windows." },
        QCommandLineOption {               "j",            "Pretend printer preparation is complete."                                },
        QCommandLineOption {               "k",            "Ignore stdio-shepherd failure reports."                                  },
#endif // defined _DEBUG
        QCommandLineOption { QStringList { "l", "light" }, "Selects the \"light\" theme."                                            },
    };

    QList<std::function<void( )>> commandLineActions {
        [] ( ) { // -? or --help
            /* empty - handled elsewhere */
        },
#if defined _DEBUG
        [] ( ) { // -h
            g_settings.mainWindowPosition.setY( 0 );
            g_settings.pngDisplayWindowPosition.setY( 480 );
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
#endif // defined _DEBUG
        [] ( ) { // -l or --light
            g_settings.theme = Theme::Light;
        },
    };

}

void App::_parseCommandLine( ) {
    QCommandLineParser parser;

    parser.setOptionsAfterPositionalArgumentsMode( QCommandLineParser::ParseAsOptions );
    parser.setSingleDashWordOptionMode( QCommandLineParser::ParseAsCompactedShortOptions );
    parser.addOptions( commandLineOptions );
    parser.process( *this );

    if ( parser.isSet( commandLineOptions[0] ) ) { // -? or --help
        ::fputs( parser.helpText( ).toUtf8( ).data( ), stderr );
        ::exit( 0 );
    }

    for ( auto i = 1; i < commandLineOptions.count( ); ++i ) {
        if ( parser.isSet( commandLineOptions[i] ) ) {
            commandLineActions[i]( );
        }
    }
}

void App::_setTheme( ) {
    if ( g_settings.theme == Theme::None ) {
        return;
    }

    QFile file;
    if ( g_settings.theme == Theme::Dark ) {
        file.setFileName( ":/dark.qss" );
    } else if ( g_settings.theme == Theme::Light ) {
        file.setFileName( ":/light.qss" );
    }
    file.open( QFile::ReadOnly | QFile::Text );
    setStyleSheet( QTextStream { &file }.readAll( ) );
}

App::App( int& argc, char* argv[] ): QApplication( argc, argv ) {
    g_signalHandler = new SignalHandler;

    QCoreApplication::setOrganizationName( "Volumetric" );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "LightField" );
    QCoreApplication::setApplicationVersion( LIGHTFIELD_VERSION );
    QGuiApplication::setFont( ModifyFont( QGuiApplication::font( ), "Montserrat" ) );

    QProcess::startDetached( SetpowerCommand, { "0" } );

    _parseCommandLine( );
    _setTheme( );

    _debugManager = new DebugManager;
    _window = new Window;
    _window->show( );
}

App::~App( ) {
    delete _window;
    delete g_signalHandler;

    QProcess::startDetached( SetpowerCommand, { "0" } );
}
