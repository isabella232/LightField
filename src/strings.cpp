#include "pch.h"

#include "shepherd.h"

namespace {

    char const* ProcessErrorStrings[] {
        "FailedToStart",
        "Crashed",
        "Timedout",
        "WriteError",
        "ReadError",
        "UnknownError",
    };

    char const* ProcessStateStrings[] {
        "NotRunning",
        "Starting",
        "Running",
    };

    char const* ExitStatusStrings[] {
        "NormalExit",
        "CrashExit"
    };

    char const* PendingCommandStrings[] {
        "none",
        "move",
        "moveTo",
        "home",
        "lift",
    };

}

char const* ToString( QProcess::ProcessError value ) {
#if defined _DEBUG
    if ( ( value >= QProcess::FailedToStart ) && ( value <= QProcess::UnknownError ) ) {
#endif
        return ProcessErrorStrings[value];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

char const* ToString( QProcess::ProcessState value ) {
#if defined _DEBUG
    if ( ( value >= QProcess::NotRunning ) && ( value <= QProcess::Running ) ) {
#endif
        return ProcessStateStrings[value];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

char const* ToString( QProcess::ExitStatus value ) {
#if defined _DEBUG
    if ( ( value >= QProcess::NormalExit ) && ( value <= QProcess::CrashExit ) ) {
#endif
        return ExitStatusStrings[value];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

char const* ToString( PendingCommand value ) {
#if defined _DEBUG
    if ( ( value >= PendingCommand::none ) && ( value <= PendingCommand::lift ) ) {
#endif
        return PendingCommandStrings[static_cast<int>( value )];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}
