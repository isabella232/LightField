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

    QStringList GpgExpectedTokens {
        "NEWSIG",
        "KEY_CONSIDERED",
        "SIG_ID",
        "KEY_CONSIDERED",
        "GOODSIG",
        "VALIDSIG",
        "VERIFICATION_COMPLIANCE_MODE",
    };

    QList<int> GpgExpectedTokenCount {
         3,
         4,
         5,
         4,
        -1,
        12,
         3,
    };

    QString ExpectedKeyId         { "0EF6486549978C0C76B49E99C9FC781B66B69981"                    };
    QString ExpectedFingerprint   { "C9FC781B66B69981"                                            };
    QString ExpectedSignerAddress { "lightfield-packager@volumetricbio.com"                       };
    QString ExpectedSignerName    { "LightField packager <lightfield-packager@volumetricbio.com>" };

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
        debug( "  + found signature file\n" );
        _rawUpgradeKits += std::move( UpgradeKitInfo { std::move( kitFile ), std::move( sigFile ) } );
    }
}

void UpgradeManager::_checkNextSignature( ) {
    if ( _rawUpgradeKits.count( ) == 0 ) {
        _unpackNextKit( );
    }

    _processRunner = new ProcessRunner( this );
    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &UpgradeManager::gpg_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &UpgradeManager::gpg_failed                  );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &UpgradeManager::gpg_readyReadStandardOutput );
    _processRunner->start(
        { "gpgv" },
        {
            "--status-fd", "1",
            "--keyring",   GpgKeyRingPath,
            _rawUpgradeKits[0].sigFileInfo.canonicalFilePath( ),
            _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( )
        }
    );
}

