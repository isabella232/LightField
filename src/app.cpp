#include "pch.h"

#include "app.h"

#include "window.h"
#include "signalhandler.h"

AppSettings g_settings;

std::map<std::string, std::function<void( )>> commandLineArgHandlers {
    { std::string( "-f"      ), [] ( ) { g_settings.fullScreen        = false;        } },
    { std::string( "-g"      ), [] ( ) { g_settings.debuggingPosition = true;         } },
    { std::string( "--dark"  ), [] ( ) { g_settings.theme             = Theme::Dark;  } },
    { std::string( "--light" ), [] ( ) { g_settings.theme             = Theme::Light; } },
};

App::App( int& argc, char *argv[] ):
    QApplication( argc, argv )
{
    QCoreApplication::setOrganizationName( "Volumetric" );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "fstl" );

    for ( int n = 1; n < argc; ++n ) {
        try {
            commandLineArgHandlers.at( argv[n] )( );
        }
        catch ( std::out_of_range const& ) {
            fprintf( stderr, "ignoring unrecognized parameter '%s'\n", argv[n] );
        }
    }

    g_signalHandler = new SignalHandler;

    window = new Window( );
    window->show( );
}

App::~App( ) {
    delete window;
    delete g_signalHandler;
}
