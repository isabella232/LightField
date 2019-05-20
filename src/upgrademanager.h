#ifndef __UPGRADEMANAGER_H__
#define __UPGRADEMANAGER_H__

/*
#include <atomic>

#include <QObject>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QThread>
*/

#include "version.h"

//
// Forward declarations
//

class ProcessRunner;

//
// Class UpgradeKitInfo
//

class UpgradeKitInfo {

public:

    UpgradeKitInfo( QFileInfo&& kitFileInfo_, QFileInfo&& sigFileInfo_ ):
        kitFileInfo { std::move( kitFileInfo_ ) },
        sigFileInfo { std::move( sigFileInfo_ ) }
    {
        /*empty*/
    }

    QFileInfo kitFileInfo;
    QFileInfo sigFileInfo;
    QString   version;
    unsigned  versionCode { };
    QDate     releaseDate;
    BuildType buildType   { };
    QString   description;

};

using UpgradeKitInfoList = QList<UpgradeKitInfo>;

//
// Class UpgradeManager
//

class UpgradeManager: public QObject {

    Q_OBJECT

public:

    UpgradeKitInfoList const& availableUpgrades( ) { return _goodUpgradeKits; }

    bool isCheckingForUpgrades( ) {
        bool value = _isChecking.test_and_set( );
        if ( !value ) {
            _isChecking.clear( );
        }
        return value;
    }

protected:

private:

    std::atomic_flag   _isChecking         { ATOMIC_FLAG_INIT };

    ProcessRunner*     _processRunner      { };

    UpgradeKitInfoList _rawUpgradeKits;
    UpgradeKitInfoList _goodSigUpgradeKits;
    UpgradeKitInfoList _goodUpgradeKits;

    QString            _gpgResult;

#if defined _DEBUG
    QString            _tarOutput;
    QString            _tarError;
#endif // defined _DEBUG

    void _checkForUpgrades( QString const upgradesPath );
    void _checkNextKitSignature( );
    void _unpackNextKit( );
    void _examineUnpackedKits( );

signals:
    ;

    void upgradeCheckComplete( bool const found );

public slots:
    ;

    void checkForUpgrades( QString const& upgradesPath );

protected slots:
    ;

private slots:
    ;

    void gpg_succeeded( );
    void gpg_failed( int const exitCode, QProcess::ProcessError const error );
    void gpg_readyReadStandardOutput( QString const& data );

    void tar_succeeded( );
    void tar_failed( int const exitCode, QProcess::ProcessError const error );
#if defined _DEBUG
    void tar_readyReadStandardOutput( QString const& data );
    void tar_readyReadStandardError( QString const& data );
#endif // _DEBUG

};

#endif // __UPGRADEMANAGER_H__
