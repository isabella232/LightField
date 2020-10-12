#include <string.h>
#include <limits.h>

#include "libprojector.h"
#include "dlpc350impl.h"
#include "dlp4710impl.h"

bool ProjectorController::stringToUnsignedLong(char const *ptr, int const radix, unsigned long *result)
{
    char *endptr = NULL;

    errno = 0;
    unsigned long ret = strtoul(ptr, &endptr, radix);
    if (endptr != (ptr + strlen(ptr)))
    {
        return false;
    }
    if (errno == ERANGE && ret == ULONG_MAX)
    {
        return false;
    }
    if (errno != 0 && ret == 0UL)
    {
        return false;
    }

    *result = ret;
    return true;
}

ProjectorController* ProjectorController::getInstance()
{
    if(ProjectorDlpc350Impl::devicePlugged()) {
        return new ProjectorDlpc350Impl();
    } else {
        return new ProjectorDlp4710Impl();
    }
}