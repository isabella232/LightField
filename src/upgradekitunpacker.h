#ifndef __UPGRADEKITUNPACKER_H__
#define __UPGRADEKITUNPACKER_H__

class ProcessRunner;

class UpgradeKitUnpacker: public QObject {

    Q_OBJECT

public:

    UpgradeKitUnpacker( QObject* parent = nullptr );
    virtual ~UpgradeKitUnpacker( ) override;

    void startUnpacking( QString const& fileName, QString const& targetDirectory );

    int instanceId( ) const;
    QProcess::ProcessState state( ) const;

protected:

private:

    ProcessRunner* _processRunner { };
    QString        _tarOutput;
    QString        _tarError;

signals:

    void unpackComplete( bool const result, QString const& output, QString const& error );

public slots:

protected slots:

private slots:

    void tar_succeeded( );
    void tar_failed( int const exitCode, QProcess::ProcessError const error );
    void tar_readyReadStandardOutput( QString const& data );
    void tar_readyReadStandardError( QString const& data );

};

#endif // __UPGRADEKITUNPACKER__
