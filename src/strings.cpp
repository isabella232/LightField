#include "pch.h"

#include "shepherd.h"
#include "strings.h"

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
        "CrashExit",
    };

    char const* DialogCodeStrings[] {
        "Rejected",
        "Accepted",
    };

    char const* PendingCommandStrings[] {
        "none",
        "move",
        "moveTo",
        "home",
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

char const* ToString( QDialog::DialogCode value ) {
#if defined _DEBUG
    if ( ( value >= QDialog::Rejected ) && ( value <= QDialog::Accepted ) ) {
#endif
        return DialogCodeStrings[static_cast<int>( value )];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

char const* ToString( PendingCommand value ) {
#if defined _DEBUG
    if ( ( value >= PendingCommand::none ) && ( value <= PendingCommand::home ) ) {
#endif
        return PendingCommandStrings[static_cast<int>( value )];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

QString FormatDouble( double a, int prec ) {
    QString str = QString( "%1" ).arg( a, 0, 'f', prec );
    int index = str.length( ) - 1;
    while ( ( index > -1 ) && ( str[index].unicode( ) == L'0' ) ) {
        --index;
    }
    if ( str[index].unicode( ) == L'.' ) {
        --index;
    }
    str.resize( index + 1 );
    return str;
}
