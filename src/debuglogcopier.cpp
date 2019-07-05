#include "pch.h"

#include "filecopier.h"
#include "debuglogcopier.h"
#include "window.h"

DebugLogCopier::DebugLogCopier( UsbMountManager* manager, QWidget* parent ): QObject( parent ) {
    _usbMountManager = manager;
}

DebugLogCopier::~DebugLogCopier( ) {
    /*empty*/
}

void DebugLogCopier::copyDebugLogs( QString const& targetPath ) {
    _targetPath = targetPath;
}

void DebugLogCopier::_startCopy( ) {
    QVector<QPair<QString, QString>> fileList;
    for ( QString path : DebugLogPaths ) {
        fileList.append( QPair<QString, QString> { path, _targetPath % Slash % path.right( path.length( ) - path.lastIndexOf( Slash ) - 1 ) } );
    }

    _fileCopier = new FileCopier( static_cast<QWidget*>( parent( ) ) );
    QObject::connect( _fileCopier, &FileCopier::finished, this, &DebugLogCopier::fileCopier_finished );

    _fileCopier->show( );

    auto mainWindow = getMainWindow( );
    mainWindow->hide( );

    _fileCopier->startCopy( fileList );
}

void DebugLogCopier::fileCopier_finished( ) {
    debug( "+ DebugLogCopier::fileCopier_finished\n" );

    auto mainWindow = getMainWindow( );
    mainWindow->show( );

    _fileCopier->close( );
    _fileCopier->deleteLater( );
    _fileCopier = nullptr;
}
