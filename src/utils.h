#ifndef __UTILS_H__
#define __UTILS_H__

QHBoxLayout* WrapWidgetsInHBox( std::initializer_list<QWidget*> widgets );
QVBoxLayout* WrapWidgetsInVBox( std::initializer_list<QWidget*> widgets );

QFont ModifyFont( QFont const& font, double const pointSizeF );
QFont ModifyFont( QFont const& font, double const pointSizeF, QFont::Weight const weight );
QFont ModifyFont( QFont const& font, QFont::Weight const weight );
QFont ModifyFont( QFont const& font, QString const& familyName );

QPalette ModifyPalette( QPalette const& palette, QPalette::ColorGroup const group, QPalette::ColorRole const role, QColor const& color );
QPalette ModifyPalette( QPalette const& palette, QPalette::ColorRole const role, QColor const& color );

QString GetUserName( );

qreal Distance( QPointF const& a, QPointF const& b );
int   Distance( QPoint  const& a, QPoint  const& b );

double GetBootTimeClock( );

QString ReadWholeFile( QString const& fileName );

inline QString GetFileBaseName( QString const& fileName ) {
    return fileName.mid( fileName.lastIndexOf( Slash ) + 1 );
}

inline QString RemoveFileExtension( QString const& fileName ) {
    return fileName.left( fileName.lastIndexOf( QChar( '.' ) ) );
}

inline QSize minSize( QSize const& a, QSize const& b ) {
    return QSize { std::min( a.width( ), b.width( ) ), std::min( a.height( ), b.height( ) ) };
}

inline QSize maxSize( QSize const& a, QSize const& b ) {
    return QSize { std::max( a.width( ), b.width( ) ), std::max( a.height( ), b.height( ) ) };
}

inline int PercentagePowerLevelToRawLevel( int percentage ) {
    return static_cast<int>( static_cast<double>( percentage ) / 100.0 * 255.0 + 0.5 );
}

inline int RawPowerLevelToPercentage( int raw ) {
    return static_cast<int>( static_cast<double>( raw ) / 255.0 * 100.0 + 0.5 );
}

#endif // __UTILS_H__
