#include "pch.h"

#include <ctime>

#include "utils.h"

QVBoxLayout* WrapWidgetInVBox( QWidget* widget, Qt::AlignmentFlag const alignment ) {
    auto layout = new QVBoxLayout;

    layout->setAlignment( alignment );
    layout->setContentsMargins( { } );
    layout->addWidget( widget );
    return layout;
}

QHBoxLayout* WrapWidgetsInHBox( std::initializer_list<QWidget*> widgets ) {
    auto layout = new QHBoxLayout;

    for ( auto widget : widgets ) {
        if ( widget ) {
            layout->addWidget( widget );
        } else {
            layout->addStretch( );
        }
    }
    return layout;
}

QVBoxLayout* WrapWidgetsInVBox( std::initializer_list<QWidget*> widgets ) {
    auto layout = new QVBoxLayout;

    for ( auto widget : widgets ) {
        if ( widget ) {
            layout->addWidget( widget );
        } else {
            layout->addStretch( );
        }
    }
    return layout;
}

QFont ModifyFont( QFont const& font_, float const pointSizeF ) {
    auto font { font_ };
    font.setPointSizeF( pointSizeF );
    return font;
}

QFont ModifyFont( QFont const& font_, float const pointSizeF, QFont::Weight const weight ) {
    auto font { font_ };
    font.setPointSizeF( pointSizeF );
    font.setWeight( weight );
    return font;
}

QPalette ModifyPalette( QPalette const& palette_, QPalette::ColorGroup const group, QPalette::ColorRole const role, QColor const& color ) {
    auto palette { palette_ };
    palette.setColor( group, role, color );
    return palette;
}

QPalette ModifyPalette( QPalette const& palette_, QPalette::ColorRole const role, QColor const& color ) {
    auto palette { palette_ };
    palette.setColor( role, color );
    return palette;
}

QString GetUserName( ) {
    struct passwd pwd;
    char* buf = new char[16384];
    struct passwd* result;

    if ( 0 != ::getpwuid_r( ::getuid( ), &pwd, buf, 16384, &result ) ) {
        debug( "@@@ + SelectTab::_lookForUsbStick: getpwuid_r failed?!\n" );
        delete[] buf;
        return QString( );
    }

    QString userName { pwd.pw_name };
    delete[] buf;
    return userName;
}

QString GetFirstDirectoryIn( QString const& directory ) {
    auto dir = new QDir( directory );
    dir->setFilter( QDir::Dirs );

    QString dirname;
    for ( auto name : dir->entryList( ) ) {
        if ( ( name == "." ) || ( name == ".." ) ) {
            continue;
        }

        dirname = name;
        break;
    }

    return dirname;
}

qreal Distance( QPointF const& a, QPointF const& b ) {
    return sqrt( pow( a.x( ) - b.x( ), 2.0 ) + pow( a.y( ) - b.y( ), 2.0 ) );
}

int Distance( QPoint const& a, QPoint const& b ) {
    return sqrt( pow( a.x( ) - b.x( ), 2.0 ) + pow( a.y( ) - b.y( ), 2.0 ) );
}

double GetBootTimeClock( ) {
    timespec now;
    clock_gettime( CLOCK_BOOTTIME, &now );
    return now.tv_sec + now.tv_nsec / 1'000'000'000.0;
}

void BreakDownTime( uint64_t totalSeconds, int& days, int& hours, int& minutes, int& seconds ) {
    seconds = totalSeconds % 60; totalSeconds /= 60;
    minutes = totalSeconds % 60; totalSeconds /= 60;
    hours   = totalSeconds % 24;
    days    = totalSeconds / 24;
}

QString TimeDeltaToString( double delta ) {
    int days, hours, minutes, seconds;
    BreakDownTime( delta + 0.5, days, hours, minutes, seconds );

    QString timeString;
    if ( days > 0 ) {
        timeString = QString( "%1d " ).arg( days );
    }
    timeString += QString( "%1h%2m%3s" )
        .arg( hours,   2, 10, QChar( '0' ) )
        .arg( minutes, 2, 10, QChar( '0' ) )
        .arg( seconds, 2, 10, QChar( '0' ) );

    return timeString;
}
