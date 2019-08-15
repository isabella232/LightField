#ifndef __TIMINGLOGGER_H__
#define __TIMINGLOGGER_H__

enum class TimingId {
    unknown,
    VolumeCalculation,
    Slicing,
    Printing,
    AllUpdatesValidation,
    OneUpdateValidation,
    InstallUpdate,
};

class TimingLogger {

public:

    static void startTiming( TimingId const id );
    static void stopTiming( TimingId const id );

};

#endif // !__TIMINGLOGGER_H__
