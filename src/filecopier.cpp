#include "pch.h"

#include "filecopier.h"

namespace {

    template<typename T>
    constexpr std::enable_if_t<std::is_integral_v<T>, T> ceiling( T const value, T const significance ) {
        return ( ( value + ( significance - static_cast<T>( 1 ) ) ) / significance ) * significance;
    }

}

FileCopier::FileCopier( QObject* parent ): QObject( parent ) {
    /*empty*/
}

FileCopier::~FileCopier( ) {
    /*empty*/
}

void FileCopier::_copy( ) {
    auto limit = _fileList.length( );
    debug( "+ FileCopier::_copy: copying %d files\n", limit );

    int copiedFiles  { };
    int skippedFiles { };

    for ( int index = 0; index < limit; ++index ) {
        if ( _abortRequested ) {
            debug( "+ FileCopier::_copy: abort requested [1]\n" );
            emit notify( index, "Aborting..." );
            break;
        }

        auto const& item { _fileList[index] };

        debug( "+ FileCopier::_copy: copying '%s' to '%s'\n", item.first.toUtf8( ).data( ), item.second.toUtf8( ).data( ) );

        QFile srcFile { item.first  };
        QFile dstFile { item.second };

        if ( !srcFile.exists( ) ) {
            debug( "+ FileCopier::_copy: source file '%s' does not exist, skipping\n", item.first.toUtf8( ).data( ) );
            ++skippedFiles;
            continue;
        }

        qint64 srcSize = QFileInfo { srcFile }.size( );

        qint64 dstFree, blockSize;
        if ( !GetFileSystemInfoFromPath( dstFile.fileName( ), dstFree, blockSize ) ) {
            debug( "+ FileCopier::_copy: GetFileSystemInfoFromPath failed\n" );
            break;
        }

        debug( "+ FileCopier::_copy: source file size: %lld [%lld on USB stick with block size %lld]; free space in destination: %lld\n", srcSize, ceiling( srcSize, blockSize ), blockSize, dstFree );
        if ( dstFree < ceiling( srcSize, blockSize ) ) {
            debug( "+ FileCopier::_copy: source file is too big, skipping\n" );
            ++skippedFiles;
            emit notify( index, "Skipping file <span style=\"font-weight: bold;\">" % item.first.mid( item.first.lastIndexOf( Slash ) + 1 ) % "</span>: Insufficient free space on USB stick." );
            continue;
        }

        if ( !srcFile.open( QIODevice::ReadOnly | QIODevice::ExistingOnly | QIODevice::Unbuffered ) ) {
            debug( "+ FileCopier::_copy: can't open source file '%s', skipping\n", item.first.toUtf8( ).data( ) );
            ++skippedFiles;
            emit notify( index, "Skipping file <span style=\"font-weight: bold;\">" % item.first.mid( item.first.lastIndexOf( Slash ) + 1 ) % "</span>: Couldn't open source file." );
            continue;
        }

        if ( !dstFile.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered ) ) {
            debug( "+ FileCopier::_copy: can't open destination file '%s', skipping\n", item.second.toUtf8( ).data( ) );
            srcFile.close( );
            ++skippedFiles;
            emit notify( index, "Skipping file <span style=\"font-weight: bold;\">" % item.first.mid( item.first.lastIndexOf( Slash ) + 1 ) % "</span>: Couldn't create destination file." );
            continue;
        }

        debug( "+ FileCopier::_copy: starting copy\n" );
        emit fileStarted( index, srcSize );

        qint64 bytesCopied = 0;
        bool shortRead;
        do {
            if ( _abortRequested ) {
                debug( "+ FileCopier::_copy: abort requested [2]\n" );
                dstFile.remove( );
                ++skippedFiles;
                emit notify( index, "Aborting..." );
                break;
            }

            auto data = srcFile.read( blockSize );
            auto dataLength = data.length( );

            shortRead = dataLength < blockSize;
            if ( shortRead ) {
                debug(
                    "+ FileCopier::_copy: short read, this must be the last block\n"
                    "  + blockSize:   %lld\n"
                    "  + data.length: %lld\n"
                    "  + srcSize:     %lld\n"
                    "  + bytesRead:   %lld\n"
                    "  + shortRead?   %s\n"
                    "",
                    blockSize,
                    dataLength,
                    srcSize,
                    bytesCopied + dataLength,
                    ToString( shortRead )
                );
                srcSize = bytesCopied + dataLength;
            }

            auto bytesWritten = dstFile.write( data );
            if ( bytesWritten < dataLength ) {
                debug( "+ FileCopier::_copy: short write? %lld vs %d\n", bytesWritten, dataLength );
                dstFile.remove( );
                ++skippedFiles;
                emit notify( index, "Skipping file <span style=\"font-weight: bold;\">" % item.first.mid( item.first.lastIndexOf( Slash ) + 1 ) % "</span>: Encountered a short write during copy." );
                break;
            }

            bytesCopied += bytesWritten;
            emit fileProgress( index, bytesCopied );
        } while ( !shortRead );

        debug( "+ FileCopier::_copy: bytesCopied: %lld, srcSize: %lld\n", bytesCopied, srcSize );
        if ( bytesCopied == srcSize ) {
            ++copiedFiles;
        }

        srcFile.close( );
        dstFile.close( );

        emit fileFinished( index, bytesCopied );
    }

    emit finished( copiedFiles, skippedFiles );
}

void FileCopier::copy( FileNamePairList const& fileList ) {
    _fileList = fileList;
    debug( "+ FileCopier::copy: creating thread\n", this, _thread );
    _thread = QThread::create( std::bind( &FileCopier::_copy, this ) );
    debug( "+ FileCopier::copy: moving this[%p] to new thread[%p]\n", this, _thread );
    moveToThread( _thread );
    debug( "+ FileCopier::copy: setting thread[%p]'s parent to this[%p]\n", _thread, this );
    _thread->setParent( this );
    debug( "+ FileCopier::copy: starting thread[%p]\n", _thread );
    _thread->start( );
}

void FileCopier::abort( ) {
    if ( !_thread ) {
        return;
    }

    _abortRequested = true;
    _thread->wait( );
    _thread->deleteLater( );
    _thread = nullptr;
}
