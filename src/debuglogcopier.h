#ifndef __DEBUGLOGCOPIER_H__
#define __DEBUGLOGCOPIER_H__

#include "filecopier.h"
#include "initialshoweventmixin.h"

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
    QVBoxLayout*     _messageLayout        {                  };

    QLabel*          _currentFileNameLabel { new QLabel       };
    QLabel*          _currentFileName      { new QLabel       };
    QLabel*          _fileSizeLabel        { new QLabel       };
    QLabel*          _fileSize             { new QLabel       };
    QProgressBar*    _progressBar          { new QProgressBar };
    QLabel*          _notifications        { new QLabel       };
    QVBoxLayout*     _copyStatusLayout     { new QVBoxLayout  };

    QWidget*         _innerWidget          { new QWidget      };

    QPushButton*     _button               { new QPushButton  };

    void _remountRw_start( );
    void _fileCopier_start( );
    void _remountRo_start( );
    void _showOkButton( );
    void _showMessage( QString const& message );

signals:
    ;

    void finished( );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void abortButton_clicked( bool );
    void okButton_clicked( bool );

    void remountRw_finished( bool const succeeded, bool const /*writable*/ );

    void fileCopier_fileStarted( int const index, qint64 const totalSize );
    void fileCopier_fileProgress( int const index, qint64 const bytesCopied );
    void fileCopier_fileFinished( int const index, qint64 const bytesCopied );

    void fileCopier_notify( int const index, QString const message );
    void fileCopier_failure( int const index, QString const message );
    void fileCopier_finished( int const copiedFiles, int const skippedFiles );

    void remountRo_finished( bool const succeeded, bool const /*writable*/ );

};

#endif // __DEBUGLOGCOPIER_H__
