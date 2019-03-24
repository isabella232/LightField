#include "pch.h"

#include "app.h"

#include "signalhandler.h"
#include "utils.h"
#include "window.h"

AppSettings g_settings;

namespace {

    QCommandLineParser commandLineParser;

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
            ::fputs( commandLineParser.helpText( ).toUtf8( ).data( ), stderr );
            ::exit( 0 );
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
    commandLineParser.setOptionsAfterPositionalArgumentsMode( QCommandLineParser::ParseAsOptions );
    commandLineParser.setSingleDashWordOptionMode( QCommandLineParser::ParseAsCompactedShortOptions );
    commandLineParser.addOptions( commandLineOptions );
    commandLineParser.process( *this );

    for ( auto i = 0; i < commandLineOptions.count( ); ++i ) {
        if ( commandLineParser.isSet( commandLineOptions[i] ) ) {
            commandLineActions[i]( );
        }
    }
}

void App::_setTheme( ) {
    setStyle( QStyleFactory::create( "Fusion" ) );

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
    g_signalHandler = new SignalHandler;

    QCoreApplication::setOrganizationName( "Volumetric" );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "LightField" );
    QCoreApplication::setApplicationVersion( LIGHTFIELD_VERSION );
    QGuiApplication::setFont( ModifyFont( QGuiApplication::font( ), "Montserrat" ) );

    QProcess::startDetached( SetpowerCommand, { "0" } );

    _parseCommandLine( );
    _debugManager = new DebugManager;
    _setTheme( );

    _window = new Window;
    _window->show( );
}

App::~App( ) {
    delete _window;
    delete g_signalHandler;

    QProcess::startDetached( SetpowerCommand, { "0" } );
}
