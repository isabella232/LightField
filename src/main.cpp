#include "pch.h"

void _initializeOpenGL( ) {
    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );

    QSurfaceFormat::setDefaultFormat( format );
}

int main( int argc, char* argv[] ) {
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    qputenv("QT_VIRTUALKEYBOARD_STYLE_PATH","./styles");
    _initializeOpenGL( );
    return App( argc, argv ).exec( );
}
