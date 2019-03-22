#ifndef __UTILS_H__
#define __UTILS_H__

QVBoxLayout* WrapWidgetInVBox( QWidget* widget, Qt::AlignmentFlag const alignment = Qt::AlignCenter );

QHBoxLayout* WrapWidgetsInHBox( std::initializer_list<QWidget*> widgets );
QVBoxLayout* WrapWidgetsInVBox( std::initializer_list<QWidget*> widgets );

QFont ModifyFont( QFont const& font_, double const pointSizeF );
QFont ModifyFont( QFont const& font_, double const pointSizeF, QFont::Weight const weight );
QFont ModifyFont( QFont const& font_, QFont::Weight const weight );
QFont ModifyFont( QFont const& font_, QString const& familyName );

QPalette ModifyPalette( QPalette const& palette_, QPalette::ColorGroup const group, QPalette::ColorRole const role, QColor const& color );
QPalette ModifyPalette( QPalette const& palette_, QPalette::ColorRole const role, QColor const& color );

QString GetUserName( );
QString GetFirstDirectoryIn( QString const& directory );

qreal Distance( QPointF const& a, QPointF const& b );
int   Distance( QPoint  const& a, QPoint  const& b );

double GetBootTimeClock( );

void BreakDownTime( uint64_t totalSeconds, int& days, int& hours, int& minutes, int& seconds );

QString TimeDeltaToString( double delta );

inline QString GetFileBaseName( QString const& fileName ) {
    return fileName.mid( fileName.lastIndexOf( Slash ) + 1 );
}

inline QString RemoveFileExtension( QString const& fileName ) {
    return fileName.left( fileName.lastIndexOf( QChar( '.' ) ) );
}

#endif // __UTILS_H__
