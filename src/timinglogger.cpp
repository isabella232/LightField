#include "pch.h"

#include "timinglogger.h"

namespace {

    char const* TimingIdStrings[] {
        "unknown",
        "VolumeCalculation",
        "SlicingSvg",
        "RenderingPngs",
        "Printing",
        "UpgradeCheck",
        "UpgradeInstallation",
    };

    struct TimingInfo {
        timespec startTime { };
        timespec stopTime  { };
        QString  note      { };
        bool     running   { };
    };

    TimingInfo           timingInfo[7] { };
    std::recursive_mutex timingLock;

    constexpr int operator+( TimingId const value ) {
        return static_cast<int>( value );
    }

    char const* ToString( TimingId value ) {
#if defined _DEBUG
        if ( ( value >= TimingId::unknown ) && ( value <= TimingId::UpgradeInstallation ) ) {
#endif
            return TimingIdStrings[+value];
#if defined _DEBUG
        } else {
            return nullptr;
        }
#endif
    }

}

void TimingLogger::startTiming( TimingId const id, QString const& note ) {
    std::lock_guard<std::recursive_mutex> lock { timingLock };

    if ( timingInfo[+id].running ) {
        debug( "+ TimingLogger::startTiming: Timing ID %s is already running!\n", ToString( id ) );
        return;
    }

    timingInfo[+id].running = true;
    timingInfo[+id].note    = note;
    clock_gettime( CLOCK_BOOTTIME, &timingInfo[+id].startTime );
}

void TimingLogger::stopTiming( TimingId const id ) {
    std::lock_guard<std::recursive_mutex> lock { timingLock };

    if ( !timingInfo[+id].running ) {
        debug( "+ TimingLogger::stopTiming: Timing ID %s is not running!\n", ToString( id ) );
        return;
    }

    clock_gettime( CLOCK_BOOTTIME, &timingInfo[+id].stopTime );
    timingInfo[+id].running = false;

    double start = timingInfo[+id].startTime.tv_sec + timingInfo[+id].startTime.tv_nsec / 1'000'000'000.0;
    double stop  = timingInfo[+id].stopTime .tv_sec + timingInfo[+id].stopTime .tv_nsec / 1'000'000'000.0;

    if ( timingInfo[+id].note.isEmpty( ) ) {
        debug( "|TIMING|%s %.6f\n", ToString( id ), stop - start );
    } else {
        debug( "|TIMING|%s [%s] %.6f\n", ToString( id ), timingInfo[+id].note.toUtf8( ).data( ), stop - start );
    }
}
