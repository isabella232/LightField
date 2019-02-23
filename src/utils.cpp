#include "pch.h"

#include <pwd.h>

#include "utils.h"

QVBoxLayout* WrapWidgetInVBox( QWidget* widget, Qt::AlignmentFlag const alignment ) {
    auto layout = new QVBoxLayout;

    layout->setAlignment( alignment );
    layout->setContentsMargins( { } );
    layout->addWidget( widget );
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
