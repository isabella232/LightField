#include "pch.h"

#include "calibrationtab.h"

CalibrationTab::CalibrationTab( QWidget* parent ): QWidget( parent ) {
    debug( "+ CalibrationTab::`ctor: constructing instance at %p\n", this );

    setLayout( _layout );
}

CalibrationTab::~CalibrationTab( ) {
    debug( "+ CalibrationTab::`dtor: destroying instance at %p\n", this );
}
