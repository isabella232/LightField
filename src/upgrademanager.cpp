#include "pch.h"

#include "upgrademanager.h"

#include "gpgsignaturechecker.h"
#include "hasher.h"
#include "processrunner.h"
#include "strings.h"
#include "upgradekitunpacker.h"
#include "utils.h"
#include "version.h"

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

    QRegularExpression const MetadataFieldMatcherRegex { "^([-0-9A-Za-z]+):\\s*" };
    QRegularExpression const WhitespaceRegex           { "\\s+"                  };

    QStringList ExpectedMetadataFields {
        "Metadata-Version",
        "Version",
        "Build-Type",
        "Release-Date",
        "Description",
        "Checksums-SHA256"
    };

}

UpgradeManager::UpgradeManager( QObject* parent ): QObject( parent ) {
    /*empty*/
}

UpgradeManager::~UpgradeManager( ) {
    /*empty*/
}

void UpgradeManager::_dumpBufferContents( ) {
    if ( !_stdoutBuffer.isEmpty( ) ) {
        debug( "  + stdout:\n[apt-get] %s\n", _stdoutBuffer.replace( NewLineRegex, "\n[apt-get] " ).toUtf8( ).data( ) );
    }
    if ( !_stderrBuffer.isEmpty( ) ) {
        debug( "  + stderr:\n[apt-get] %s\n", _stderrBuffer.replace( NewLineRegex, "\n[apt-get] " ).toUtf8( ).data( ) );
    }
}

void UpgradeManager::_checkForUpgrades( QString const& upgradesPath ) {
    debug( "+ UpgradeManager::_checkForUpgrades: looking for unpacked upgrade kits in path %s\n", UpdatesRootPath.toUtf8( ).data( ) );
    for ( auto kitDirInfo : QDir { UpdatesRootPath }.entryInfoList( UpgradeKitDirGlobs, QDir::Dirs | QDir::Readable | QDir::Executable, QDir::Name ) ) {
        QString kitDirName { kitDirInfo.absoluteFilePath( ) };
        debug( "+ UpgradeManager::_checkForUpgrades: found unpacked kit %s\n", kitDirName.toUtf8( ).data( ) );

        if ( !( QFileInfo { kitDirName + QString { "/version.inf" } } ).exists( ) ) {
            debug( "  + bad kit: version.inf file is missing\n" );
            continue;
        }
        if ( !( QFileInfo { kitDirName + QString { "/version.inf.sig" } } ).exists( ) ) {
            debug( "  + bad kit: version.inf.sig file is missing\n" );
            continue;
        }

        debug( "  + found unpacked update kit\n" );
        _unprocessedUpgradeKits.append( UpgradeKitInfo { QDir { kitDirName } } );
    }

    if ( !upgradesPath.isEmpty( ) ) {
        debug( "+ UpgradeManager::_checkForUpgrades: looking for upgrade kits in path %s\n", upgradesPath.toUtf8( ).data( ) );
        for ( auto kitFile : QDir { upgradesPath }.entryInfoList( UpgradeKitFileGlobs, QDir::Files | QDir::Readable, QDir::Name ) ) {
            debug( "+ UpgradeManager::_checkForUpgrades: found kit file %s\n", kitFile.absoluteFilePath( ).toUtf8( ).data( ) );

            QFileInfo sigFile { kitFile.absoluteFilePath( ) + QString { ".sig" } };
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
            _unprocessedUpgradeKits.append( UpgradeKitInfo { kitFile, sigFile } );
        }
    }

    if ( _unprocessedUpgradeKits.isEmpty( ) ) {
        debug( "+ UpgradeManager::_checkForUpgrades: no upgrade kits found\n" );
        _isChecking.clear( );
        emit upgradeCheckComplete( false );
        return;
    }

    debug(
        "+ UpgradeManager::_checkForUpgrades: found %d upgrade kits\n"
        "==================================================\n"
        "",
        _unprocessedUpgradeKits.count( )
    );
    _checkNextKitSignature( );
}

