#ifndef __DEBUGLOGCOPIER_H__
#define __DEBUGLOGCOPIER_H__

#include <QtCore>
#include <QtWidgets>
#include "filecopier.h"
#include "initialshoweventmixin.h"
#include "usbmountmanager.h"

class UsbMountManager;

class DebugLogCopier: public InitialShowEventMixin<DebugLogCopier, QMainWindow> {

    Q_OBJECT

public:

    DebugLogCopier( UsbMountManager* manager, QWidget* parent = nullptr );
    virtual ~DebugLogCopier( ) override;

    void copyTo( QString const& mountPointPath );

protected:

    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    UsbMountManager* _usbMountManager      { };
    FileCopier*      _fileCopier           { };
    FileNamePairList _fileList;
    QString          _targetPath;

    QLabel*          _message              { new QLabel       };
    QWidget*         _messageWidget        { new QWidget      };

    QLabel*          _currentFileNameLabel { new QLabel       };
    QLabel*          _currentFileName      { new QLabel       };
    QLabel*          _fileSizeLabel        { new QLabel       };
    QLabel*          _fileSize             { new QLabel       };
    QProgressBar*    _progressBar          { new QProgressBar };
    QLabel*          _notifications        { new QLabel       };
    QWidget*         _copyStatusWidget     { new QWidget      };

    QPushButton*     _button               { new QPushButton  };

    void _remountRw_start( );
    void _fileCopier_start( );
    void _remountRo_start( );
    void _showOkButton( );
    void _showMessage( QString const& message );

signals:
    void finished( );

private slots:
    void abortButton_clicked( bool );
    void okButton_clicked( bool );

    void remountRw_finished(UsbDevice const &dev, bool succeeded);

    void fileCopier_fileStarted( int const index, qint64 const totalSize );
    void fileCopier_fileProgress( int const index, qint64 const bytesCopied );
    void fileCopier_fileFinished( int const index, qint64 const bytesCopied );

    void fileCopier_notify( int const index, QString const message );
    void fileCopier_finished( int const copiedFiles, int const skippedFiles );

    void remountRo_finished(UsbDevice const &dev, bool succeeded);

};

#endif // __DEBUGLOGCOPIER_H__
