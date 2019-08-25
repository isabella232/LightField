#include "pch.h"

#include <sys/statvfs.h>

#include <execinfo.h>

#include <ctime>

namespace {

    char const* DriveSizeUnits[] = {
        "iB",
        "KiB",
        "MiB",
        "GiB",
        "TiB",
        "PiB",
        "EiB",
        "ZiB",
        "YiB"
    };

}

QHBoxLayout* WrapWidgetsInHBox( std::initializer_list<QWidget*> widgets ) {
    auto layout = new QHBoxLayout;

    for ( auto widget : widgets ) {
        if ( widget ) {
            layout->addWidget( widget );
        } else {
            layout->addStretch( );
        }
    }
    return layout;
}

QVBoxLayout* WrapWidgetsInVBox( std::initializer_list<QWidget*> widgets ) {
    auto layout = new QVBoxLayout;

    for ( auto widget : widgets ) {
        if ( widget ) {
            layout->addWidget( widget );
        } else {
            layout->addStretch( );
        }
    }
    return layout;
}

QPalette ModifyPalette( QPalette const& palette_, QPalette::ColorGroup const group, QPalette::ColorRole const role, QColor const& color ) {
    auto palette { palette_ };
    palette.setColor( group, role, color );
    return palette;
}

QPalette ModifyPalette( QPalette const& palette_, QPalette::ColorRole const role, QColor const& color ) {
    auto palette { palette_ };
    palette.setColor( role, color );
    return palette;
}

QString GetUserName( ) {
    char* buf = new char[16384];
    struct passwd pwd;
    struct passwd* result;
    QString userName;

    if ( 0 == ::getpwuid_r( ::getuid( ), &pwd, buf, 16384, &result ) ) {
        userName = pwd.pw_name;
    } else {
        debug( "+ GetUserName: getpwuid_r failed?!\n" );
    }

    delete[] buf;
    return userName;
}

qreal Distance( QPointF const& a, QPointF const& b ) {
    return sqrt( pow( a.x( ) - b.x( ), 2.0 ) + pow( a.y( ) - b.y( ), 2.0 ) );
}

int Distance( QPoint const& a, QPoint const& b ) {
    return sqrt( pow( a.x( ) - b.x( ), 2.0 ) + pow( a.y( ) - b.y( ), 2.0 ) );
}

double GetBootTimeClock( ) {
    timespec now;
    clock_gettime( CLOCK_BOOTTIME, &now );
    return now.tv_sec + now.tv_nsec / 1'000'000'000.0;
}

QString ReadWholeFile( QString const& fileName ) {
    QFile file { fileName };
    QString result;

    if ( file.open( QFile::ReadOnly | QFile::ExistingOnly ) ) {
        auto fileContents = file.readAll( );
        file.close( );
        result = fileContents;
    }
    return result;
}

bool GetFileSystemInfoFromPath( QString const& fileName, qint64& bytesFree, qint64& optimalWriteBlockSize ) {
    QString filePath = QFileInfo { fileName }.canonicalPath( );
    struct statvfs buf;
    if ( -1 == statvfs( filePath.toUtf8( ).data( ), &buf ) ) {
        debug( "+ GetFreeSpaceFromPath: path '%s': statvfs failed: %s [%d]\n", filePath.toUtf8( ).data( ), strerror( errno ), errno );
        return false;
    }

    bytesFree             = buf.f_bsize * buf.f_bavail;
    optimalWriteBlockSize = buf.f_bsize;
    return true;
}

void ScaleSize( qint64 const inputSize, double& scaledSize, char const*& suffix ) {
    int unitIndex = 0;

    scaledSize = inputSize;
    while ( scaledSize > 1024.0 ) {
        ++unitIndex;
        scaledSize /= 1024.0;
    }
    suffix = DriveSizeUnits[unitIndex];
}

void PrintBacktrace( char const* tracerName ) {
    void* frames[256];
    auto size    = backtrace( frames, std::size( frames ) );
    auto strings = backtrace_symbols( frames, size );

    debug( "+ %s: %zu frames:\n", tracerName, size - 1 );
    for ( decltype( size ) i = 1; i < size; i++ ) {
        debug( "  + %s\n", strings[i] );
    }

    free( strings );
}
