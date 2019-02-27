#include "pch.h"

#include "pngdisplayer.h"

#include "app.h"
#include "utils.h"

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    debug( "+ construct PngDisplayer at %p\n", this );
    _label = new QLabel( );
    _label->setAlignment( Qt::AlignCenter );
    setCentralWidget( _label );

    setPalette( ModifyPalette( palette( ), QPalette::Window, Qt::black ) );

    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
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
