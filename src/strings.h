#ifndef __STRINGS_H__
#define __STRINGS_H__

inline QString getFileBaseName( QString const& fileName ) {
    return fileName.mid( fileName.lastIndexOf( QChar( '/' ) ) + 1 );
}

char const* ToString( QProcess::ProcessError value );
char const* ToString( QProcess::ProcessState value );
char const* ToString( QProcess::ExitStatus   value );

#if defined __SHEPHERD_H__
char const* ToString( PendingCommand         value );
#endif // __SHEPHERD_H__

#endif // __STRINGS_H__
