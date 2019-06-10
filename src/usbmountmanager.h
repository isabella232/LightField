#ifndef __USBMOUNTMANAGER_H__
#define __USBMOUNTMANAGER_H__

class ProcessRunner;

class UsbMountManager: public QObject {

    Q_OBJECT

public:

    UsbMountManager( QObject* parent = nullptr );
    virtual ~UsbMountManager( ) override;

protected:

private:

    ProcessRunner* _mountmonProcess { };
    QString        _stdoutBuffer;
    QString        _stderrBuffer;

signals:
    ;

    void filesystemMounted( QString const& mountPoint );
    void filesystemUnmounted( QString const& mountPoint );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void mountmon_failed( int const exitCode, QProcess::ProcessError const error );
    void mountmon_readyReadStandardOutput( QString const& data );
    void mountmon_readyReadStandardError( QString const& data );

};

#endif // __USBMOUNTMANAGER_H__
