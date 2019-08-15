#include "pch.h"

#include "timinglogger.h"

#include "utils.h"

namespace {

    char const* TimingIdStrings[] {
        "unknown",
        "VolumeCalculation",
        "Slicing",
        "Printing",
        "AllUpdatesValidation",
        "OneUpdateValidation",
        "InstallUpdate",
    };

    struct TimingInfo {
        timespec startTime { };
        timespec stopTime  { };
        bool     running   { };
    };

    TimingInfo           timingInfo[7] { };
    std::recursive_mutex timingLock;

    constexpr int operator+( TimingId const value ) {
        return static_cast<int>( value );
    }

    char const* ToString( TimingId value ) {
#if defined _DEBUG
        if ( ( value >= TimingId::unknown ) && ( value <= TimingId::InstallUpdate ) ) {
#endif
            return TimingIdStrings[+value];
#if defined _DEBUG
        } else {
            return nullptr;
        }
#endif
    }

}

void TimingLogger::startTiming( TimingId const id ) {
    debug( "+ TimingLogger::startTiming: id=%s\n", ToString( id ) );
    std::lock_guard<std::recursive_mutex> lock { timingLock };

    if ( timingInfo[+id].running ) {
        debug(
            "********** PANIC **********\n"
            "Timing ID %s already running!!\n"
        );
        PrintBacktrace( "TimingLogger::startTiming" );
        abort( );
    }

    clock_gettime( CLOCK_BOOTTIME, &timingInfo[+id].startTime );
    timingInfo[+id].running = true;
}

void TimingLogger::stopTiming( TimingId const id ) {
    debug( "+ TimingLogger::stopTiming: id=%s\n", ToString( id ) );
    std::lock_guard<std::recursive_mutex> lock { timingLock };

    if ( !timingInfo[+id].running ) {
        debug(
            "********** PANIC **********\n"
            "Timing ID %s not running!!\n"
        );
        PrintBacktrace( "TimingLogger::stopTiming" );
        abort( );
    }

    clock_gettime( CLOCK_BOOTTIME, &timingInfo[+id].stopTime );
    timingInfo[+id].running = false;

    double start = timingInfo[+id].startTime.tv_sec + timingInfo[+id].startTime.tv_nsec / 1'000'000'000.0;
    double stop  = timingInfo[+id].stopTime .tv_sec + timingInfo[+id].stopTime .tv_nsec / 1'000'000'000.0;
    debug( "|TIMING|%s %.6f\n", ToString( id ), stop - start );
}
