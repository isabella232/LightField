#ifndef __TIMINGLOGGER_H__
#define __TIMINGLOGGER_H__

#include <QtCore>

enum class TimingId {
    unknown,
    VolumeCalculation,
    SlicingSvg,
    RenderingPngs,
    LoadingPngFolder,
    Printing,
    UpgradeCheck,
    UpgradeInstallation,
};

class TimingLogger {

    TimingLogger( )                                 = delete;
    ~TimingLogger( )                                = delete;
    TimingLogger( TimingLogger& )                   = delete;
    TimingLogger( TimingLogger const&& )            = delete;
    TimingLogger& operator=( TimingLogger& )        = delete;
    TimingLogger& operator=( TimingLogger const&& ) = delete;

public:

    static void startTiming( TimingId const id, QString const& note = { } );
    static void stopTiming( TimingId const id );

};

#endif // !__TIMINGLOGGER_H__
