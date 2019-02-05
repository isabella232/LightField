#ifndef __STRINGS_H__
#define __STRINGS_H__

char const* ToString( QProcess::ProcessError value );
char const* ToString( QProcess::ProcessState value );
char const* ToString( QProcess::ExitStatus   value );

#if defined __SHEPHERD_H__
char const* ToString( PendingCommand         value );
#endif // __SHEPHERD_H__

// template:
//char const* ToString( typename value );

#endif // __STRINGS_H__
