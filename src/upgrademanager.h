#ifndef __UPGRADEMANAGER_H__
#define __UPGRADEMANAGER_H__

#include "version.h"

//
// Forward declarations
//

class GpgSignatureChecker;
class Hasher;
class ProcessRunner;
class UpgradeKitUnpacker;

//
// Class UpgradeKitInfo
//

class UpgradeKitInfo {

public:

    UpgradeKitInfo( QString const& directory_ ):
        isAlreadyUnpacked { true                },
        directory         { QDir { directory_ } }
    {
        /*empty*/
    }

    UpgradeKitInfo( QFileInfo const& kitFileInfo_, QFileInfo const& sigFileInfo_ ):
        isAlreadyUnpacked { false        },
        kitFileInfo       { kitFileInfo_ },
        sigFileInfo       { sigFileInfo_ }
    {
        /*empty*/
    }

    bool                   isAlreadyUnpacked     { };

    QDir                   directory;
    QFileInfo              kitFileInfo;
    QFileInfo              sigFileInfo;

    QString                metadataVersionString;
    int                    metadataVersion       { };

    QString                versionString;
    unsigned               version               { };

    BuildType              buildType             { };

    QDate                  releaseDate;

    QString                description;

    QMap<QString, QString> checksums;

};

using UpgradeKitInfoList = QList<UpgradeKitInfo>;

//
// Class UpgradeManager
//

class UpgradeManager: public QObject {

    Q_OBJECT

public:

    UpgradeManager( QObject* parent = nullptr );
    virtual ~UpgradeManager( ) override;

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

    std::atomic_flag     _isChecking             { ATOMIC_FLAG_INIT };

    GpgSignatureChecker* _gpgSignatureChecker    { };
    Hasher*              _hashChecker            { };
    ProcessRunner*       _processRunner          { };
    UpgradeKitUnpacker*  _upgradeKitUnpacker     { };
    UpgradeKitInfo*      _kitToInstall           { };

    UpgradeKitInfoList   _unprocessedUpgradeKits;
    UpgradeKitInfoList   _processedUpgradeKits;
    UpgradeKitInfoList   _goodUpgradeKits;

    QString              _stdoutBuffer;
    QString              _stderrBuffer;

    void _checkForUpgrades( QString const& upgradesPath );
    void _checkNextKitSignature( );
    void _unpackNextKit( );
    void _checkNextVersionInfSignature( );
    bool _parseVersionInfo( QString const& versionInfoFileName, UpgradeKitInfo& info );
    void _examineUnpackedKits( );
    void _checkNextKitsHashes( );
    void _dumpBufferContents( );

signals:
    ;

    void upgradeCheckComplete( bool const upgradesFound );
    void upgradeFailed( );

public slots:
    ;

    void checkForUpgrades( QString const& upgradesPath );
    void installUpgradeKit( UpgradeKitInfo const& kit );

protected slots:
    ;

private slots:
    ;

    void gpgSignatureChecker_kit_complete( bool const result );
    void gpgSignatureChecker_versionInf_complete( bool const result );

    void hasher_hashCheckResult( bool const result );

#if defined _DEBUG
    void upgradeKitUnpacker_complete( bool const result, QString const& tarOutput, QString const& tarError );
#else
    void upgradeKitUnpacker_complete( bool const result );
#endif // defined _DEBUG

    void readyReadStandardOutput( QString const& data );
    void readyReadStandardError( QString const& data );

    void aptGetUpdate_succeeded( );
    void aptGetUpdate_failed( int const exitCode, QProcess::ProcessError const error );

    void aptGetInstall_succeeded( );
    void aptGetInstall_failed( int const exitCode, QProcess::ProcessError const error );

    void aptGetDistUpgrade_succeeded( );
    void aptGetDistUpgrade_failed( int const exitCode, QProcess::ProcessError const error );

};

#endif // __UPGRADEMANAGER_H__
