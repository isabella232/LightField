#ifndef __STRINGS_H__
#define __STRINGS_H__

char const* ToString( QProcess::ProcessError value );
char const* ToString( QProcess::ProcessState value );
char const* ToString( QProcess::ExitStatus   value );

// template:
//char const* ToString( typename value );

#endif // __STRINGS_H__
