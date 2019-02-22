#include "pch.h"

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
