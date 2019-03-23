#include "pch.h"

#include "pngdisplayer.h"

#include "app.h"
#include "utils.h"

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    debug( "+ construct PngDisplayer at %p\n", this );

    setStyleSheet( QString( "QWidget { background: black }" ) );

    _label->setAlignment( Qt::AlignCenter );
    setCentralWidget( _label );

    setPalette( ModifyPalette( palette( ), QPalette::Window, Qt::black ) );

    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
}

PngDisplayer::~PngDisplayer( ) {
    debug( "+ destruct PngDisplayer at %p\n", this );
}

void PngDisplayer::clear( ) {
    _label->clear( );
}

bool PngDisplayer::setImageFileName( QString const& fileName ) {
    if ( !_png.load( fileName ) ) {
        _label->clear( );
        return false;
    }

    _label->setPixmap( _png );
    return true;
}

void PngDisplayer::setPixmap( QPixmap const& pixmap ) {
    _label->setPixmap( pixmap );
}
