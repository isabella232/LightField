#ifndef __STRINGS_H__
#define __STRINGS_H__

inline QString getFileBaseName( QString const& fileName ) {
    return fileName.mid( fileName.lastIndexOf( QChar( '/' ) ) + 1 );
}

char const* ToString( QProcess::ProcessError const value );
char const* ToString( QProcess::ProcessState const value );
char const* ToString( QProcess::ExitStatus   const value );

char const* ToString( QDialog::DialogCode    const value );

char const* ToString( bool                   const value );

QString FormatDouble( double const value, int precision = -1 );

#endif // __STRINGS_H__
