#ifndef __WIN32JUNK_H__
#define __WIN32JUNK_H__

#include <io.h>

#undef GetUserName

//================================================
// Compatibility macros
//================================================

#define __attribute__(x)

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

constexpr auto const SIG_BLOCK   = 0;
constexpr auto const SIG_UNBLOCK = 1;
constexpr auto const SIG_SETMASK = 2;

//================================================
// Forward type declarations
//================================================

struct group;
struct passwd;
struct sigaction;
struct siginfo_t;
struct sigset_t;
union  sigval_t;
struct statvfs;

//================================================
// Type declarations
//================================================

struct group {
    gid_t gr_gid;
};

struct passwd {
    char* pw_name;
    uid_t pw_uid;
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

struct statvfs {
    unsigned long f_bsize;
    unsigned long f_bavail;
};

//================================================
// POSIX function prototypes
//================================================

extern int     chown( char const* pathname, uid_t owner, gid_t group );
extern int     clock_gettime( int clk_id, timespec* tp );
extern int     getpid( );
extern group*  getgrnam( const char* name );
extern int     getgrnam_r( char const* name, group* grp, char* buf, size_t buflen, group** result );
extern passwd* getpwnam( const char* name );
extern int     getpwnam_r( char const* name, passwd* pwd, char* buf, size_t buflen, passwd** result );
extern int     getpwuid_r( uid_t uid, passwd* pwd, char* buf, size_t buflen, passwd** result );
extern int     getuid( );
extern int     kill( pid_t pid, int sig );
extern int     mkdir( char const* pathName, int mode );
extern int     rmdir( char const* pathName );
extern int     sigaction( int signum, struct sigaction const* act, struct sigaction* oldact );
extern int     sigaddset( sigset_t* set, int signum );
extern int     sigemptyset( sigset_t* set );
extern int     sigprocmask( int how, sigset_t const* set, sigset_t* oldset );
extern int     sigqueue( pid_t pid, int sig, sigval_t const value );
extern int     sigtimedwait( sigset_t const* set, siginfo_t* info, timespec const* timeout );
extern int     socketpair( int domain, int type, int protocol, int sv[2] );
extern int     statvfs( char const* path, struct statvfs* buf );
extern char*   strsignal( int sig );

//================================================
// GNU C Library function prototypes
//================================================

extern int     backtrace( void** buffer, int size );
extern char**  backtrace_symbols( void* const* buffer, int size );

#endif // !__WIN32JUNK_H__
