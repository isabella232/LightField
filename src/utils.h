#ifndef __UTILS_H__
#define __UTILS_H__

#include <QtCore>
#include <QtWidgets>
#include "constants.h"

double   GetBootTimeClock( );
bool     GetFileSystemInfoFromPath( QString const& fileName, qint64& bytesFree, qint64& optimalWriteBlockSize );
QString  GetUserName( );
void     ScaleSize( qint64 const inputSize, double& scaledSize, char const*& suffix );
void     PrintBacktrace( char const* tracerName );
QPalette ModifyPalette( QPalette const& palette, QPalette::ColorGroup const group, QPalette::ColorRole const role, QColor const& color );
QPalette ModifyPalette( QPalette const& palette, QPalette::ColorRole const role, QColor const& color );
QString  ReadWholeFile( QString const& fileName );
bool     YesNoPrompt( QWidget* parent, QString const& title, QString const& text );
void     RebootPrinter( );
void     ShutDownPrinter( );

inline constexpr qreal Distance( QPointF const& a, QPointF const& b ) {
    return sqrt( pow( a.x( ) - b.x( ), 2.0 ) + pow( a.y( ) - b.y( ), 2.0 ) );
}

inline constexpr int Distance( QPoint const& a, QPoint const& b ) {
    return sqrt( pow( a.x( ) - b.x( ), 2.0 ) + pow( a.y( ) - b.y( ), 2.0 ) );
}

inline QString GetFileBaseName( QString const& fileName ) {
    return fileName.mid( fileName.lastIndexOf( Slash ) + 1 );
}

inline QString RemoveFileExtension( QString const& fileName ) {
    return fileName.left( fileName.lastIndexOf( QChar( '.' ) ) );
}

inline constexpr int PercentagePowerLevelToRawLevel( int percentage ) {
    return static_cast<int>( static_cast<double>( percentage ) / 100.0 * ProjectorMaxPowerLevel + 0.5 );
}

inline constexpr int RawPowerLevelToPercentage( int raw ) {
    return static_cast<int>( static_cast<double>( raw ) / ProjectorMaxPowerLevel * 100.0 + 0.5 );
}

template<typename... Rest>
inline QFont ModifyFont( QFont const& font, QString const& first, Rest const&... rest ) {
    return ModifyFont( ModifyFont( font, first ), rest... );
}

template<typename... Rest>
inline QFont ModifyFont( QFont const& font, double const  first, Rest const&... rest ) {
    return ModifyFont( ModifyFont( font, first ), rest... );
}

template<typename... Rest>
inline QFont ModifyFont( QFont const& font, QFont::Weight const  first, Rest const&... rest ) {
    return ModifyFont( ModifyFont( font, first ), rest... );
}

template<>
inline QFont ModifyFont( QFont const& font_, QString const& familyName ) {
    auto font { font_ };
    font.setFamily( familyName );
    return font;
}

template<>
inline QFont ModifyFont( QFont const& font_, double const pointSizeF ) {
    auto font { font_ };
    font.setPointSizeF( pointSizeF );
    return font;
}

template<>
inline QFont ModifyFont( QFont const& font_, QFont::Weight const weight ) {
    auto font { font_ };
    font.setWeight( weight );
    return font;
}

template<typename... Args>
inline QHBoxLayout* WrapWidgetsInHBox( Args... args ) {
    auto hbox { new QHBoxLayout };
    hbox->setContentsMargins( { } );
    return _WrapWidgetsInHBoxImpl( hbox, args... );
}

template<typename... Args>
inline QHBoxLayout* WrapWidgetsInHBoxDM( Args... args ) {
    return _WrapWidgetsInHBoxImpl( new QHBoxLayout, args... );
}

template<typename... Rest>
inline QHBoxLayout* _WrapWidgetsInHBoxImpl( QHBoxLayout* hbox, QWidget* widget, Rest... rest ) {
    hbox->addWidget( widget );
    return _WrapWidgetsInHBoxImpl( hbox, rest... );
}

template<typename... Rest>
inline QHBoxLayout* _WrapWidgetsInHBoxImpl( QHBoxLayout* hbox, QLayout* layout, Rest... rest ) {
    hbox->addLayout( layout );
    return _WrapWidgetsInHBoxImpl( hbox, rest... );
}

template<typename... Rest>
inline QHBoxLayout* _WrapWidgetsInHBoxImpl( QHBoxLayout* hbox, std::nullptr_t, Rest... rest ) {
    hbox->addStretch( );
    return _WrapWidgetsInHBoxImpl( hbox, rest... );
}

inline QHBoxLayout* _WrapWidgetsInHBoxImpl( QHBoxLayout* hbox ) {
    return hbox;
}

template<typename... Args>
inline QVBoxLayout* WrapWidgetsInVBox( Args... args ) {
    auto vbox { new QVBoxLayout };
    vbox->setContentsMargins( { } );
    return _WrapWidgetsInVBoxImpl( vbox, args... );
}

template<typename... Args>
inline QVBoxLayout* WrapWidgetsInVBoxDM( Args... args ) {
    return _WrapWidgetsInVBoxImpl( new QVBoxLayout, args... );
}

template<typename... Rest>
inline QVBoxLayout* _WrapWidgetsInVBoxImpl( QVBoxLayout* vbox, QWidget* widget, Rest... rest ) {
    vbox->addWidget( widget );
    return _WrapWidgetsInVBoxImpl( vbox, rest... );
}

template<typename... Rest>
inline QVBoxLayout* _WrapWidgetsInVBoxImpl( QVBoxLayout* vbox, QLayout* layout, Rest... rest ) {
    vbox->addLayout( layout );
    return _WrapWidgetsInVBoxImpl( vbox, rest... );
}

template<typename... Rest>
inline QVBoxLayout* _WrapWidgetsInVBoxImpl( QVBoxLayout* vbox, std::nullptr_t, Rest... rest ) {
    vbox->addStretch( );
    return _WrapWidgetsInVBoxImpl( vbox, rest... );
}

inline QVBoxLayout* _WrapWidgetsInVBoxImpl( QVBoxLayout* vbox ) {
    return vbox;
}

template<typename... Rest>
inline QSize minSize( QSize const& first, QSize const& second, Rest const&... rest ) {
    return minSize( QSize {
        std::min( first.width( ),  second.width( ) ),
        std::min( first.height( ), second.height( ) )
    }, rest... );
}

template<>
inline QSize minSize( QSize const& first, QSize const& second ) {
    return QSize {
        std::min( first.width( ),  second.width( ) ),
        std::min( first.height( ), second.height( ) )
    };
}

template<typename... Rest>
inline QSize maxSize( QSize const& first, QSize const& second, Rest const&... rest ) {
    return maxSize( QSize {
        std::max( first.width( ),  second.width( ) ),
        std::max( first.height( ), second.height( ) )
    }, rest... );
}

template<>
inline QSize maxSize( QSize const& first, QSize const& second ) {
    return QSize {
        std::max( first.width( ),  second.width( ) ),
        std::max( first.height( ), second.height( ) )
    };
}

#endif // __UTILS_H__
