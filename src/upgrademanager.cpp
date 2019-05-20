#include "pch.h"

#include "upgrademanager.h"

#include "gpgsignaturechecker.h"
#include "processrunner.h"
#include "strings.h"
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

    QMap<QString, BuildType> StringToBuildType {
        { "debug",   BuildType::Debug   },
        { "release", BuildType::Release },
    };

    QString            const ExpectedKeyId                  { "0EF6486549978C0C76B49E99C9FC781B66B69981"                    };
    QString            const ExpectedFingerprint            { "C9FC781B66B69981"                                            };
    QString            const ExpectedSignerAddress          { "lightfield-packager@volumetricbio.com"                       };
    QString            const ExpectedSignerName             { "LightField packager <lightfield-packager@volumetricbio.com>" };

    QRegularExpression const EndsWithWhitespaceRegex        { "\\s+$"                                                       };
    QRegularExpression const MetadataFieldMatcherRegex      { "^([-0-9A-Za-z]+):\\s*"                                       };
    QRegularExpression const MetadataFieldParserRegex       { "^([-0-9A-Za-z]+):\\s+(.*?)\\s+$"                             };
    QRegularExpression const SingleWhitespaceCharacterRegex { "\\s"                                                         };
    QRegularExpression const StartsWithWhitespaceRegex      { "^\\s+"                                                       };

}

UpgradeManager::UpgradeManager( QObject* parent ): QObject( parent ) {
    /*empty*/
}

UpgradeManager::~UpgradeManager( ) {
    /*empty*/
}

void UpgradeManager::checkForUpgrades( QString const& upgradesPath ) {
    if ( _isChecking.test_and_set( ) ) {
        debug( "+ UpgradeManager::checkForUpgrades: check already in progress\n" );
        return;
    }

    _checkForUpgrades( upgradesPath );
}

