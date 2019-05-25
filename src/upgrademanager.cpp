#include "pch.h"

#include "upgrademanager.h"

#include "gpgsignaturechecker.h"
#include "strings.h"
#include "upgradekitunpacker.h"
#include "utils.h"
#include "version.h"

// find kit files
// check signatures
// inspect version.inf
// fill in UpgradeKitInfo instances as we go
// announce results

namespace {

    QStringList UpgradeKitFileGlobs {
        "lightfield-debug_*_amd64.kit",
        "lightfield-release_*_amd64.kit"
    };

    QStringList UpgradeKitDirGlobs {
        "lightfield-debug_*_amd64",
        "lightfield-release_*_amd64"
    };

    QMap<QString, BuildType> StringToBuildType {
        { "release", BuildType::Release },
        { "debug",   BuildType::Debug   },
    };

    QRegularExpression const EndsWithWhitespaceRegex   { "\\s+$"                 };
    QRegularExpression const MetadataFieldMatcherRegex { "^([-0-9A-Za-z]+):\\s*" };

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

void UpgradeManager::_checkForUpgrades( QString const& upgradesPath ) {
    debug( "+ UpgradeManager::_checkForUpgrades: looking for upgrade kits in path %s\n", upgradesPath.toUtf8( ).data( ) );

    for ( auto kitDir : QDir { UpdatesRootPath }.entryInfoList( UpgradeKitDirGlobs, QDir::Dirs | QDir::Readable | QDir::Executable, QDir::Name ) ) {
        debug( "+ UpgradeManager::_checkForUpgrades: found unpacked kit %s\n", kitDir.fileName( ).toUtf8( ).data( ) );

        if ( QFileInfo versionInfoFile { kitDir.absoluteFilePath( ).append( "/version.inf" ) }; versionInfoFile.exists( ) ) {
            debug( "  + found unpacked update kit\n" );
            _rawUpgradeKits.append( UpgradeKitInfo { kitDir.dir( ) } );
        }
    }

    for ( auto kitFile : QDir { upgradesPath }.entryInfoList( UpgradeKitFileGlobs, QDir::Files | QDir::Readable, QDir::Name ) ) {
        debug( "+ UpgradeManager::_checkForUpgrades: found kit file %s\n", kitFile.fileName( ).toUtf8( ).data( ) );

        QFileInfo sigFile { kitFile.absoluteFilePath( ).append( ".sig" ) };
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
        _rawUpgradeKits.append( { kitFile, sigFile } );
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

    auto kitFilePath = _rawUpgradeKits[0].kitFileInfo.canonicalFilePath( );
    debug( "  + checking signature for upgrade kit %s\n", kitFilePath.toUtf8( ).data( ) );

    _gpgSignatureChecker = new GpgSignatureChecker( this );
    QObject::connect( _gpgSignatureChecker, &GpgSignatureChecker::signatureCheckComplete, this, &UpgradeManager::gpgSignatureChecker_kit_complete );
    _gpgSignatureChecker->startCheckDetachedSignature( kitFilePath, _rawUpgradeKits[0].sigFileInfo.canonicalFilePath( ) );
}

void UpgradeManager::gpgSignatureChecker_kit_complete( bool const result, QStringList const& /*results*/ ) {
    _gpgSignatureChecker->deleteLater( );
    _gpgSignatureChecker = nullptr;

    debug( "+ UpgradeManager::gpgSignatureChecker_kit_complete: result is %s\n", ToString( result ) );

    debug( "+ UpgradeManager::gpgSignatureChecker_kit_complete: signature is %s\n", result ? "good" : "bad" );
    if ( result ) {
        _goodSigUpgradeKits.append( _rawUpgradeKits.front( ) );
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
    _goodSigUpgradeKits[0].directory = unpackDir;

    _upgradeKitUnpacker = new UpgradeKitUnpacker( this );
    QObject::connect( _upgradeKitUnpacker, &UpgradeKitUnpacker::unpackComplete, this, &UpgradeManager::upgradeKitUnpacker_complete );
    _upgradeKitUnpacker->startUnpacking( kitFilePath, unpackPath );
}

void UpgradeManager::upgradeKitUnpacker_complete( bool const result, QString const& tarOutput, QString const& tarError ) {
    _upgradeKitUnpacker->deleteLater( );
    _upgradeKitUnpacker = nullptr;

    debug( "+ UpgradeManager::upgradeKitUnpacker_complete: result is %s\n", ToString( result ) );

    if ( result ) {
        _goodUpgradeKits.append( _goodSigUpgradeKits.front( ) );
#if defined _DEBUG
    } else {
        if ( !tarOutput.isEmpty( ) ) {
            debug( "+ UpgradeManager::upgradeKitUnpacker_complete: tar's stdout:\n%s", tarOutput.toUtf8( ).data( ) );
        }
        if ( !tarError.isEmpty( ) ) {
            debug( "+ UpgradeManager::upgradeKitUnpacker_complete: tar's stderr:\n%s", tarError.toUtf8( ).data( ) );
        }
#endif // defined _DEBUG
    }

    _goodSigUpgradeKits.removeFirst( );
    _unpackNextKit( );
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

bool UpgradeManager::_parseVersionInfo( QString const& versionInfoFileName, UpgradeKitInfo& update ) {
    QMap<QString, QString> fields;
    QString currentKey;
    int index = 1;

    debug( "+ UpgradeManager::_parseVersionInfo: file name is \"%s\"\n", versionInfoFileName.toUtf8( ).data( ) );
    auto versionInfo = ReadWholeFile( versionInfoFileName ).replace( EndsWithWhitespaceRegex, "" ).split( NewLineRegex );

    while ( index < versionInfo.count( ) ) {
        auto line = versionInfo[index];
        line.replace( EndsWithWhitespaceRegex, "" );
        debug( "  + line #%d: \"%s\"\n", index, line.toUtf8( ).data( ) );

        if ( auto result = MetadataFieldMatcherRegex.match( line ); result.hasMatch( ) ) {
            currentKey = result.captured( 1 );
            line.remove( result.capturedStart( 0 ), result.capturedLength( 0 ) );
            if ( fields.contains( currentKey ) ) {
                debug( "    + metadata for this kit contains duplicate key '%s'\n", currentKey.toUtf8( ).data( ) );
                return false;
            }
            fields.insert( currentKey, line );
        } else if ( ( line.length( ) > 1 ) && ( line[0] == Space ) ) {
            if ( currentKey.isEmpty( ) ) {
                debug( "    + continuation line in metadata file without anything to continue\n" );
                return false;
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
            return false;
        }

        ++index;
    }

    bool missingMetadata = false;
    for ( auto const& field : { "Version", "Release-Date", "Description", "Build-Type" } ) {
        if ( !fields.contains( field ) ) {
            debug( "  + metadata is missing field %s\n", field );
            missingMetadata = true;
        }
    }
    if ( missingMetadata ) {
        return false;
    }

    auto buildType = fields["Build-Type"];
    auto buildTypeValue = buildType.toLower( );
    if ( !StringToBuildType.contains( buildTypeValue ) ) {
        debug( "  + unknown build type \"%s\"\n", buildType.toUtf8( ).data( ) );
        return false;
    }
    update.buildType = StringToBuildType[buildTypeValue];

    auto version = fields["Version"];
    update.version = version;

    update.description = fields["Description"];

    auto releaseDate = fields["Release-Date"];
    auto releaseDateParts = releaseDate.split( "-" );
    if ( releaseDateParts.count( ) != 3 ) {
        debug( "  + bad release date format \"%s\"\n", releaseDate.toUtf8( ).data( ) );
        return false;
    }
    auto year  = releaseDateParts[0].toInt( );
    auto month = releaseDateParts[1].toInt( );
    auto day   = releaseDateParts[2].toInt( );
    update.releaseDate = QDate( year, month, day );
    if ( !update.releaseDate.isValid( ) ) {
        debug( "  + bad release date \"%s\"\n", releaseDate.toUtf8( ).data( ) );
        return false;
    }

    auto versionParts = version.split( "." );
    if ( versionParts.count( ) != 3 ) {
        debug( "  + bad software version \"%s\"\n", version.toUtf8( ).data( ) );
        return false;
    }
    update.versionCode = ( versionParts[0].toUInt( ) << 24u ) | ( versionParts[1].toUInt( ) << 16u ) | ( versionParts[2].toUInt( ) << 8u );

    return true;
}

void UpgradeManager::_examineUnpackedKits( ) {
    debug( "+ UpgradeManager::_examineUnpackedKits: %d kits to examine\n", _goodUpgradeKits.count( ) );

    std::vector<int> badKitIndices;
    for ( int kitIndex = 0; kitIndex < _goodUpgradeKits.count( ); ++kitIndex ) {
        auto& update = _goodUpgradeKits[kitIndex];

        if ( !_parseVersionInfo( update.directory.absolutePath( ) + Slash + QString( "version.inf" ), update ) ) {
            debug( "+ UpgradeManager::_examineUnpackedKits: bad metadata in kit #%d (%s)\n", kitIndex, update.kitFileInfo.absoluteFilePath( ) );
            auto end = badKitIndices.end( );
            if ( auto iter = std::lower_bound( badKitIndices.begin( ), end, kitIndex ); iter != end ) {
                if ( *iter != kitIndex ) {
                    badKitIndices.insert( iter, kitIndex );
                }
            }
        }
    }

    auto end = badKitIndices.rend( );
    for ( auto iter = badKitIndices.rbegin( ); iter != end; ++iter ) {
        _goodUpgradeKits.removeAt( *iter );
    }
    debug( "  + %d kits left after removing bad kits\n", _goodUpgradeKits.count( ) );
}
