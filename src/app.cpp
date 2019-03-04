#include "pch.h"

#include "app.h"

#include "signalhandler.h"
#include "strings.h"
#include "window.h"

AppSettings g_settings;

namespace {

    QList<QCommandLineOption> commandLineOptions {
        QCommandLineOption { QStringList { "?", "help"  }, "Displays this help."                                                     },
        QCommandLineOption {               "g",            "Positions main window at southwest corner of a 1080p display, (0, 560)." },
        QCommandLineOption {               "h",            "Positions main window at (0, 0)."                                        },
        QCommandLineOption {               "i",            "Sets FramelessWindowHint instead of BypassWindowManagerHint on windows." },
        QCommandLineOption {               "j",            "Pretend printer preparation is complete."                                },
        QCommandLineOption { QStringList { "l", "light" }, "Selects the \"light\" theme."                                            },
    };

    QList<std::function<void( )>> commandLineActions {
        [] ( ) {
            /*empty*/
        },
        [] ( ) {
            g_settings.mainWindowPosition.setY( 560 );
        },
        [] ( ) {
            g_settings.mainWindowPosition.setY( 0 );
            g_settings.pngDisplayWindowPosition.setY( 480 );
        },
        [] ( ) {
            g_settings.frameless = true;
        },
        [] ( ) {
            g_settings.pretendPrinterIsPrepared = true;
        },
        [] ( ) {
            g_settings.theme = Theme::Light;
        },
    };

}

void App::parseCommandLine( ) {
    QCommandLineParser parser;

    for ( auto i = 1; i <= 4; ++i ) {
        commandLineOptions[i].setFlags( QCommandLineOption::HiddenFromHelp );
    }

    parser.setOptionsAfterPositionalArgumentsMode( QCommandLineParser::ParseAsOptions );
    parser.setSingleDashWordOptionMode( QCommandLineParser::ParseAsCompactedShortOptions );
    parser.addOptions( commandLineOptions );
    parser.process( *this );

    if ( parser.isSet( commandLineOptions[0] ) ) {
        // -? or --help
        fputs( parser.helpText( ).toUtf8( ).data( ), stderr );
        exit( 0 );
    }

    for ( auto i = 1; i < commandLineOptions.count( ); ++i ) {
        if ( parser.isSet( commandLineOptions[i] ) ) {
            commandLineActions[i]( );
        }
    }
}

App::App( int& argc, char *argv[] ):
    QApplication( argc, argv )
{
    QCoreApplication::setOrganizationName( "Volumetric" );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "LightField" );

    g_signalHandler = new SignalHandler;

    auto font = QGuiApplication::font( );
    font.setFamily( QString( "Montserrat" ) );
    QGuiApplication::setFont( font );

    parseCommandLine( );

    if ( g_settings.theme == Theme::Dark ) {
        QFile file { ":/dark.qss" };
        file.open( QFile::ReadOnly | QFile::Text );
        setStyleSheet( QTextStream { &file }.readAll( ) );
    }

    if ( 0 != ::mkdir( JobWorkingDirectoryPath.toUtf8( ).data( ), 0700 ) ) {
        error_t err = errno;
        debug( "App::`ctor: unable to create job working directory root: %s [%d]\n", strerror( err ), err );
        // TODO now what?
    }

    window = new Window( );
    window->show( );
}

App::~App( ) {
    delete window;
    delete g_signalHandler;
}
