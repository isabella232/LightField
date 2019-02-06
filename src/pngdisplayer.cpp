#include "pngdisplayer.h"

#include <QPalette>

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    fprintf( stderr, "+ construct PngDisplayer at %p\n", this );
    _label = new QLabel( );
    setCentralWidget( _label );

    auto pal = palette( );
    pal.setColor( QPalette::Background, Qt::black );
    setPalette( pal );
}

PngDisplayer::~PngDisplayer( ) {
    fprintf( stderr, "+ destruct PngDisplayer at %p\n", this );
}

bool PngDisplayer::load( QString const& fileName ) {
    QPixmap pixmap;
    if ( !pixmap.load( fileName ) ) {
        return false;
    }

    _label->setPixmap( pixmap );
    return true;
}
