#include "pch.h"

#include "gpgsignaturechecker.h"

#include "processrunner.h"
#include "strings.h"
#include "utils.h"

namespace {

    QStringList GpgExpectedTokens {
        "NEWSIG",
        "KEY_CONSIDERED",
        "SIG_ID",
        "KEY_CONSIDERED",
        "GOODSIG",
        "VALIDSIG",
        "VERIFICATION_COMPLIANCE_MODE",
    };

    QVector<int> GpgExpectedFieldCount {
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
        { "gpgv" },
        {
            "--status-fd", "1",
            "--keyring",   GpgKeyRingPath,
            _signatureFileName,
            _dataFileName
        }
    );
}

void GpgSignatureChecker::gpg_succeeded( ) {
    debug( "+ GpgSignatureChecker::gpg_succeeded: examining GPG output for file '%s'\n", _dataFileName.toUtf8( ).data( ) );

    auto lines = _stdout.replace( EndsWithWhitespaceRegex, "" ).split( NewLineRegex );
    if ( lines.count( ) > GpgExpectedTokens.count( ) ) {
        debug(
            "  + too many lines: expected %d, got %d\n"
            "  >> %s\n"
            "",
            GpgExpectedTokens.count( ), lines.count( ),
            lines.join( "\n" ).toUtf8( ).data( )
        );
        emit signatureCheckComplete( false );
        return;
    }

    auto lineIndex = 0;
    for ( auto line : lines ) {
        auto fields = line.split( SingleWhitespaceCharacterRegex );
        auto& token = fields[1];

        if ( ( GpgExpectedFieldCount[lineIndex] != -1 ) && ( GpgExpectedFieldCount[lineIndex] != fields.count( ) ) ) {
            debug( "  + wrong number of fields: expected %d, got %d\n", GpgExpectedFieldCount[lineIndex], fields.count( ) );
            emit signatureCheckComplete( false );
            return;
        }
        if ( token != GpgExpectedTokens[lineIndex] ) {
            debug( "  + wrong token: expected '%s', got '%s'\n", GpgExpectedTokens[lineIndex].toUtf8( ).data( ), token.toUtf8( ).data( ) );
            emit signatureCheckComplete( false );
            return;
        }
        ++lineIndex;

        if ( token == "NEWSIG" ) {
            if ( fields[2] != ExpectedSignerAddress ) {
                debug( "  + 'NEWSIG': invalid signer address: expected '%s', got '%s'\n", ExpectedSignerAddress.toUtf8( ).data( ), fields[2].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }
        } else if ( token == "KEY_CONSIDERED" ) {
            if ( fields[2] != ExpectedKeyId ) {
                debug( "  + 'KEY_CONSIDERED': invalid key ID: expected '%s', got '%s'\n", ExpectedKeyId.toUtf8( ).data( ), fields[2].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }
        } else if ( token == "SIG_ID" ) {
            debug( "  + 'SIG_ID': signature ID '%s', date '%s', timestamp '%s'\n", fields[2].toUtf8( ).data( ), fields[3].toUtf8( ).data( ), fields[4].toUtf8( ).data( ) );
        } else if ( token == "GOODSIG" ) {
            if ( fields[2] != ExpectedFingerprint ) {
                debug( "  + 'GOODSIG': invalid key fingerprint: expected '%s', got '%s'\n", ExpectedFingerprint.toUtf8( ).data( ), fields[2].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }

            fields.removeFirst( );
            fields.removeFirst( );
            fields.removeFirst( );
            auto signerName = fields.join( Space );

            if ( signerName != ExpectedSignerName ) {
                debug( "  + 'GOODSIG': invalid signer name: expected '%s', got '%s'\n", ExpectedSignerName.toUtf8( ).data( ), signerName.toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }
        } else if ( token == "VALIDSIG" ) {
            if ( ( fields[2] != ExpectedKeyId ) || ( fields[11] != ExpectedKeyId ) ) {
                debug( "  + 'VALIDSIG': invalid key ID: expected '%s', got '%s' and '%s'\n", ExpectedKeyId.toUtf8( ).data( ), fields[2].toUtf8( ).data( ), fields[11].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }
        } else if ( token == "VERIFICATION_COMPLIANCE_MODE" ) {
            if ( fields[2] != "23" ) {
                debug( "  + 'VERIFICATION_COMPLIANCE_MODE': invalid value: expected '23', got '%s'\n", fields[2].toUtf8( ).data( ) );
                emit signatureCheckComplete( false );
                return;
            }
        }
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
