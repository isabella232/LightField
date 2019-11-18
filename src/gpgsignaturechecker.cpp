#include "pch.h"

#include "gpgsignaturechecker.h"

#include "processrunner.h"

namespace {

    QRegularExpression const SingleWhitespaceCharacterRegex { "\\s"                                                         };

    QString            const ExpectedSignerAddress          { "lightfield-packager@volumetricbio.com"                       };
    QString            const ExpectedSignerName             { "LightField packager <lightfield-packager@volumetricbio.com>" };

    //
    // NOTE:
    //
    // These two arrays, ExpectedKeyIds and ExpectedFingerprints, must
    // contain information about the same keys, in the same order!
    //

    QStringList const ExpectedKeyIds {
        { "0EF6486549978C0C76B49E99C9FC781B66B69981" }, // ID: >>0EF6486549978C0C76B49E99C9FC781B66B69981<<; Fingerprint: C9FC781B66B69981;     Keygrip: BD5A428AF2092982D2D5F1989387806AD1EA50E9; Expires: 2020-01-01
        { "78DAD29978EB392992D7FE0423025033D9E840F7" }, // ID: >>78DAD29978EB392992D7FE0423025033D9E840F7<<; Fingerprint: 23025033D9E840F7;     Keygrip: 7346B3D2FA18F00CDCE091E133EED39544541E65; Expires: 2022-01-01
    };

    QStringList const ExpectedFingerprints {
        { "C9FC781B66B69981"                         }, // ID:   0EF6486549978C0C76B49E99C9FC781B66B69981;   Fingerprint: >>C9FC781B66B69981<<; Keygrip: BD5A428AF2092982D2D5F1989387806AD1EA50E9; Expires: 2020-01-01
        { "23025033D9E840F7"                         }, // ID:   78DAD29978EB392992D7FE0423025033D9E840F7;   Fingerprint: >>23025033D9E840F7<<; Keygrip: 7346B3D2FA18F00CDCE091E133EED39544541E65; Expires: 2022-01-01
    };

}

GpgSignatureChecker::GpgSignatureChecker( QObject* parent ): QObject( parent ) {
    /*empty*/
}

GpgSignatureChecker::~GpgSignatureChecker( ) {
    if ( _processRunner ) {
        _processRunner->deleteLater( );
        _processRunner = nullptr;
    }
}

void GpgSignatureChecker::startCheckDetachedSignature( QString const& dataFileName, QString const& signatureFileName ) {
    _dataFileName      = dataFileName;
    _signatureFileName = signatureFileName;

    _stdout.clear( );

    _processRunner = new ProcessRunner( this );
    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &GpgSignatureChecker::gpg_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &GpgSignatureChecker::gpg_failed                  );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &GpgSignatureChecker::gpg_readyReadStandardOutput );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardError,  this, &GpgSignatureChecker::gpg_readyReadStandardError  );

    _processRunner->start(
        { "gpg" },
        {
            "--status-fd", "1",
            "--verify",
            _signatureFileName,
            _dataFileName
        }
    );
}

