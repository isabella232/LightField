#include <QtDebug>

#include "app.h"

#include "window.h"
#include "signalhandler.h"

App::App( int& argc, char *argv[] ):
    QApplication( argc, argv )
{
    QCoreApplication::setOrganizationName( "Volumetric" );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "fstl" );

    bool fullScreen = true;
    bool debuggingPosition = false;
    for ( int n = 1; n < argc; ++n ) {
        if ( 0 == strcmp( argv[n], "-f" ) ) {
            fullScreen = false;
        } else if ( 0 == strcmp( argv[n], "-g" ) ) {
            debuggingPosition = true;
        } else {
            fprintf( stderr, "ignoring unrecognized parameter '%s'\n", argv[n] );
        }
    }

    qDebug( ).setVerbosity( 7 );
    g_signalHandler = new SignalHandler;

    window = new Window( fullScreen, debuggingPosition, nullptr );
    window->load_stl( ":gl/BoundingBox.stl" );
    window->show( );
}

App::~App( ) {
    delete window;
    delete g_signalHandler;
}
