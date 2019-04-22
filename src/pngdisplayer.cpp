#include "pch.h"

#include "pngdisplayer.h"

#include "app.h"
#include "utils.h"

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    setFixedSize( PngDisplayWindowSize );
    setPalette( ModifyPalette( palette( ), QPalette::Window, Qt::black ) );
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    move( g_settings.pngDisplayWindowPosition + g_settings.projectorOffset );

    _label->setAlignment( Qt::AlignCenter );
    setCentralWidget( _label );
}

PngDisplayer::~PngDisplayer( ) {
    /*empty*/
}

void PngDisplayer::clear( ) {
    _label->clear( );
}

bool PngDisplayer::loadImageFile( QString const& fileName ) {
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
