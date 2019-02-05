#include "pngdisplayer.h"

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    _label = new QLabel( );
    setCentralWidget( _label );
}

PngDisplayer::~PngDisplayer( ) {

}

bool PngDisplayer::load( QString const& fileName ) {
    QPixmap pixmap;
    if ( !pixmap.load( fileName ) ) {
        return false;
    }

    _label->setPixmap( pixmap );
    return true;
}

void PngDisplayer::setFullScreen( bool const fullScreen ) {
    if ( fullScreen ) {
        setWindowState( windowState( ) | Qt::WindowFullScreen );
    } else {
        setWindowState( windowState( ) & ~Qt::WindowFullScreen );
    }
}
