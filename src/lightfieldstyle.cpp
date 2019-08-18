#include "pch.h"

#include "lightfieldstyle.h"

LightFieldStyle::LightFieldStyle( QString const& key ): QProxyStyle( key ) {
    /*empty*/
}

LightFieldStyle::LightFieldStyle( QStyle* style ): QProxyStyle( style ) {
    /*empty*/
}

LightFieldStyle::~LightFieldStyle( ) {
    /*empty*/
}

int LightFieldStyle::pixelMetric( QStyle::PixelMetric metric, QStyleOption const* option, QWidget const* widget ) const {
    switch ( metric ) {
        case QStyle::PM_ScrollBarExtent: return 30;
        case QStyle::PM_SliderLength:    return 50;
        case QStyle::PM_SliderThickness: return 50;
        default:                         return QProxyStyle::pixelMetric( metric, option, widget );
    }
}