void UpgradeManager::_checkForUpgrades( QString const upgradesPath ) {
    debug( "+ UpgradeManager::_checkForUpgrades: looking for upgrade kits in path %s\n", upgradesPath.toUtf8( ).data( ) );

    for ( auto kitFile : QDir { upgradesPath }.entryInfoList( UpgradeKitGlobs, QDir::Files | QDir::Readable, QDir::Name ) ) {
        debug( "+ UpgradeManager::_checkForUpgrades: found kit %s\n", kitFile.fileName( ).toUtf8( ).data( ) );

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

    debug( "+ UpgradeManager::_checkForUpgrades: found %d upgrade kits\n", _rawUpgradeKits.count( ) );
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
        return;
    }

    debug( "  + checking signature for upgrade kit %s\n", _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( ).toUtf8( ).data( ) );

    _gpgSignatureChecker = new GpgSignatureChecker( this );
    QObject::connect( _gpgSignatureChecker, &GpgSignatureChecker::signatureCheckComplete, this, &UpgradeManager::gpgSignatureChecker_complete );
    _gpgSignatureChecker->startCheckDetachedSignature( _rawUpgradeKits[0].sigFileInfo.canonicalFilePath( ), _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( ) );
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

void UpgradeManager::gpgSignatureChecker_complete( bool const result, QStringList const& /*results*/ ) {
    _gpgSignatureChecker->deleteLater( );
    _gpgSignatureChecker = nullptr;

    debug( "+ UpgradeManager::gpgSignatureChecker_complete: result is %s\n", ToString( result ) );

    if ( result ) {
        debug( "+ UpgradeManager::gpgSignatureChecker_complete: good signature\n" );
        _goodSigUpgradeKits.append( _rawUpgradeKits.front( ) );
    } else {
        debug( "+ UpgradeManager::gpgSignatureChecker_complete: bad signature\n" );
    }

    _rawUpgradeKits.removeFirst( );
    _checkNextKitSignature( );
}

void UpgradeManager::_unpackNextKit( ) {
    debug( "+ UpgradeManager::_unpackNextKit\n" );

    if ( _goodSigUpgradeKits.isEmpty( ) ) {
        debug( "  + No more kits to unpack; %d good kits found\n", _goodUpgradeKits.count( ) );
        if ( !_goodUpgradeKits.isEmpty( ) ) {
            _examineUnpackedKits( );
        }
        _isChecking.clear( );
        emit upgradeCheckComplete( !_goodUpgradeKits.isEmpty( ) );
        return;
    }

    debug(
        "  + UpdatesRootPath:                                {%s}\n"
        "  + _goodSigUpgradeKits[0].kitFileInfo.fileName( ): {%s}\n"
        "",
        UpdatesRootPath.toUtf8( ).data( ),
        _goodSigUpgradeKits[0].kitFileInfo.fileName( ).toUtf8( ).data( )
    );

    QString kitFilePath { _goodSigUpgradeKits[0].kitFileInfo.canonicalFilePath( ) };

    QString unpackDirName { UpdatesRootPath + Slash + _goodSigUpgradeKits[0].kitFileInfo.fileName( ) };
    if ( unpackDirName.endsWith( ".kit" ) ) {
        unpackDirName = unpackDirName.left( unpackDirName.length( ) - 4 );
    }
    QDir unpackDir { unpackDirName };

    QString unpackPath { unpackDir.absolutePath( ) };
    debug( "  + Unpacking kit '%s' into directory '%s'\n", kitFilePath.toUtf8( ).data( ), unpackPath.toUtf8( ).data( ) );

    if ( unpackDir.exists( ) ) {
        debug( "  + Directory already exists, deleting\n" );
        unpackDir.removeRecursively( );
    }
    unpackDir.mkdir( unpackPath );
    _goodSigUpgradeKits[0].directory = std::move( unpackDir );

#if defined _DEBUG
    _tarOutput.clear( );
    _tarError.clear( );
#endif // defined _DEBUG

    _processRunner = new ProcessRunner( this );

    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &UpgradeManager::tar_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &UpgradeManager::tar_failed                  );
#if defined _DEBUG
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &UpgradeManager::tar_readyReadStandardOutput );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardError,  this, &UpgradeManager::tar_readyReadStandardError  );
#endif // defined _DEBUG

    _processRunner->start(
        { "tar" },
        {
            "-C",  unpackPath,
            "-xf", kitFilePath,
        }
    );
}

void UpgradeManager::tar_succeeded( ) {
    _processRunner->deleteLater( );
    _processRunner = nullptr;

    debug( "+ UpgradeManager::tar_succeeded\n" );

    _goodUpgradeKits.append( _goodSigUpgradeKits.front( ) );

    _goodSigUpgradeKits.removeFirst( );
    _unpackNextKit( );
}

void UpgradeManager::tar_failed( int const exitCode, QProcess::ProcessError const error ) {
    _processRunner->deleteLater( );
    _processRunner = nullptr;

    debug( "+ UpgradeManager::tar_failed: unpacking upgrade kit: exit status %d\n", exitCode );

#if defined _DEBUG
    if ( !_tarOutput.isEmpty( ) ) {
        debug( "  + tar's stdout:\n%s", _tarOutput.toUtf8( ).data( ) );
    }
    if ( !_tarError.isEmpty( ) ) {
        debug( "  + tar's stderr:\n%s", _tarError.toUtf8( ).data( ) );
    }
#endif // defined _DEBUG

    _goodSigUpgradeKits.removeFirst( );
    _unpackNextKit( );
}

#if defined _DEBUG
void UpgradeManager::tar_readyReadStandardOutput( QString const& data ) {
    _tarOutput.append( data );
}

