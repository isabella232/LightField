#include "pch.h"

#include "processrunner.h"
#include "upgrademanager.h"
#include "version.h"

// find kit files
// check signatures
// inspect version.inf
// fill in UpgradeKitInfo instances as we go
// announce results

namespace {

    QStringList UpgradeKitGlobs {
        "lightfield-debug_*_amd64.kit",
        "lightfield-release_*_amd64.kit"
    };

}

void UpgradeManager::checkForUpgrades( QString const& upgradesPath ) {
    if ( _isChecking.test_and_set( ) ) {
        debug( "+ UpgradeManager::checkForUpgrades: check already in progress\n" );
        return;
    }

    _thread = QThread::create( std::bind( &UpgradeManager::_checkForUpgrades, this, upgradesPath ) );
    QObject::connect( _thread, &QThread::finished, _thread, &QThread::deleteLater );
    _thread->start( );
}

void UpgradeManager::_checkForUpgrades( QString const upgradesPath ) {
    debug( "+ UpgradeManager::checkForUpgrades: looking for upgrade kits in path %s\n", upgradesPath.toUtf8( ).data( ) );
    _findUpgradeKits( upgradesPath );
    debug( "+ UpgradeManager::checkForUpgrades: found %d upgrade kits\n", _rawUpgradeKits.count( ) );
    if ( _rawUpgradeKits.isEmpty( ) ) {
        _isChecking.clear( );
        emit upgradeCheckComplete( false );
    }
    _checkNextSignature( );
}

void UpgradeManager::_findUpgradeKits( QString const& upgradesPath ) {
    for ( auto kitFile : QDir { upgradesPath }.entryInfoList( UpgradeKitGlobs, QDir::Files | QDir::Readable, QDir::Name ) ) {
        debug( "+ UpgradeManager::_findUpgradeKits: found kit %s\n", kitFile.fileName( ).toUtf8( ).data( ) );
        QFileInfo sigFile { kitFile.canonicalFilePath( ).append( ".sig" ) };
        if ( !sigFile.exists( ) || !sigFile.isFile( ) || !sigFile.isReadable( ) ) {
            debug( "  + ignoring: either signature file doesn't exist, it is not actually a file, or it is not readable by us\n" );
            continue;
        }
        debug( "  + signature file found\n" );
        _rawUpgradeKits += std::move( UpgradeKitInfo { std::move( kitFile ), std::move( sigFile ) } );
    }
}

void UpgradeManager::_checkNextSignature( ) {
    _processRunner = new ProcessRunner( this );
    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &UpgradeManager::gpg_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &UpgradeManager::gpg_failed                  );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &UpgradeManager::gpg_readyReadStandardOutput );

    //_processRunner->start(
    //    //gpg --status-fd 1 --verbose --verify lightfield-debug_1.0.1_amd64.kit.sig
    //    { "gpg" },
    //    {
    //        "--status-fd", "1",
    //        "--verbose",
    //        "--verify", _rawUpgradeKits[0].sigFileInfo.canonicalFilePath( )
    //    }
    //);
    _processRunner->start(
        // gpgv --status-fd 1 --keyring ${GNUPGHOME}/pubring.kbx /code/LightField/r1/lightfield-debug_1.0.1_amd64.kit.sig /code/LightField/r1/lightfield-debug_1.0.1_amd64.kit 2>/dev/null
        { "gpgv" },
        {
            "--status-fd", "1",
            "--keyring",   "/code/work/Volumetric/LightField/gpg/pubring.kbx",
            _rawUpgradeKits[0].sigFileInfo.canonicalFilePath( ),
            _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( )
        }
    );
}

void UpgradeManager::gpg_succeeded( ) {
}

void UpgradeManager::gpg_failed( QProcess::ProcessError const error ) {
}

void UpgradeManager::gpg_readyReadStandardOutput( QString const& data ) {
    _gpgResult.append( data );
}

/*
[GNUPG:] NEWSIG lightfield-packager@volumetricbio.com
[GNUPG:] KEY_CONSIDERED 0EF6486549978C0C76B49E99C9FC781B66B69981 0
[GNUPG:] SIG_ID bO4XJA4tRxLFCgNzlG7QIn0TNzA 2019-04-23 1556050721
[GNUPG:] KEY_CONSIDERED 0EF6486549978C0C76B49E99C9FC781B66B69981 0
[GNUPG:] GOODSIG C9FC781B66B69981 LightField packager <lightfield-packager@volumetricbio.com>
[GNUPG:] VALIDSIG 0EF6486549978C0C76B49E99C9FC781B66B69981 2019-04-23 1556050721 0 4 0 1 10 00 0EF6486549978C0C76B49E99C9FC781B66B69981
[GNUPG:] VERIFICATION_COMPLIANCE_MODE 23
*/
