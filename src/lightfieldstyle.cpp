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
    if ( ( QStyle::PM_SliderThickness == metric ) || ( QStyle::PM_SliderLength == metric ) ) {
        return 50;
    }
    return QProxyStyle::pixelMetric( metric, option, widget );;
}
