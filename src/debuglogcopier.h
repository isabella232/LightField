#ifndef __DEBUGLOGCOPIER_H__
#define __DEBUGLOGCOPIER_H__

class FileCopier;
class UsbMountManager;

class DebugLogCopier: public QObject {

    Q_OBJECT

public:

    DebugLogCopier( UsbMountManager* manager, QWidget* parent = nullptr );
    virtual ~DebugLogCopier( ) override;

    void copyDebugLogs( QString const& targetPath );

protected:

private:

    UsbMountManager* _usbMountManager { };
    FileCopier*      _fileCopier      { };
    QString          _targetPath;

    void _startCopy( );

signals:
    ;

    void finished( );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void fileCopier_finished( );

};


#endif // __DEBUGLOGCOPIER_H__