void UpgradeManager::tar_readyReadStandardError( QString const& data ) {
    _tarError.append( data );
}
#endif // defined _DEBUG

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

    QList<int> badKitIndices;
    for ( int kitIndex = 0; kitIndex < _goodUpgradeKits.count( ); ++kitIndex ) {
        auto& update = _goodUpgradeKits[kitIndex];
        debug( "  + examining upgrade kit %s\n", update.kitFileInfo.fileName( ).toUtf8( ).data( ) );
        auto versionInfo = ReadWholeFile( update.directory.absolutePath( ) + Slash + QString( "version.inf" ) ).replace( EndsWithWhitespaceRegex, "" ).split( NewLineRegex );

        QMap<QString, QString> fields;
        QString currentKey;
        int index = 1;
        while ( index < versionInfo.count( ) ) {
            auto& line = versionInfo[index];
            line.replace( EndsWithWhitespaceRegex, "" );
            debug( "    + index: %d {%s}\n", index, line.toUtf8( ).data( ) );

            if ( auto result = MetadataFieldMatcherRegex.match( line ); result.hasMatch( ) ) {
                currentKey = result.captured( 1 );
                line.remove( result.capturedStart( 0 ), result.capturedLength( 0 ) );
                if ( fields.contains( currentKey ) ) {
                    debug( "    + metadata for this kit contains duplicate key '%s'\n", currentKey.toUtf8( ).data( ) );
                    badKitIndices.append( kitIndex );
                    continue;
                }
                fields.insert( currentKey, line );
            } else if ( ( line.length( ) > 1 ) && ( line[0] == Space ) ) {
                if ( currentKey.isEmpty( ) ) {
                    debug( "    + continuation line in metadata file without anything to continue\n" );
                    badKitIndices.append( kitIndex );
                    continue;
                }

                line.remove( 0, 1 );
                auto& currentField = fields[currentKey];
                if ( !currentField.isEmpty( ) ) {
                    currentField.append( LineFeed );
                }
                if ( line != "." ) {
                    currentField.append( line );
                }
            } else {
                debug( "    + unintelligible gibberish in metadata file\n" );
                badKitIndices.append( kitIndex );
                continue;
            }

            ++index;
        }

        bool missingMetadata = false;
        for ( auto const& field : { "Version", "Release-Date", "Description", "Build-Type" } ) {
            if ( !fields.contains( field ) ) {
                debug( "    + metadata is missing field %s\n", field );
                missingMetadata = true;
            }
        }
        if ( missingMetadata ) {
            badKitIndices.append( kitIndex );
            continue;
        }

        auto buildType = fields["Build-Type"].toLower( );
        if ( !StringToBuildType.contains( buildType ) ) {
            debug( "    + unknown build type \"%s\"\n", fields["Build-Type"].toUtf8( ).data( ) );
            badKitIndices.append( kitIndex );
            continue;
        }

        update.version     = fields["Version"];
        update.description = fields["Description"];
        update.buildType   = StringToBuildType[buildType];

        auto releaseDateParts = fields["Release-Date"].split( "-" );
        if ( releaseDateParts.count( ) != 3 ) {
            debug( "    + bad release date format \"%s\"\n", fields["Release-Date"].toUtf8( ).data( ) );
            badKitIndices.append( kitIndex );
            continue;
        }
        auto year  = releaseDateParts[0].toInt( );
        auto month = releaseDateParts[1].toInt( );
        auto day   = releaseDateParts[2].toInt( );
        update.releaseDate = QDate( year, month, day );
        if ( !update.releaseDate.isValid( ) ) {
            debug( "    + bad release date \"%s\"\n", fields["Release-Date"].toUtf8( ).data( ) );
            badKitIndices.append( kitIndex );
            continue;
        }

        auto versionParts = fields["Version"].split( "." );
        if ( versionParts.count( ) != 3 ) {
            debug( "    + bad software version \"%s\"\n", fields["Version"].toUtf8( ).data( ) );
            badKitIndices.append( kitIndex );
            continue;
        }
        update.versionCode = ( versionParts[0].toUInt( ) << 24u ) | ( versionParts[1].toUInt( ) << 16u ) | ( versionParts[2].toUInt( ) << 8u );
    }

    auto end = badKitIndices.rend( );
    for ( auto iter = badKitIndices.rbegin( ); iter != end; ++iter ) {
        debug( "  + removing bad kit %d\n", *iter );
        _goodUpgradeKits.removeAt( *iter );
    }
}
