#include "pch.h"

#include "shepherd.h"
#include "window.h"

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
        "send",
    };

    char const* TabIndexStrings[] {
        "File",
        "Prepare",
        "Print",
        "Status",
        "Advanced",
        "Maintenance",
    };

    char const* SwipeDirectionStrings[] {
        "NoDirection",
        "Left",
        "Right",
        "Up",
        "Down"
    };

    char const* GestureStateStrings[] {
        "NoGesture",
        "GestureStarted",
        "GestureUpdated",
        "GestureFinished",
        "GestureCanceled",
    };

}

char const* ToString( QProcess::ProcessError const value ) {
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

char const* ToString( QProcess::ProcessState const value ) {
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

char const* ToString( QProcess::ExitStatus const value ) {
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

char const* ToString( QDialog::DialogCode const value ) {
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

QString ToString( QPoint const value ) {
    return QString( "(%1,%2)" ).arg( value.x( ) ).arg( value.y( ) );
}

QString ToString( QSize const value ) {
    return QString( "%1×%2" ).arg( value.width( ) ).arg( value.height( ) );
}

QString ToString( QRect const value ) {
    return QString( "%1-%2 [%3]" ).arg( ToString( value.topLeft( ) ) ).arg( ToString( value.bottomRight( ) ) ).arg( ToString( value.size( ) ) );
}

QString ToString( QPointF const value ) {
    return QString( "(%1,%2)" ).arg( value.x( ), 0, 'f', 2 ).arg( value.y( ), 0, 'f', 2 );
}

QString ToString( QSizeF const value ) {
    return QString( "%1×%2" ).arg( value.width( ), 0, 'f', 2 ).arg( value.height( ), 0, 'f', 2 );
}

QString ToString( QRectF const value ) {
    return QString( "%1-%2 [%3]" ).arg( ToString( value.topLeft( ) ) ).arg( ToString( value.bottomRight( ) ) ).arg( ToString( value.size( ) ) );
}

char const* ToString( PendingCommand const value ) {
#if defined _DEBUG
    if ( ( value >= PendingCommand::none ) && ( value <= PendingCommand::send ) ) {
#endif
        return PendingCommandStrings[static_cast<int>( value )];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

char const* ToString( TabIndex const value ) {
#if defined _DEBUG
    if ( ( value >= TabIndex::File ) && ( value <= TabIndex::Maintenance ) ) {
#endif
        return TabIndexStrings[static_cast<int>( value )];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

char const* ToString( QSwipeGesture::SwipeDirection const value ) {
#if defined _DEBUG
    if ( ( value >= QSwipeGesture::NoDirection ) && ( value <= QSwipeGesture::Down ) ) {
#endif
        return SwipeDirectionStrings[static_cast<int>( value )];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

char const* ToString( Qt::GestureState const value ) {
#if defined _DEBUG
    if ( ( value >= Qt::NoGesture ) && ( value <= Qt::GestureCanceled ) ) {
#endif
        return GestureStateStrings[static_cast<int>( value )];
#if defined _DEBUG
    } else {
        return nullptr;
    }
#endif
}

QString FormatDouble( double const value, int const fieldWidth, int const precision ) {
    QString str = QString( "%1" ).arg( value, fieldWidth, 'f', precision );
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

QString GroupDigits( QString const& input, char const groupSeparator ) {
    QString str { input };

    int index = str.indexOf( QChar( '.' ) );
    if ( -1 == index ) {
        index = str.length( );
    }

    while ( index - 3 > 0 ) {
        index -= 3;
        str.insert( index, QChar( ',' ) );
    }
    return str;
}
