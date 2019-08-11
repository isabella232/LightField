#ifndef __USBMOUNTMANAGER_H__
#define __USBMOUNTMANAGER_H__

class ProcessRunner;
class StdioLogger;

class UsbMountManager: public QObject {

    Q_OBJECT

public:

    UsbMountManager( QObject* parent = nullptr );
    virtual ~UsbMountManager( ) override;

    void remount( bool const writable );

protected:

private:

    QString        _mountPoint;
    bool           _isWritable    { };

    ProcessRunner* _processRunner { };
    StdioLogger*   _stderrLogger  { };
    StdioLogger*   _stdoutLogger  { };
    QString        _stdoutBuffer;

signals:
    ;

    void filesystemMounted( QString const& mountPoint );
    void filesystemRemounted( bool const succeeded, bool const writable );
    void filesystemUnmounted( QString const& mountPoint );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void mountmon_failed( int const exitCode, QProcess::ProcessError const error );
    void mountmon_readyReadStandardOutput( QString const& data );

};

#endif // __USBMOUNTMANAGER_H__
