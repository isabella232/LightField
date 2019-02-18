#include "pch.h"

#include "pngdisplayer.h"

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    debug( "+ construct PngDisplayer at %p\n", this );
    _label = new QLabel( );
    _label->setAlignment( Qt::AlignCenter );
    setCentralWidget( _label );

    auto pal = palette( );
    pal.setColor( QPalette::Background, Qt::black );
    setPalette( pal );

    setWindowFlags( windowFlags( ) | Qt::FramelessWindowHint );
}

PngDisplayer::~PngDisplayer( ) {
    debug( "+ destruct PngDisplayer at %p\n", this );
}

bool PngDisplayer::load( QString const& fileName ) {
    QPixmap pixmap;
    if ( !pixmap.load( fileName ) ) {
        return false;
    }

    _label->setPixmap( pixmap );
    return true;
}
