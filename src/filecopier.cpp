#include "pch.h"

#include "filecopier.h"

FileCopier::FileCopier( QWidget* parent ): InitialShowEventMixin<FileCopier, QMainWindow>( parent ) {
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    setFixedSize( MainWindowSize );
    move( g_settings.mainWindowPosition );

    setFont( ModifyFont( font( ), 22.0 ) );
}

FileCopier::~FileCopier( ) {
    /*empty*/
}

void FileCopier::_initialShowEvent( QShowEvent* event ) {
    /*empty*/
}

void FileCopier::setFileList( QVector<QPair<QString, QString>> const& fileList ) {
    _fileList = fileList;
}

void FileCopier::startCopy( ) {
}

void FileCopier::abortCopy( ) {
}
