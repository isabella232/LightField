#include "pch.h"

void _initializeOpenGL( ) {
    QSurfaceFormat format;

    format.setRenderableType( QSurfaceFormat::OpenGL/*ES*/ );
    format.setProfile( QSurfaceFormat::CompatibilityProfile/*CoreProfile*/ );
    format.setVersion( 2, 1 );
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );
#if defined _DEBUG
    format.setOption( QSurfaceFormat::DebugContext, true );
#endif // defined _DEBUG

    QSurfaceFormat::setDefaultFormat( format );
}

int main( int argc, char* argv[] ) {
    _initializeOpenGL( );
    return App( argc, argv ).exec( );
}
