#include "pch.h"

#include "pngdisplayer.h"

#include "app.h"
#include "strings.h"
#include "utils.h"

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    _label->setAlignment( Qt::AlignCenter );

    QPoint topLeft    { g_settings.pngDisplayWindowPosition };
    QSize  windowSize { PngDisplayWindowSize                };

    auto offsetX = g_settings.projectorOffset.x( );
    auto offsetY = g_settings.projectorOffset.y( );

    if ( offsetX < 0 ) {
        topLeft.setX( topLeft.x( ) + offsetX );
    }
    if ( offsetY < 0 ) {
        topLeft.setY( topLeft.y( ) + offsetY );
    }

    windowSize += QSize { abs( offsetX ), abs( offsetY ) };

    setFixedSize( windowSize );
    setPalette( ModifyPalette( palette( ), QPalette::Window, Qt::black ) );
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    move( topLeft );

    auto topPadding = new QWidget;
    topPadding->setContentsMargins( { } );
    topPadding->setFixedSize( windowSize.width( ), offsetY );
    topPadding->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    auto leftPadding = new QWidget;
    leftPadding->setContentsMargins( { } );
    leftPadding->setFixedSize( offsetX, windowSize.height( ) );
    leftPadding->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget( leftPadding );
    hbox->addWidget( _label );
    hbox->setContentsMargins( { } );

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget( topPadding );
    vbox->addLayout( hbox );
    vbox->setContentsMargins( { } );

    QWidget* centralWidget = new QWidget;
    centralWidget->setLayout( vbox );

    setCentralWidget( centralWidget );
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
