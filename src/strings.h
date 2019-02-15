#ifndef __STRINGS_H__
#define __STRINGS_H__

inline QString getFileBaseName( QString const& fileName ) {
    return fileName.mid( fileName.lastIndexOf( QChar( '/' ) ) + 1 );
}

char const* ToString( QProcess::ProcessError value );
char const* ToString( QProcess::ProcessState value );
char const* ToString( QProcess::ExitStatus   value );

char const* ToString( QDialog::DialogCode    value );

QString FormatDouble( double const a, int prec = -1 );

#endif // __STRINGS_H__
