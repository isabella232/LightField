#ifndef __UTILS_H__
#define __UTILS_H__

QVBoxLayout* WrapWidgetInVBox( QWidget* widget, Qt::AlignmentFlag const alignment = Qt::AlignCenter );

QFont ModifyFont( QFont const& font_, float const pointSizeF );
QFont ModifyFont( QFont const& font_, float const pointSizeF, QFont::Weight const weight );

QPalette ModifyPalette( QPalette const& palette_, QPalette::ColorGroup const group, QPalette::ColorRole const role, QColor const& color );
QPalette ModifyPalette( QPalette const& palette_, QPalette::ColorRole const role, QColor const& color );

QString GetUserName( );
QString GetFirstDirectoryIn( QString const& directory );

qreal Distance( QPointF const& a, QPointF const& b );
int   Distance( QPoint  const& a, QPoint  const& b );

#endif // __UTILS_H__
