#ifndef __UTILS_H__
#define __UTILS_H__

qreal        Distance( QPointF const& a, QPointF const& b );
int          Distance( QPoint  const& a, QPoint  const& b );

double       GetBootTimeClock( );
bool         GetFileSystemInfoFromPath( QString const& fileName, qint64& bytesFree, qint64& optimalWriteBlockSize );
QString      GetUserName( );

template<typename... Rest>
inline QFont ModifyFont( QFont const& font, QString       const& first, Rest const&... rest ) { return ModifyFont( ModifyFont( font, first ), rest... ); }
template<typename... Rest>
inline QFont ModifyFont( QFont const& font, double        const  first, Rest const&... rest ) { return ModifyFont( ModifyFont( font, first ), rest... ); }
template<typename... Rest>
inline QFont ModifyFont( QFont const& font, QFont::Weight const  first, Rest const&... rest ) { return ModifyFont( ModifyFont( font, first ), rest... ); }

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

QPalette     ModifyPalette( QPalette const& palette, QPalette::ColorGroup const group, QPalette::ColorRole const role, QColor const& color );
QPalette     ModifyPalette( QPalette const& palette,                                   QPalette::ColorRole const role, QColor const& color );

QString      ReadWholeFile( QString const& fileName );

QHBoxLayout* WrapWidgetsInHBox( std::initializer_list<QWidget*> widgets );
QVBoxLayout* WrapWidgetsInVBox( std::initializer_list<QWidget*> widgets );

inline QString GetFileBaseName( QString const& fileName ) {
    return fileName.mid( fileName.lastIndexOf( Slash ) + 1 );
}

inline QString RemoveFileExtension( QString const& fileName ) {
    return fileName.left( fileName.lastIndexOf( QChar( '.' ) ) );
}

template<typename... Rest>
inline QSize minSize( QSize const& first, QSize const& second, Rest const&... rest ) {
    return minSize( QSize {
        std::min( first.width( ),  second.width( )  ),
        std::min( first.height( ), second.height( ) )
    }, rest... );
}

template<>
inline QSize minSize( QSize const& first, QSize const& second ) {
    return QSize {
        std::min( first.width( ),  second.width( )  ),
        std::min( first.height( ), second.height( ) )
    };
}

template<typename... Rest>
inline QSize maxSize( QSize const& first, QSize const& second, Rest const&... rest ) {
    return maxSize( QSize {
        std::max( first.width( ),  second.width( )  ),
        std::max( first.height( ), second.height( ) )
    }, rest... );
}

template<>
inline QSize maxSize( QSize const& first, QSize const& second ) {
    return QSize {
        std::max( first.width( ),  second.width( )  ),
        std::max( first.height( ), second.height( ) )
    };
}

inline int PercentagePowerLevelToRawLevel( int percentage ) {
    return static_cast<int>( static_cast<double>( percentage ) / 100.0 * 255.0 + 0.5 );
}

inline int RawPowerLevelToPercentage( int raw ) {
    return static_cast<int>( static_cast<double>( raw ) / 255.0 * 100.0 + 0.5 );
}

void ScaleSize( qint64 const inputSize, double& scaledSize, char const*& suffix );

void PrintBacktrace( char const* tracerName );

#endif // __UTILS_H__
