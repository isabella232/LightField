#ifndef __UTILS_H__
#define __UTILS_H__

QHBoxLayout* WrapWidgetsInHBox( std::initializer_list<QWidget*> widgets );
QVBoxLayout* WrapWidgetsInVBox( std::initializer_list<QWidget*> widgets );

QFont ModifyFont( QFont const& font, QString       const& familyName );
QFont ModifyFont( QFont const& font, double        const  pointSizeF );
QFont ModifyFont( QFont const& font, QFont::Weight const  weight     );
QFont ModifyFont( QFont const& font, QString       const& familyName, double        const  pointSizeF );
QFont ModifyFont( QFont const& font, QString       const& familyName, QFont::Weight const  weight     );
QFont ModifyFont( QFont const& font, double        const  pointSizeF, QFont::Weight const  weight     );
QFont ModifyFont( QFont const& font, QString       const& familyName, double        const  pointSizeF, QFont::Weight const  weight );

QPalette ModifyPalette( QPalette const& palette, QPalette::ColorGroup const group, QPalette::ColorRole const role, QColor const& color );
QPalette ModifyPalette( QPalette const& palette, QPalette::ColorRole const role, QColor const& color );

QString GetUserName( );

qreal Distance( QPointF const& a, QPointF const& b );
int   Distance( QPoint  const& a, QPoint  const& b );

double GetBootTimeClock( );

QString ReadWholeFile( QString const& fileName );

bool GetFileSystemInfoFromPath( QString const& fileName, qint64& bytesFree, qint64& optimalWriteBlockSize );

inline QString GetFileBaseName( QString const& fileName ) {
    return fileName.mid( fileName.lastIndexOf( Slash ) + 1 );
}

inline QString RemoveFileExtension( QString const& fileName ) {
    return fileName.left( fileName.lastIndexOf( QChar( '.' ) ) );
}

inline QSize minSize( QSize const& a, QSize const& b ) {
    return QSize { std::min( a.width( ), b.width( ) ), std::min( a.height( ), b.height( ) ) };
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