void GpgSignatureChecker::gpg_succeeded( ) {
    debug( "+ GpgSignatureChecker::gpg_succeeded: examining GPG output for file '%s'\n", _dataFileName.toUtf8( ).data( ) );

    auto lines = _stdout.replace( EndsWithWhitespaceRegex, "" ).split( NewLineRegex );
    debug( "  + Output from GPG:\n" );
    for ( int limit = lines.count( ), index = 0; index < limit; ++index ) {
        debug( "    + line %d: %s\n", index + 1, lines[index].toUtf8( ).data( ) );
    }

    auto keyIdIndex       = -1;
    auto fingerprintIndex = -1;

    auto lineIndex = 0;
    for ( auto line : lines ) {
        auto fields = line.split( SingleWhitespaceCharacterRegex );
        auto& token = fields[1];
        ++lineIndex;

        debug( "  + line %d: token '%s'\n", lineIndex, token.toUtf8( ).data( ) );

        if ( token == "NEWSIG" ) {
            if ( ( fields.count( ) > 2 ) && ( fields[2] != ExpectedSignerAddress ) ) {
                debug( "  + 'NEWSIG': invalid signer address: '%s'\n", fields[2].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }
        } else if ( token == "SIG_ID" ) {
            if ( fields.count( ) >= 5 ) {
                debug( "  + 'SIG_ID': signature ID '%s', date '%s', timestamp '%s'\n", fields[2].toUtf8( ).data( ), fields[3].toUtf8( ).data( ), fields[4].toUtf8( ).data( ) );
            }
        } else if ( token == "GOODSIG" ) {
            if ( fields.count( ) < 3 ) {
                debug( "  + 'GOODSIG': invalid input\n" );
                emit signatureCheckComplete( false );
                return;
            }

            if ( auto index = ExpectedFingerprints.indexOf( fields[2] ); -1 == index ) {
                debug( "  + 'GOODSIG': invalid key fingerprint: '%s'\n", fields[2].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            } else {
                if ( -1 == fingerprintIndex ) {
                    debug( "  + 'GOODSIG': setting fingerprint index to %d\n", index );
                    fingerprintIndex = index;
                } else if ( fingerprintIndex != index ) {
                    debug( "  + 'GOODSIG': existing fingerprint index %d doesn't match new fingerprint index %d\n", fingerprintIndex, index );
                    emit signatureCheckComplete( false );
                    return;
                }
            }

            fields.removeFirst( );
            fields.removeFirst( );
            fields.removeFirst( );
            auto signerName = fields.join( Space );
            if ( signerName != ExpectedSignerName ) {
                debug( "  + 'GOODSIG': invalid signer name: got '%s'\n", signerName.toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }
        } else if ( token == "VALIDSIG" ) {
            if ( fields.count( ) < 12 ) {
                debug( "  + 'VALIDSIG': invalid input\n" );
                emit signatureCheckComplete( false );
                return;
            }

            if ( auto index = ExpectedKeyIds.indexOf( fields[2] ); -1 == index ) {
                debug( "  + 'VALIDSIG': unknown key ID: '%s'\n", fields[2].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            } else {
                if ( -1 == keyIdIndex ) {
                    debug( "  + 'VALIDSIG': setting key index to %d\n", index );
                    keyIdIndex = index;
                } else if ( keyIdIndex != index ) {
                    debug( "  + 'VALIDSIG': existing key index %d doesn't match new key index %d\n", keyIdIndex, index );
                    emit signatureCheckComplete( false );
                    return;
                }
            }
            if ( fields[2] != fields[11] ) {
                debug( "  + 'VALIDSIG': first and second key IDs don't match: '%s' vs '%'\n", fields[2].toUtf8( ).data( ), fields[11].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }
        }
    }
    debug( "  + key index: %d; fingerprint index: %d\n", keyIdIndex, fingerprintIndex );
    if ( ( -1 == keyIdIndex ) || ( -1 == fingerprintIndex ) || ( keyIdIndex != fingerprintIndex ) ) {
        debug( "    + key index or fingerprint index is bad\n" );
        emit signatureCheckComplete( false );
        return;
    }

    debug( "  + signature is good\n" );
    emit signatureCheckComplete( true );
}

void GpgSignatureChecker::gpg_failed( int const exitCode, QProcess::ProcessError const error ) {
    debug(
        "+ GpgSignatureChecker::gpg_failed: checking signature of upgrade kit:\n"
        "  + ExitCode:     %d\n"
        "  + ProcessError: %s\n"
        "",
        exitCode,
        ToString( error )
    );
    if ( !_stdout.isEmpty( ) ) {
        debug( "  + stdout:\n%s", _stdout.toUtf8( ).data( ) );
    }
    if ( !_stderr.isEmpty( ) ) {
        debug( "  + stderr:\n%s", _stderr.toUtf8( ).data( ) );
    }
    emit signatureCheckComplete( false );
}

void GpgSignatureChecker::gpg_readyReadStandardOutput( QString const& data ) {
    _stdout.append( data );
}

void GpgSignatureChecker::gpg_readyReadStandardError( QString const& data ) {
    _stderr.append( data );
}

int GpgSignatureChecker::instanceId( ) const {
    return _processRunner ? _processRunner->instanceId( ) : -1;
}

QProcess::ProcessState GpgSignatureChecker::state( ) const {
    return _processRunner ? _processRunner->state( ) : QProcess::ProcessState::NotRunning;
}
