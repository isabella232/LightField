#include "pch.h"

#include <sys/statvfs.h>
#include <execinfo.h>

#include <ctime>

#include "app.h"
#include "window.h"

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

double GetBootTimeClock( ) {
    timespec now;
    clock_gettime( CLOCK_BOOTTIME, &now );
    return now.tv_sec + now.tv_nsec / 1'000'000'000.0;
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

bool YesNoPrompt( QWidget* parent, QString const& title, QString const& text ) {
    QMessageBox messageBox { parent };
    messageBox.setIcon( QMessageBox::Question );
    messageBox.setText( title );
    messageBox.setInformativeText( text );
    messageBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    messageBox.setDefaultButton( QMessageBox::No );
    messageBox.setFont( ModifyFont( messageBox.font( ), 16.0 ) );

    App::mainWindow( )->hide( );
    auto result = ( QMessageBox::Yes == static_cast<QMessageBox::StandardButton>( messageBox.exec( ) ) );
    App::mainWindow( )->show( );

    return result;
}

void RebootPrinter( ) {
    PidFile.remove( );
    system( ResetLumenArduinoPortCommand.toUtf8( ).data( ) );
    system( ( SetProjectorPowerCommand % " 0" ).toUtf8( ).data( ) );
    QProcess::startDetached( "sudo", { "systemctl", "reboot" } );
}

void ShutDownPrinter( ) {
    PidFile.remove( );
    system( ResetLumenArduinoPortCommand.toUtf8( ).data( ) );
    system( ( SetProjectorPowerCommand % " 0" ).toUtf8( ).data( ) );
    QProcess::startDetached( "sudo", { "systemctl", "poweroff" } );
}