void UpgradeManager::_checkNextKitSignature( ) {
    debug( "+ UpgradeManager::_checkNextKitSignature\n" );

top:
    if ( _unprocessedUpgradeKits.isEmpty( ) ) {
        debug( "  + finished checking kit signatures\n" );

        _unprocessedUpgradeKits = std::move( _processedUpgradeKits );
        if ( _unprocessedUpgradeKits.isEmpty( ) ) {
            debug( "  + unprocessed upgrade kits list is empty; emitting upgradeCheckComplete(false).\n" );
            _isChecking.clear( );
            emit upgradeCheckComplete( false );
        } else {
            debug(
                "  + starting unpacking %d kits\n"
                "==================================================\n"
                "",
                _unprocessedUpgradeKits.count( )
            );
            _unpackNextKit( );
        }
        return;
    }

    auto& currentUpgradeKit = _unprocessedUpgradeKits.front( );

    if ( currentUpgradeKit.isAlreadyUnpacked ) {
        debug( "  + skipping unpacked upgrade kit %s\n", currentUpgradeKit.directory.absolutePath( ).toUtf8( ).data( ) );
        _processedUpgradeKits.append( currentUpgradeKit );
        _unprocessedUpgradeKits.removeFirst( );
        goto top;
    }

    auto kitFilePath = currentUpgradeKit.kitFileInfo.absoluteFilePath( );
    auto sigFilePath = currentUpgradeKit.sigFileInfo.absoluteFilePath( );
    debug(
        "  + checking signature for upgrade kit\n"
        "    + upgrade kit file name: %s\n"
        "    + signature file name:   %s\n"
        "",
        kitFilePath.toUtf8( ).data( ),
        sigFilePath.toUtf8( ).data( )
    );

    _gpgSignatureChecker = new GpgSignatureChecker( this );
    QObject::connect( _gpgSignatureChecker, &GpgSignatureChecker::signatureCheckComplete, this, &UpgradeManager::gpgSignatureChecker_kit_complete );
    _gpgSignatureChecker->startCheckDetachedSignature( kitFilePath, sigFilePath );
}

void UpgradeManager::gpgSignatureChecker_kit_complete( bool const result ) {
    _gpgSignatureChecker->deleteLater( );
    _gpgSignatureChecker = nullptr;

    debug( "+ UpgradeManager::gpgSignatureChecker_kit_complete: signature is %s\n", result ? "good" : "bad" );
    if ( result ) {
        _processedUpgradeKits.append( _unprocessedUpgradeKits.front( ) );
    }

    _unprocessedUpgradeKits.removeFirst( );
    _checkNextKitSignature( );
}

void UpgradeManager::_unpackNextKit( ) {
    debug( "+ UpgradeManager::_unpackNextKit\n" );

top:
    if ( _unprocessedUpgradeKits.isEmpty( ) ) {
        debug( "  + finished unpacking kits\n", _processedUpgradeKits.count( ) );

        _unprocessedUpgradeKits = std::move( _processedUpgradeKits );
        if ( _unprocessedUpgradeKits.isEmpty( ) ) {
            debug( "  + unprocessed upgrade kits list is empty; emitting upgradeCheckComplete(false).\n" );
            _isChecking.clear( );
            emit upgradeCheckComplete( false );
        } else {
            debug(
                "  + starting checking version.inf signatures for %d kits\n"
                "==================================================\n"
                "",
                _unprocessedUpgradeKits.count( )
            );
            _checkNextVersionInfSignature( );
        }
        return;
    }

    auto& currentUpgradeKit = _unprocessedUpgradeKits.front( );

    if ( currentUpgradeKit.isAlreadyUnpacked ) {
        debug( "  + skipping unpacked upgrade kit %s\n", currentUpgradeKit.directory.absolutePath( ).toUtf8( ).data( ) );
        _processedUpgradeKits.append( currentUpgradeKit );
        _unprocessedUpgradeKits.removeFirst( );
        goto top;
    }

    QString kitFilePath { currentUpgradeKit.kitFileInfo.absoluteFilePath( ) };
    debug( "  + kit file name: %s\n", kitFilePath.toUtf8( ).data( ) );

    QString dirName { UpdatesRootPath + Slash + currentUpgradeKit.kitFileInfo.fileName( ) };
    if ( dirName.endsWith( ".kit" ) ) {
        dirName = dirName.left( dirName.length( ) - 4 );
    }

    debug( "  + Unpacking kit '%s' into directory '%s'\n", kitFilePath.toUtf8( ).data( ), dirName.toUtf8( ).data( ) );
    currentUpgradeKit.directory = QDir { dirName };
    if ( currentUpgradeKit.directory.exists( ) ) {
        debug( "  + Directory already exists, deleting\n" );
        currentUpgradeKit.directory.removeRecursively( );

        auto const pred = [ &dirName ] ( UpgradeKitInfo const& kitInfo ) {
            return kitInfo.isAlreadyUnpacked && ( kitInfo.directory.absolutePath( ) == dirName );
        };

        if ( auto iter = std::find_if( _unprocessedUpgradeKits.begin( ), _unprocessedUpgradeKits.end( ), pred ); iter != _unprocessedUpgradeKits.end( ) ) {
            debug( "  + Forgetting about unprocessed upgrade kit already unpacked into same directory\n" );
            _unprocessedUpgradeKits.erase( iter );
        }
        if ( auto iter = std::find_if( _processedUpgradeKits.begin( ), _processedUpgradeKits.end( ), pred ); iter != _processedUpgradeKits.end( ) ) {
            debug( "  + Forgetting about processed upgrade kit already unpacked into same directory\n" );
            _processedUpgradeKits.erase( iter );
        }
    }

    currentUpgradeKit.directory.mkdir( dirName );

    _upgradeKitUnpacker = new UpgradeKitUnpacker( this );
    QObject::connect( _upgradeKitUnpacker, &UpgradeKitUnpacker::unpackComplete, this, &UpgradeManager::upgradeKitUnpacker_complete );
    _upgradeKitUnpacker->startUnpacking( kitFilePath, dirName );
}

