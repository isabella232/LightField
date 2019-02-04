#include <QDebug>

#include "app.h"
#include "window.h"

App::App( int& argc, char *argv[] ):
    QApplication( argc, argv )
{
    QCoreApplication::setOrganizationName( "Volumetric" );
    QCoreApplication::setOrganizationDomain( "https://www.volumetricbio.com/" );
    QCoreApplication::setApplicationName( "fstl" );

    bool fullScreen = true;
    if ( argc > 1 ) {
        if ( 0 == strcmp( argv[1], "-f" ) ) {
            fullScreen = false;
        }
    }

    qDebug( ).setVerbosity( 7 );
    window = new Window( fullScreen );
    window->load_stl( ":gl/BoundingBox.stl" );
    window->show( );
}

App::~App( ) {
    delete window;
}
