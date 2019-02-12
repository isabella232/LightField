#include "pch.h"

#include "app.h"

#include "window.h"
#include "signalhandler.h"

App::App( int& argc, char *argv[] ):
    QApplication( argc, argv )
{
    QCoreApplication::setOrganizationName( "Volumetric" );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "fstl" );

    AppSettings settings;
    bool fullScreen = true;
    bool debuggingPosition = false;
    for ( int n = 1; n < argc; ++n ) {
        if ( 0 == strcmp( argv[n], "-f" ) ) {
            settings.fullScreen = false;
        } else if ( 0 == strcmp( argv[n], "-g" ) ) {
            settings.debuggingPosition = true;
        } else if ( 0 == strcmp( argv[n], "--dark" ) ) {
            settings.theme = Theme::Dark;
        } else if ( 0 == strcmp( argv[n], "--light" ) ) {
            settings.theme = Theme::Light;
        } else {
            fprintf( stderr, "ignoring unrecognized parameter '%s'\n", argv[n] );
        }
    }

    qDebug( ).setVerbosity( 7 );
    g_signalHandler = new SignalHandler;

    window = new Window( settings );
    window->show( );
}

App::~App( ) {
    delete window;
    delete g_signalHandler;
}
