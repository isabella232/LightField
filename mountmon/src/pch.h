#if ! defined __PCH_H__
#define __PCH_H__

#include <QtCore/QtCore>
#include <QtDBus/QtDBus>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <grp.h>
#include <signal.h>
#include <unistd.h>

#include <cstdio>

#include <bitset>
#include <memory>

#define debug(...)  fprintf( stderr, __VA_ARGS__ )
#define _countof(x) (sizeof(x) / sizeof(x[0]))

#ifdef WIN32
#include "../../src/win32junk.h"
#endif // WIN32

#endif // ! defined __PCH_H__
