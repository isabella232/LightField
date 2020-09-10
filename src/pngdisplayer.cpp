#include "pch.h"

#include "pngdisplayer.h"
#include "printjob.h"

PngDisplayer::PngDisplayer( QWidget* parent ): QMainWindow( parent ) {
    _label->setAlignment( Qt::AlignCenter );
    _label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _label->setFixedSize(ProjectorWindowSize);

    QPoint topLeft    { g_settings.projectorWindowPosition };
    QSize  windowSize { ProjectorWindowSize                };

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

    setCentralWidget(_label);
}

PngDisplayer::~PngDisplayer( ) {
    /*empty*/
}

void PngDisplayer::closeEvent( QCloseEvent* event ) {
    debug( "+ PngDisplayer::closeEvent\n" );
    event->ignore( );

    emit terminationRequested( );
}

void PngDisplayer::clear( ) {
    _label->clear();
    image = QImage();
}

bool PngDisplayer::loadImageFile( QString const& fileName ) {
    if ( !image.load(fileName) ) {
        _label->clear( );
        image = QImage();
        return false;
    }
    QPoint offset = printJob.getPrintOffset();

    int imgWidth = image.width();
    int imgHeight = image.height();
    int offsetX = g_settings.projectorOffset.x( );
    int offsetY = g_settings.projectorOffset.y( );

    int absOffsetX = (ProjectorWindowSize.width() / 2) + offsetX - offset.x() - (imgWidth/2);
    int absOffsetY = (ProjectorWindowSize.height() / 2) + offsetY + offset.y() - (imgHeight/2);

    QPixmap png { ProjectorWindowSize };
    QPainter painter (&png);
    painter.fillRect(0,0,ProjectorWindowSize.width(),ProjectorWindowSize.height(), Qt::black);
    painter.drawImage(absOffsetX, absOffsetY, image);
    _label->setPixmap(png);
    _label->repaint();

    return true;
}

void PngDisplayer::setPixmap( QPixmap const& pixmap ) {
    _label->setPixmap( pixmap );
    _label->repaint();

}

void PngDisplayer::printJobChanged() {
    connect(&printJob, &PrintJob::printOffsetChanged, this, &PngDisplayer::moveToOffset);
}

void PngDisplayer::moveToOffset(QPoint offset) {
    int imgWidth = image.width();
    int imgHeight = image.height();
    int offsetX = g_settings.projectorOffset.x( );
    int offsetY = g_settings.projectorOffset.y( );

    int absOffsetX = (ProjectorWindowSize.width() / 2) + offsetX - offset.x() - (imgWidth/2);
    int absOffsetY = (ProjectorWindowSize.height() / 2) + offsetY + offset.y() - (imgHeight/2);

    QPixmap png { ProjectorWindowSize };
    QPainter painter (&png);
    painter.fillRect(0,0,ProjectorWindowSize.width(),ProjectorWindowSize.height(), Qt::black);
    painter.drawImage(absOffsetX, absOffsetY, image);
    _label->setPixmap(png);
    _label->repaint();

    update();
}
