#include "pch.h"

#include "pngdisplayer.h"

#include "app.h"
#include "utils.h"

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    setStyleSheet( QString( "QWidget { background: black }" ) );

    _label->setAlignment( Qt::AlignCenter );
    setCentralWidget( _label );

    setPalette( ModifyPalette( palette( ), QPalette::Window, Qt::black ) );

    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );

    setFixedSize( PngDisplayWindowSize );
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    move( g_settings.pngDisplayWindowPosition );
}

PngDisplayer::PngDisplayer( QString const& fileName, QWidget* parent ): PngDisplayer( parent ) {
    setImageFileName( fileName );
}

PngDisplayer::~PngDisplayer( ) {
    /*empty*/
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
