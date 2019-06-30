#ifndef __WIN32JUNK_H__
#define __WIN32JUNK_H__

#include <io.h>

using error_t = int;
using gid_t   = int;
using pid_t   = int;
using uid_t   = int;

int const R_OK = 4;
int const W_OK = 2;
int const X_OK = 1;
int const F_OK = 0;

int const CLOCK_BOOTTIME = 7;

#define SIGHUP   1
#define SIGQUIT  3
#define SIGUSR1 10
#define SIGUSR2 11

union sigval_t {
    int   sival_int;
    void* sival_ptr;
};

struct siginfo_t {
    int            si_signo;
    pid_t          si_pid;
    uid_t          si_uid;
    sigval_t       si_value;
};

struct passwd {
    char* pw_name;
};

extern int access( char const* pathName, int mode );
extern int clock_gettime( int clk_id, timespec* tp );
extern int getpid( );
extern int getpwuid_r( uid_t uid, passwd* pwd, char* buf, size_t buflen, passwd** result );
extern int getuid( );
extern int kill( pid_t pid, int sig );
extern struct tm* gmtime_r( time_t const* timep, tm* result );
extern int mkdir( char const* pathName, int mode );
extern char* strsignal( int sig );
extern int sigqueue( pid_t pid, int sig, sigval_t const value );

#endif // !__WIN32JUNK_H__
