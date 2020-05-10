#ifndef __UPGRADEKITUNPACKER_H__
#define __UPGRADEKITUNPACKER_H__

#include <QtCore>

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
#if defined _DEBUG
    QString        _tarOutput;
    QString        _tarError;
#endif // defined _DEBUG

signals:

#if defined _DEBUG
    void unpackComplete( bool const result, QString const& output, QString const& error );
#else
    void unpackComplete( bool const result );
#endif // defined _DEBUG

public slots:

protected slots:

private slots:

    void tar_succeeded( );
    void tar_failed( int const exitCode, QProcess::ProcessError const error );
#if defined _DEBUG
    void tar_readyReadStandardOutput( QString const& data );
    void tar_readyReadStandardError( QString const& data );
#endif // defined _DEBUG

};

#endif // __UPGRADEKITUNPACKER__