#if defined _DEBUG
void UpgradeManager::upgradeKitUnpacker_complete( bool const result, QString const& tarOutput, QString const& tarError ) {
#else
void UpgradeManager::upgradeKitUnpacker_complete( bool const result ) {
#endif // defined _DEBUG
    _upgradeKitUnpacker->deleteLater( );
    _upgradeKitUnpacker = nullptr;

    debug( "+ UpgradeManager::upgradeKitUnpacker_complete: result is %s\n", ToString( result ) );

    if ( result ) {
        _unprocessedUpgradeKits[0].isAlreadyUnpacked = true;
        _processedUpgradeKits.append( _unprocessedUpgradeKits.front( ) );
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

    _unprocessedUpgradeKits.removeFirst( );
    _unpackNextKit( );
}

void UpgradeManager::_checkNextVersionInfSignature( ) {
    debug( "+ UpgradeManager::_checkNextVersionInfSignature\n" );

    if ( _unprocessedUpgradeKits.isEmpty( ) ) {
        debug( "  + finished checking version.inf signatures, moving on to examining %d kits\n", _processedUpgradeKits.count( ) );

        _unprocessedUpgradeKits = std::move( _processedUpgradeKits );
        if ( _unprocessedUpgradeKits.isEmpty( ) ) {
            _isChecking.clear( );
            emit upgradeCheckComplete( false );
        } else {
            _examineUnpackedKits( );
        }
        return;
    }

    auto versionInfFilePath = _unprocessedUpgradeKits[0].directory.absoluteFilePath( "version.inf" );
    debug( "  + checking signature for version.inf file %s\n", versionInfFilePath.toUtf8( ).data( ) );

    _gpgSignatureChecker = new GpgSignatureChecker( this );
    QObject::connect( _gpgSignatureChecker, &GpgSignatureChecker::signatureCheckComplete, this, &UpgradeManager::gpgSignatureChecker_versionInf_complete );
    _gpgSignatureChecker->startCheckDetachedSignature( versionInfFilePath, versionInfFilePath + QString { ".sig" } );
}

void UpgradeManager::gpgSignatureChecker_versionInf_complete( bool const result ) {
    _gpgSignatureChecker->deleteLater( );
    _gpgSignatureChecker = nullptr;

    debug( "+ UpgradeManager::gpgSignatureChecker_versionInf_complete: signature is %s\n", result ? "good" : "bad" );
    if ( result ) {
        _processedUpgradeKits.append( _unprocessedUpgradeKits.front( ) );
    }

    _unprocessedUpgradeKits.removeFirst( );
    _checkNextVersionInfSignature( );
}

bool UpgradeManager::_parseVersionInfo( QString const& versionInfoFileName, UpgradeKitInfo& update ) {
    debug( "+ UpgradeManager::_parseVersionInfo: file name is \"%s\"\n", versionInfoFileName.toUtf8( ).data( ) );
    auto versionInfo = ReadWholeFile( versionInfoFileName ).replace( EndsWithWhitespaceRegex, "" ).split( NewLineRegex );

    QMap<QString, QString> fields;
    QString currentKey;
    int index = 0;

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
    for ( auto const& field : ExpectedMetadataFields ) {
        if ( !fields.contains( field ) ) {
            debug( "  + metadata is missing field %s\n", field.toUtf8( ).data( ) );
            missingMetadata = true;
        }
    }
    if ( missingMetadata ) {
        return false;
    }

    {
        update.metadataVersionString = fields["Metadata-Version"];

        bool ok = false;
        update.metadataVersion = update.metadataVersionString.toInt( &ok );
        if ( !ok ) {
            debug( "  + bad metadata version \"%s\"\n", update.metadataVersionString.toUtf8( ).data( ) );
            return false;
        }

        if ( update.metadataVersion != 1 ) {
            debug( "  + unknown metadata version \"%s\"\n", update.metadataVersionString.toUtf8( ).data( ) );
            return false;
        }
    }

    {
        update.versionString = fields["Version"];

        auto versionParts = update.versionString.split( "." );
        if ( versionParts.count( ) != 3 ) {
            debug( "  + bad software version \"%s\" (1)\n", update.versionString.toUtf8( ).data( ) );
            return false;
        }

        bool ok0 = false, ok1 = false, ok2 = false;
        update.version = MakeVersionCode( versionParts[0].toUInt( &ok0 ), versionParts[1].toUInt( &ok1 ), versionParts[2].toUInt( &ok2 ) );
        if ( !ok0 || !ok1 || !ok2 ) {
            debug( "  + bad software version \"%s\" (2)\n", update.versionString.toUtf8( ).data( ) );
            return false;
        }
    }

    {
        auto buildType = fields["Build-Type"];
        auto buildTypeValue = buildType.toLower( );
        if ( !StringToBuildType.contains( buildTypeValue ) ) {
            debug( "  + unknown build type \"%s\"\n", buildType.toUtf8( ).data( ) );
            return false;
        }
        update.buildType = StringToBuildType[buildTypeValue];
    }

    {
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
    }

    update.description = fields["Description"];

    update.checksums.clear( );
    auto path = update.directory.absolutePath( ) + Slash;
    for ( auto item : fields["Checksums-SHA256"].replace( EndsWithWhitespaceRegex, "" ).split( NewLineRegex ) ) {
        auto pieces = item.split( WhitespaceRegex, QString::SkipEmptyParts );
        if ( ( pieces.count( ) != 2 ) || ( pieces[0].length( ) != 64 ) ) {
            debug( "  + bad checksum entry \"%s\"\n", item.toUtf8( ).data( ) );
            return false;
        }

        update.checksums[path + pieces[1]] = pieces[0];
        debug( "    + file '%s': checksum %s\n", ( path + pieces[1] ).toUtf8( ).data( ), pieces[0].toUtf8( ).data( ) );
    }

    return true;
}

void UpgradeManager::_examineUnpackedKits( ) {
    debug( "+ UpgradeManager::_examineUnpackedKits: %d kits to examine\n", _unprocessedUpgradeKits.count( ) );

    for ( auto& update : _unprocessedUpgradeKits ) {
        if ( _parseVersionInfo( update.directory.absoluteFilePath( "version.inf" ), update ) ) {
            if ( update.version > LIGHTFIELD_VERSION_CODE ) {
                debug( "  + keeping newer version %s, build type %s\n", update.versionString.toUtf8( ).data( ), ToString( update.buildType ) );
                _processedUpgradeKits.append( update );
            } else {
                debug( "  + discarding older version %s, build type %s\n", update.versionString.toUtf8( ).data( ), ToString( update.buildType ) );
            }
        } else {
            debug( "+ UpgradeManager::_examineUnpackedKits: bad metadata in unpacked kit %s\n", update.directory.absolutePath( ).toUtf8( ).data( ) );
        }
    }

    _unprocessedUpgradeKits = std::move( _processedUpgradeKits );
    _checkNextKitsHashes( );
}

void UpgradeManager::_checkNextKitsHashes( ) {
    debug( "+ UpgradeManager::_checkNextKitsHashes\n" );

    if ( _unprocessedUpgradeKits.isEmpty( ) ) {
        debug( "  + finished checking checksums, %d good kits\n", _goodUpgradeKits.count( ) );

        _unprocessedUpgradeKits.clear( );
        _processedUpgradeKits.clear( );

        _isChecking.clear( );
        emit upgradeCheckComplete( _goodUpgradeKits.count( ) > 0 );
        return;
    }

    if ( !_hashChecker ) {
        _hashChecker = new Hasher;
        QObject::connect( _hashChecker, &Hasher::hashCheckResult, this, &UpgradeManager::hasher_hashCheckResult, Qt::QueuedConnection );
    }

    _hashChecker->checkHashes( _unprocessedUpgradeKits[0].checksums, QCryptographicHash::Sha256 );
}

void UpgradeManager::hasher_hashCheckResult( bool const result ) {
    debug( "+ UpgradeManager::hasher_hashCheckResult: result is %s\n", ToString( result ) );

    if ( result ) {
        _goodUpgradeKits.append( _unprocessedUpgradeKits[0] );
    }
    _unprocessedUpgradeKits.removeFirst( );
    _checkNextKitsHashes( );
}

void UpgradeManager::checkForUpgrades( QString const& upgradesPath ) {
    if ( _isChecking.test_and_set( ) ) {
        debug( "+ UpgradeManager::checkForUpgrades: check already in progress\n" );
        return;
    }

    _checkForUpgrades( upgradesPath );
}

void UpgradeManager::installUpgradeKit( UpgradeKitInfo const& kit ) {
    QString symlinkPath { UpdatesRootPath % Slash % "selected" };
    QString kitPath     { kit.directory.absolutePath( )        };

    debug(
        "+ UpgradeManager::installUpgradeKit:\n"
        "  + creating symlink from %s to %s\n",
        symlinkPath.toUtf8( ).data( ), kitPath.toUtf8( ).data( )
    );
    unlink( symlinkPath.toUtf8( ).data( ) );
    link( kit.directory.absolutePath( ).toUtf8( ).data( ), symlinkPath.toUtf8( ).data( ) );

    debug( "+ UpgradeManager::installUpgradeKit: running `apt-get update`\n" );
    _stdoutBuffer.clear( );
    _stderrBuffer.clear( );
    _processRunner = new ProcessRunner { this };
    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &UpgradeManager::aptGetUpdate_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &UpgradeManager::aptGetUpdate_failed                  );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &UpgradeManager::aptGetUpdate_readyReadStandardOutput );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardError,  this, &UpgradeManager::aptGetUpdate_readyReadStandardError  );

    _processRunner->start(
        { "sudo" },
        {
            "apt-get",
            "-y",
            "update",
        }
    );
}

