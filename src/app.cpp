#include "pch.h"

#include "app.h"

#include "signalhandler.h"
#include "strings.h"
#include "window.h"

AppSettings g_settings;

namespace {

    QList<QCommandLineOption> commandLineOptions {
        QCommandLineOption { QStringList { "?", "help"  }, "Displays this help."                                                     },
        QCommandLineOption {               "f",            "Ignored for backwards compatibility."                                    },
        QCommandLineOption {               "g",            "Positions main window at southwest corner of a 1080p display, (0, 560)." },
        QCommandLineOption {               "h",            "Positions main window at (0, 0)."                                        },
        QCommandLineOption {               "i",            "Sets FramelessWindowHint instead of BypassWindowManagerHint on windows." },
        QCommandLineOption { QStringList { "d", "dark"  }, "Selects the \"dark\" theme."                                             },
        QCommandLineOption { QStringList { "l", "light" }, "Selects the \"light\" theme."                                            },
    };

}

void App::parseCommandLine( ) {
    QCommandLineParser parser;

    for ( auto i : { 1, 2, 3, 4 } ) {
        commandLineOptions[i].setFlags( QCommandLineOption::HiddenFromHelp );
    }

    parser.setOptionsAfterPositionalArgumentsMode( QCommandLineParser::ParseAsOptions );
    parser.setSingleDashWordOptionMode( QCommandLineParser::ParseAsCompactedShortOptions );
    parser.addOptions( commandLineOptions );
    parser.process( *this );

    if ( parser.isSet( commandLineOptions[0] ) ) {
        fputs( parser.helpText( ).toUtf8( ).data( ), stderr );
        exit( 0 );
    }
    if ( parser.isSet( commandLineOptions[2] ) ) {
        g_settings.mainWindowPosition.setY( 560 );
    }
    if ( parser.isSet( commandLineOptions[3] ) ) {
        g_settings.mainWindowPosition.setY( 0 );
        g_settings.pngDisplayWindowPosition.setY( 480 );
    }
    if ( parser.isSet( commandLineOptions[4] ) ) {
        g_settings.frameless = true;
    }
    if ( parser.isSet( commandLineOptions[5] ) ) {
        g_settings.theme = Theme::Dark;
    }
    if ( parser.isSet( commandLineOptions[6] ) ) {
        g_settings.theme = Theme::Light;
    }

    debug(
        "+ App::parseCommandLine:\n"
        "  + Main window position:        %s\n"
        "  + PNG display window position: %s\n"
        "",
        ToString( g_settings.mainWindowPosition       ).toUtf8( ).data( ),
        ToString( g_settings.pngDisplayWindowPosition ).toUtf8( ).data( )
    );
}

App::App( int& argc, char *argv[] ):
    QApplication( argc, argv )
{
    QCoreApplication::setOrganizationName( "Volumetric" );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "LightField" );

    QFile file { ":/dark.qss" };
    file.open( QFile::ReadOnly | QFile::Text );
    setStyleSheet( QTextStream { &file }.readAll( ) );

    auto font = QGuiApplication::font( );
    font.setFamily( QString( "Montserrat" ) );
    QGuiApplication::setFont( font );

    parseCommandLine( );

    g_signalHandler = new SignalHandler;

    window = new Window( );
    window->show( );
}

App::~App( ) {
    delete window;
    delete g_signalHandler;
}
