#ifndef __WIN32JUNK_H__
#define __WIN32JUNK_H__

#include <io.h>

//================================================
// Type aliases
//================================================

using error_t   = errno_t;
using gid_t     = int;
using pid_t     = int;
using socklen_t = unsigned;
using uid_t     = int;

//================================================
// Constants
//================================================

constexpr auto const R_OK           = 4;
constexpr auto const W_OK           = 2;
constexpr auto const X_OK           = 1;
constexpr auto const F_OK           = 0;

constexpr auto const CLOCK_BOOTTIME = 7;

constexpr auto const SIGHUP         =  1;
constexpr auto const SIGQUIT        =  3;
constexpr auto const SIGUSR1        = 10;
constexpr auto const SIGUSR2        = 11;

constexpr auto const SA_SIGINFO     = 0x00000004;
constexpr auto const SA_RESTART     = 0x10000000;

constexpr auto const MSG_DONTWAIT   = 0x40;

//================================================
// Forward type declarations
//================================================

struct passwd;
struct sigaction;
struct siginfo_t;
struct sigset_t;
union  sigval_t;
struct statfs;

//================================================
// Type declarations
//================================================

struct passwd {
    char* pw_name;
};

struct sigaction {
    void     (*sa_handler)( int );
    void     (*sa_sigaction)( int, siginfo_t*, void* );
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)( void );
};

struct siginfo_t {
    int      si_signo;
    pid_t    si_pid;
    uid_t    si_uid;
    sigval_t si_value;
};

struct sigset_t {
    /*empty on purpose*/
};

union sigval_t {
    int   sival_int;
    void* sival_ptr;
};

struct statfs {
    quint64 f_bsize;
    quint64 f_blocks;
    quint64 f_bfree;
    quint64 f_bavail;
    quint64 f_frsize;
};

//================================================
// Function prototypes
//================================================

extern int   clock_gettime( int clk_id, timespec* tp );
extern int   getpid( );
extern int   getpwuid_r( uid_t uid, passwd* pwd, char* buf, size_t buflen, passwd** result );
extern int   getuid( );
extern int   kill( pid_t pid, int sig );
extern int   mkdir( char const* pathName, int mode );
extern int   sigaction( int signum, struct sigaction const* act, struct sigaction* oldact );
extern int   sigaddset( sigset_t* set, int signum );
extern int   sigemptyset( sigset_t* set );
extern int   sigqueue( pid_t pid, int sig, sigval_t const value );
extern int   sigtimedwait( sigset_t const* set, siginfo_t* info, timespec const* timeout );
extern int   socketpair( int domain, int type, int protocol, int sv[2] );
extern int   statfs( char const* path, struct statfs* buf );
extern char* strsignal( int sig );

#endif // !__WIN32JUNK_H__
