#ifndef __UPGRADEMANAGER_H__
#define __UPGRADEMANAGER_H__

class ProcessRunner;

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

};

using UpgradeKitInfoList = QList<UpgradeKitInfo>;

class UpgradeManager: public QObject {

    Q_OBJECT;

public:

    UpgradeKitInfoList availableUpgrades( ) { return _goodUpgradeKits; }

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

    QThread*           _thread             { };
    ProcessRunner*     _processRunner      { };

    UpgradeKitInfoList     _rawUpgradeKits;
    UpgradeKitInfoList _goodSigUpgradeKits;
    UpgradeKitInfoList    _goodUpgradeKits;

    QString _gpgResult;

    void _checkForUpgrades( QString const upgradesPath );
    void _findUpgradeKits( QString const& upgradesPath );
    void _checkNextSignature( );

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

};

#endif // __UPGRADEMANAGER_H__