void UpgradeManager::aptGetUpdate_succeeded( ) {
    debug( "+ UpgradeManager::aptGetUpdate_succeeded: apt-get update succeeded\n" );
    _dumpBufferContents( );

    debug( "  + running `apt-get dist-upgrade`\n" );
    _stdoutBuffer.clear( );
    _stderrBuffer.clear( );
    _processRunner->deleteLater( );
    _processRunner = new ProcessRunner { this };
    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &UpgradeManager::aptGetDistUpgrade_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &UpgradeManager::aptGetDistUpgrade_failed                  );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &UpgradeManager::aptGetDistUpgrade_readyReadStandardOutput );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardError,  this, &UpgradeManager::aptGetDistUpgrade_readyReadStandardError  );

    _processRunner->start(
        { "sudo" },
        {
            "apt-get",
            "-y",
            "dist-upgrade",
        }
    );
}

void UpgradeManager::aptGetUpdate_failed( int const exitCode, QProcess::ProcessError const error ) {
    _processRunner->deleteLater( );
    _processRunner = nullptr;

    debug( "+ UpgradeManager::aptGetUpdate_failed: apt-get update failed\n" );
    _dumpBufferContents( );

    emit upgradeFailed( );
}

void UpgradeManager::aptGetUpdate_readyReadStandardOutput( QString const& data ) {
    _stdoutBuffer += data;
}

void UpgradeManager::aptGetUpdate_readyReadStandardError( QString const& data ) {
    _stderrBuffer += data;
}

void UpgradeManager::aptGetDistUpgrade_succeeded( ) {
    debug( "+ UpgradeManager::aptGetDistUpgrade_succeeded: software upgrade succeeded\n" );
    _dumpBufferContents( );

    _processRunner->deleteLater( );
    _processRunner = nullptr;

    debug( "  + restarting\n" );
    system( "sudo systemctl reboot" );
}

void UpgradeManager::aptGetDistUpgrade_failed( int const exitCode, QProcess::ProcessError const error ) {
    _processRunner->deleteLater( );
    _processRunner = nullptr;

    debug( "+ UpgradeManager::aptGetDistUpgrade_failed: apt-get dist-upgrade failed\n" );
    _dumpBufferContents( );

    emit upgradeFailed( );
}

void UpgradeManager::aptGetDistUpgrade_readyReadStandardOutput( QString const& data ) {
    _stdoutBuffer += data;
}

void UpgradeManager::aptGetDistUpgrade_readyReadStandardError( QString const& data ) {
    _stderrBuffer += data;
}
