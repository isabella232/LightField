#include "pch.h"

#include "hasher.h"

void Hasher::_hash( QString const fileName, QCryptographicHash::Algorithm const algorithm ) {
    QFile file { fileName };
    if ( file.open( QIODevice::ReadOnly ) ) {
        QCryptographicHash hasher { algorithm };
        hasher.addData( &file );
        file.close( );
        emit resultReady( { hasher.result( ).toHex( ) } );
    } else {
        debug( "+ Hasher::_hash: couldn't open file '%s'\n", fileName.toUtf8( ).data( ) );
        emit resultReady( { } );
    }
}

void Hasher::_checkHashes( QMap<QString, QString> const fileNames, QCryptographicHash::Algorithm const algorithm ) {
    debug( "+ Hasher::_checkHashes\n" );
    for ( auto const& fileName : fileNames.keys( ) ) {
        debug( "+ Hasher::_checkHashes: file %s\n", fileName.toUtf8( ).data( ) );
        QFile file { fileName };
        if ( file.open( QIODevice::ReadOnly ) ) {
            QCryptographicHash hasher { algorithm };
            hasher.addData( &file );
            file.close( );

            QString result { hasher.result( ).toHex( ) };
            if ( result != fileNames[fileName] ) {
                debug( "+ Hasher::_checkHashes: hash mismatch! %s vs %s\n", fileNames[fileName].toUtf8( ).data( ), result.toUtf8( ).data( ) );
                emit hashCheckResult( false );
            }
        } else {
            debug( "+ Hasher::_checkHashes: couldn't open file '%s'\n", fileName.toUtf8( ).data( ) );
            emit hashCheckResult( false );
        }
    }
    emit hashCheckResult( true );
}
