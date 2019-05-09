#include "pch.h"

#include "processrunner.h"
#include "upgrademanager.h"
#include "utils.h"
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

    QList<int> GpgExpectedFieldCount {
         3,
         4,
         5,
         4,
        -1,
        12,
         3,
    };

    QString            const ExpectedKeyId                  { "0EF6486549978C0C76B49E99C9FC781B66B69981"                    };
    QString            const ExpectedFingerprint            { "C9FC781B66B69981"                                            };
    QString            const ExpectedSignerAddress          { "lightfield-packager@volumetricbio.com"                       };
    QString            const ExpectedSignerName             { "LightField packager <lightfield-packager@volumetricbio.com>" };

    QRegularExpression const SingleWhitespaceCharacterRegex { "\\s"                                                         };
    QRegularExpression const StartsWithWhitespaceRegex      { "^\\s+"                                                       };
    QRegularExpression const EndsWithWhitespaceRegex        { "\\s+$"                                                       };
    QRegularExpression const MetadataFieldMatcherRegex      { "^([-0-9A-Za-z]+):\\s*"                                       };
    QRegularExpression const MetadataFieldParserRegex       { "^([-0-9A-Za-z]+):\\s+(.*?)\\s+$"                             };

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

    for ( auto kitFile : QDir { upgradesPath }.entryInfoList( UpgradeKitGlobs, QDir::Files | QDir::Readable, QDir::Name ) ) {
        debug( "+ UpgradeManager::_findUpgradeKits: found kit %s\n", kitFile.fileName( ).toUtf8( ).data( ) );

        QFileInfo sigFile { kitFile.canonicalFilePath( ).append( ".sig" ) };
        if ( !sigFile.exists( ) ) {
            debug( "  + ignoring: signature file doesn't exist\n" );
            continue;
        }
        if ( !sigFile.isFile( ) ) {
            debug( "  + ignoring: signature file is not actually a file\n" );
            continue;
        }
        if ( !sigFile.isReadable( ) ) {
            debug( "  + ignoring: we do not have permission to read the signature file\n" );
            continue;
        }

        debug( "  + found signature file\n" );
        _rawUpgradeKits += std::move( UpgradeKitInfo { std::move( kitFile ), std::move( sigFile ) } );
    }

    debug( "+ UpgradeManager::checkForUpgrades: found %d upgrade kits\n", _rawUpgradeKits.count( ) );
    if ( _rawUpgradeKits.isEmpty( ) ) {
        _isChecking.clear( );
        emit upgradeCheckComplete( false );
    }

    _checkNextKitSignature( );
}

void UpgradeManager::_checkNextKitSignature( ) {
    debug( "+ UpgradeManager::_checkNextKitSignature\n" );

    if ( _rawUpgradeKits.isEmpty( ) ) {
        debug( "  + finished checking signatures, moving on to unpacking %d kits\n", _goodSigUpgradeKits.count( ) );
        _unpackNextKit( );
    }

    debug( "  + checking signature for upgrade kit %s\n", _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( ).toUtf8( ).data( ) );
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
    debug( "+ UpgradeManager::_unpackNextKit\n" );

    if ( _goodSigUpgradeKits.isEmpty( ) ) {
        debug( "  + No more kits to unpack; %d good kits found\n", _goodUpgradeKits.count( ) );
        if ( !_goodUpgradeKits.isEmpty( ) ) {
            _examineUnpackedKits( );
        }
        emit upgradeCheckComplete( !_goodUpgradeKits.isEmpty( ) );
        return;
    }

    QDir unpackDir { UpdatesRootPath + Slash + _rawUpgradeKits[0].kitFileInfo.fileName( ) };
    debug( "  + Unpacking kit '%s' into directory '%s'\n", _rawUpgradeKits[0].kitFileInfo.fileName( ).toUtf8( ).data( ), unpackDir.canonicalPath( ).toUtf8( ).data( ) );
    if ( unpackDir.exists( ) ) {
        debug( "  + Directory already exists, deleting\n" );
        unpackDir.removeRecursively( );
    }
    unpackDir.mkdir( unpackDir.canonicalPath( ) );

    _processRunner = new ProcessRunner( this );
    QObject::connect( _processRunner, &ProcessRunner::succeeded, this, &UpgradeManager::tar_succeeded );
    QObject::connect( _processRunner, &ProcessRunner::failed,    this, &UpgradeManager::tar_failed    );
    _processRunner->start(
        { "tar" },
        {
            "-C",  unpackDir.canonicalPath( ),
            "-xf", _goodSigUpgradeKits[0].kitFileInfo.canonicalFilePath( )
        }
    );
}

/*
Metadata-Version: 1
Version: 1.0.1
Build-Type: release
Release-Date: 2019-04-23
Description:
 This is the description for this update. Include information about changes
 here.
 .
 The description text needs to start in the *second* column. To add a blank
 line, put a single period ('.') in the second column.
*/