void UpgradeManager::_unpackNextKit( ) {
    if ( _goodSigUpgradeKits.count( ) == 0 ) {
        emit upgradeCheckComplete( _goodUpgradeKits.count( ) > 0 );
        return;
    }

    QDir unpackDir { UpdatesRootPath + Slash + _rawUpgradeKits[0].kitFileInfo.fileName( ) };
    if ( unpackDir.exists( ) ) {
        unpackDir.removeRecursively( );
    }
    unpackDir.mkdir( unpackDir.canonicalPath( ) );

    _processRunner = new ProcessRunner( this );
    QObject::connect( _processRunner, &ProcessRunner::succeeded, this, &UpgradeManager::tar_succeeded );
    QObject::connect( _processRunner, &ProcessRunner::failed,    this, &UpgradeManager::tar_failed    );
    _processRunner->start(
        { "gpgv" },
        {
            "--status-fd", "1",
            "--keyring",   GpgKeyRingPath,
            _rawUpgradeKits[0].sigFileInfo.canonicalFilePath( ),
            _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( )
        }
    );
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

void UpgradeManager::gpg_succeeded( ) {
    _processRunner->deleteLater( );

    debug( "+ UpgradeManager::gpg_succeeded: examining GPG output for upgrade kit '%s'\n", _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( ).toUtf8( ).data( ) );

    auto lines = _gpgResult.split( QChar( '\n' ) );
    if ( lines.count( ) > GpgExpectedTokens.count( ) ) {
        debug( "  + too many lines\n" );
        _rawUpgradeKits.removeFirst( );
        _checkNextSignature( );
        return;
    }

    auto expectedTokenIndex = 0;
    for ( auto line : lines ) {
        auto tokens = line.split( QRegularExpression( "\\s" ) );
        if ( expectedTokenIndex >= GpgExpectedTokens.count( ) ) {
            debug( "  + too many tokens\n" );
            goto fail;
        }
        if ( tokens[1] != GpgExpectedTokens[expectedTokenIndex] ) {
            debug( "  + invalid token: expected '%s', got '%s'\n", GpgExpectedTokens[expectedTokenIndex].toUtf8( ).data( ), tokens[1].toUtf8( ).data( ) );
            goto fail;
        }
        if ( ( GpgExpectedTokenCount[expectedTokenIndex] != -1 ) && ( GpgExpectedTokenCount[expectedTokenIndex] != tokens.count( ) ) ) {
            debug( "  + wrong number of tokens: expected %d, got %d\n", GpgExpectedTokenCount[expectedTokenIndex], tokens.count( ) );
            goto fail;
        }
        ++expectedTokenIndex;

        if ( tokens[1] == "NEWSIG" ) {
            if ( tokens[2] != ExpectedSignerAddress ) {
                debug( "  + 'NEWSIG': invalid signer address: expected '%s', got '%s'\n", ExpectedSignerAddress.toUtf8( ).data( ), tokens[2].toUtf8( ).data( ) );
                goto fail;
            }
        } else if ( tokens[1] == "KEY_CONSIDERED" ) {
            if ( tokens[2] != ExpectedKeyId ) {
                debug( "  + 'KEY_CONSIDERED': invalid key ID: expected '%s', got '%s'\n", ExpectedKeyId.toUtf8( ).data( ), tokens[2].toUtf8( ).data( ) );
                goto fail;
            }
        } else if ( tokens[1] == "SIG_ID" ) {
            debug( "  + 'SIG_ID': signature ID '%s', date '%s', timestamp '%s'\n", tokens[2].toUtf8( ).data( ), tokens[3].toUtf8( ).data( ), tokens[4].toUtf8( ).data( ) );
        } else if ( tokens[1] == "GOODSIG" ) {
            if ( tokens[2] != ExpectedFingerprint ) {
                debug( "  + 'GOODSIG': invalid key fingerprint: expected '%s', got '%s'\n", ExpectedFingerprint.toUtf8( ).data( ), tokens[2].toUtf8( ).data( ) );
                goto fail;
            }

            tokens.removeFirst( );
            tokens.removeFirst( );
            tokens.removeFirst( );
            auto signerName = tokens.join( Space );

            if ( signerName != ExpectedSignerName ) {
                debug( "  + 'GOODSIG': invalid signer name: expected '%s', got '%s'\n", ExpectedSignerName.toUtf8( ).data( ), signerName.toUtf8( ).data( ) );
                goto fail;
            }
        } else if ( tokens[1] == "VALIDSIG" ) {
            if ( ( tokens[2] != ExpectedKeyId ) || ( tokens[11] != ExpectedKeyId ) ) {
                debug( "  + 'VALIDSIG': invalid key ID: expected '%s', got '%s' and '%s'\n", ExpectedKeyId.toUtf8( ).data( ), tokens[2].toUtf8( ).data( ), tokens[11].toUtf8( ).data( ) );
                goto fail;
            }
        } else if ( tokens[1] == "VERIFICATION_COMPLIANCE_MODE" ) {
            if ( tokens[2] != "23" ) {
                debug( "  + 'VERIFICATION_COMPLIANCE_MODE': invalid value: expected '23', got '%s'\n", tokens[2].toUtf8( ).data( ) );
                goto fail;
            }
        } else {
            debug( "  + unexpected token: got '%s'\n", tokens[1].toUtf8( ).data( ) );
            goto fail;
        }
    }

    // TODO good
    _goodSigUpgradeKits.append( _rawUpgradeKits.front( ) );
    _rawUpgradeKits.removeFirst( );
    _checkNextSignature( );
    return;

fail:
    _rawUpgradeKits.removeFirst( );
    _checkNextSignature( );
}

void UpgradeManager::gpg_failed( int const exitCode, QProcess::ProcessError const error ) {
    _processRunner->deleteLater( );

    debug( "+ UpgradeManager::gpg_failed: checking signature of upgrade kit '%s': exit status %d\n", _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( ).toUtf8( ).data( ), exitCode );
    _rawUpgradeKits.removeFirst( );
    _checkNextSignature( );
}

void UpgradeManager::gpg_readyReadStandardOutput( QString const& data ) {
    _gpgResult.append( data );
}

void UpgradeManager::tar_succeeded( ) {
}

void UpgradeManager::tar_failed( int const exitCode, QProcess::ProcessError const error ) {
}
