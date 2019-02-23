#ifndef __UTILS_H__
#define __UTILS_H__

QVBoxLayout* WrapWidgetInVBox( QWidget* widget, Qt::AlignmentFlag const alignment = Qt::AlignCenter );

QFont ModifyFont( QFont const& font_, float const pointSizeF );
QFont ModifyFont( QFont const& font_, float const pointSizeF, QFont::Weight const weight );

QString GetUserName( );
QString GetFirstDirectoryIn( QString const& directory );

#endif // __UTILS_H__
