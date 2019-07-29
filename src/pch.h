#ifndef __PCH_H__
#define __PCH_H__

#if defined WIN32

// Yes, we have no intentions of running this software under Windows.
// However, one of the developers uses Visual Studio as their IDE, and it
// doesn't recognize POSIX stuff. They don't even try to *build* under
// Windows; they just want the IntelliSense® source browsing &c to work.

#   define _CRT_NONSTDC_NO_WARNINGS

#endif // defined WIN32

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pwd.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <atomic>
#include <exception>
#include <functional>
#include <future>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtXml/QtXml>

#include "constants.h"
#include "debug.h"
#include "app.h"
#include "strings.h"
#include "utils.h"

#if defined WIN32
#   include "win32junk.h"
#endif // defined WIN32

#define _countof(x) (sizeof(x) / sizeof(x[0]))

#endif // __PCH_H__
