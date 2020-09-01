#include "pch.h"
#include <ctime>
#include <iomanip>
#include "debug.h"

namespace {

    FILE* DebugLog { };
    FILE* FirmwareLog { };
    FILE* OriginalStderr { stderr };

}

DebugManager::DebugManager(DebugType type, const char **paths) {
    _type = type;
    _paths = paths;

    rotate();
}

void DebugManager::rotate() {
    ::unlink(_paths[5]);
    for (int n = 5; n > 0; --n) {
        ::rename(_paths[n - 1], _paths[n]);
    }

    FILE **log = &DebugLog;

    switch (_type) {
    case (DebugType::APP):
        log = &DebugLog;
        break;
    case (DebugType::FIRMWARE):
        log = &FirmwareLog;
        break;
    }

    *log = ::fopen(_paths[0], "wtx");
    if (!*log) {
        error_t err = errno;
        ::fprintf(stderr, "failed to open log file '%s': %s [%d]", _paths[0], strerror(err),
            err);
    }

    ::setvbuf(*log, nullptr, _IONBF, 0);
    ::setvbuf(OriginalStderr, nullptr, _IONBF, 0);
}

DebugManager::~DebugManager() {
    /*empty*/
}

void debug(char const* str) {
    time_t timeNow = std::time(nullptr);

    std::stringstream ss;

    ss << std::put_time(std::localtime(&timeNow), "%OH:%OM:%OS | ") << str;

    const std::string formattedOutput = ss.str();
    const char* cstr = formattedOutput.c_str();

    if (DebugLog) {
        ::fputs(cstr, DebugLog);
    }

    ::fputs(cstr, OriginalStderr);
}

void firmware_debug(char const* str) {
    time_t timeNow = std::time(nullptr);

    std::stringstream ss;

    ss << std::put_time(std::localtime(&timeNow), "%OH:%OM:%OS | ") << str;

    const std::string formattedOutput = ss.str();
    const char* cstr = formattedOutput.c_str();

    if (FirmwareLog) {
        ::fputs(cstr, FirmwareLog);
    }

    ::fputs(cstr, OriginalStderr);
}