void UpgradeManager::_examineUnpackedKits( ) {
    debug( "+ UpgradeManager::_examineUnpackedKits\n" );

    for ( auto& update : _goodUpgradeKits ) {
        debug( "  + examining upgrade kit %s\n", update.kitFileInfo.fileName( ).toUtf8( ).data( ) );
        auto versionInfo = ReadWholeFile( UpdatesRootPath + Slash + update.kitFileInfo.fileName( ) + Slash + QString( "version.inf" ) ).split( NewLineRegex );

        QMap<QString, QString> fields;
        QString currentKey;
        int index = 1;
        while ( index < versionInfo.count( ) ) {
            auto& line = versionInfo[index];
            line.replace( EndsWithWhitespaceRegex, { } );

            if ( auto result = MetadataFieldMatcherRegex.match( line ); result.hasMatch( ) ) {
                currentKey = result.captured( 1 );
                line.remove( result.capturedStart( 0 ), result.capturedLength( 0 ) );
                if ( fields.contains( currentKey ) ) {
                    debug( "  + metadata for this kit contains duplicate key '%s'\n", currentKey.toUtf8( ).data( ) );
                    goto nextKit;
                }
                fields.insert( currentKey, line );
            } else if ( ( line.length( ) > 1 ) && ( line[0] == Space ) ) {
                if ( currentKey.isEmpty( ) ) {
                    debug( "  + continuation line in metadata file without anything to continue\n" );
                    goto nextKit;
                }

                line.remove( 0, 1 );
                fields[currentKey].append( LineFeed );
                if ( line != "." ) {
                    fields[currentKey].append( line );
                }
            } else {
                debug( "  + unintelligible gibberish in metadata file\n" );
                goto nextKit;
            }

            ++index;
        }

nextKit: /*empty statement*/ ;
    }
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

    auto lines = _gpgResult.split( NewLineRegex );
    if ( lines.count( ) > GpgExpectedTokens.count( ) ) {
        debug( "  + too many lines\n" );
        _rawUpgradeKits.removeFirst( );
        _checkNextKitSignature( );
        return;
    }

    auto lineIndex = 0;
    for ( auto line : lines ) {
        auto fields = line.split( SingleWhitespaceCharacterRegex );
        auto& token = fields[1];

        if ( ( GpgExpectedFieldCount[lineIndex] != -1 ) && ( GpgExpectedFieldCount[lineIndex] != fields.count( ) ) ) {
            debug( "  + wrong number of fields: expected %d, got %d\n", GpgExpectedFieldCount[lineIndex], fields.count( ) );
            goto fail;
        }
        if ( token != GpgExpectedTokens[lineIndex] ) {
            debug( "  + wrong token: expected '%s', got '%s'\n", GpgExpectedTokens[lineIndex].toUtf8( ).data( ), token.toUtf8( ).data( ) );
            goto fail;
        }
        ++lineIndex;

        if ( token == "NEWSIG" ) {
            if ( fields[2] != ExpectedSignerAddress ) {
                debug( "  + 'NEWSIG': invalid signer address: expected '%s', got '%s'\n", ExpectedSignerAddress.toUtf8( ).data( ), fields[2].toUtf8( ).data( ) );
                goto fail;
            }
        } else if ( token == "KEY_CONSIDERED" ) {
            if ( fields[2] != ExpectedKeyId ) {
                debug( "  + 'KEY_CONSIDERED': invalid key ID: expected '%s', got '%s'\n", ExpectedKeyId.toUtf8( ).data( ), fields[2].toUtf8( ).data( ) );
                goto fail;
            }
        } else if ( token == "SIG_ID" ) {
            debug( "  + 'SIG_ID': signature ID '%s', date '%s', timestamp '%s'\n", fields[2].toUtf8( ).data( ), fields[3].toUtf8( ).data( ), fields[4].toUtf8( ).data( ) );
        } else if ( token == "GOODSIG" ) {
            if ( fields[2] != ExpectedFingerprint ) {
                debug( "  + 'GOODSIG': invalid key fingerprint: expected '%s', got '%s'\n", ExpectedFingerprint.toUtf8( ).data( ), fields[2].toUtf8( ).data( ) );
                goto fail;
            }

            fields.removeFirst( );
            fields.removeFirst( );
            fields.removeFirst( );
            auto signerName = fields.join( Space );

            if ( signerName != ExpectedSignerName ) {
                debug( "  + 'GOODSIG': invalid signer name: expected '%s', got '%s'\n", ExpectedSignerName.toUtf8( ).data( ), signerName.toUtf8( ).data( ) );
                goto fail;
            }
        } else if ( token == "VALIDSIG" ) {
            if ( ( fields[2] != ExpectedKeyId ) || ( fields[11] != ExpectedKeyId ) ) {
                debug( "  + 'VALIDSIG': invalid key ID: expected '%s', got '%s' and '%s'\n", ExpectedKeyId.toUtf8( ).data( ), fields[2].toUtf8( ).data( ), fields[11].toUtf8( ).data( ) );
                goto fail;
            }
        } else if ( token == "VERIFICATION_COMPLIANCE_MODE" ) {
            if ( fields[2] != "23" ) {
                debug( "  + 'VERIFICATION_COMPLIANCE_MODE': invalid value: expected '23', got '%s'\n", fields[2].toUtf8( ).data( ) );
                goto fail;
            }
        }
    }

    debug( "  + good signature\n" );
    _goodSigUpgradeKits.append( _rawUpgradeKits.front( ) );

fail:
    _rawUpgradeKits.removeFirst( );
    _checkNextKitSignature( );
}

void UpgradeManager::gpg_failed( int const exitCode, QProcess::ProcessError const error ) {
    _processRunner->deleteLater( );

    debug( "+ UpgradeManager::gpg_failed: checking signature of upgrade kit '%s': exit status %d\n", _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( ).toUtf8( ).data( ), exitCode );
    _rawUpgradeKits.removeFirst( );
    _checkNextKitSignature( );
}

void UpgradeManager::gpg_readyReadStandardOutput( QString const& data ) {
    _gpgResult.append( data );
}

void UpgradeManager::tar_succeeded( ) {
}

void UpgradeManager::tar_failed( int const exitCode, QProcess::ProcessError const error ) {
}
