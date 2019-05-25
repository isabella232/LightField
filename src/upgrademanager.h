#ifndef __UPGRADEMANAGER_H__
#define __UPGRADEMANAGER_H__

#include "version.h"

//
// Forward declarations
//

class GpgSignatureChecker;
class UpgradeKitUnpacker;

//
// Class UpgradeKitInfo
//

class UpgradeKitInfo {

public:

    UpgradeKitInfo( QDir const& directory_ ): directory { directory_ } {
        /*empty*/
    }

    UpgradeKitInfo( QFileInfo const& kitFileInfo_, QFileInfo const& sigFileInfo_ ): kitFileInfo { kitFileInfo_ }, sigFileInfo { sigFileInfo_ } {
        /*empty*/
    }

    QFileInfo kitFileInfo;
    QFileInfo sigFileInfo;
    QDir      directory;

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

    std::atomic_flag     _isChecking          { ATOMIC_FLAG_INIT };

    GpgSignatureChecker* _gpgSignatureChecker { };
    UpgradeKitUnpacker*  _upgradeKitUnpacker  { };

    UpgradeKitInfoList   _rawUpgradeKits;
    UpgradeKitInfoList   _goodSigUpgradeKits;
    UpgradeKitInfoList   _goodUpgradeKits;

    void _checkForUpgrades( QString const& upgradesPath );
    void _checkNextKitSignature( );
    void _unpackNextKit( );
    bool _parseVersionInfo( QString const& versionInfoFileName, UpgradeKitInfo& info );
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

    void gpgSignatureChecker_kit_complete( bool const result, QStringList const& results );

    void upgradeKitUnpacker_complete( bool const result, QString const& tarOutput, QString const& tarError );

};

#endif // __UPGRADEMANAGER_H__
