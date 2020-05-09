#include <magick/api.h>
#include "pch.h"

void _initializeOpenGL( ) {
    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );

    QSurfaceFormat::setDefaultFormat( format );
}

int main( int argc, char* argv[] ) {
    _initializeOpenGL( );
    InitializeMagick(argv[0]);

    return App( argc, argv ).exec( );
}
