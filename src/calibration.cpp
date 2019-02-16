#include "pch.h"

#include "calibration.h"

CalibrationDialog::CalibrationDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ) {
    debug( "+ CalibrationDialog::`ctor: constructing instance at %p\n", this );

    _prevButton->setText( "&Prev" );
    _nextButton->setText( "&Next" );
    _cancelButton->setText( "Cancel" );

    QObject::connect( _prevButton,   &QPushButton::clicked, this, &CalibrationDialog::_prevButton_clicked   );
    QObject::connect( _nextButton,   &QPushButton::clicked, this, &CalibrationDialog::_nextButton_clicked   );
    QObject::connect( _cancelButton, &QPushButton::clicked, this, &CalibrationDialog::_cancelButton_clicked );

    _buttonsHBox->addWidget( _prevButton   );
    _buttonsHBox->addWidget( _nextButton   );
    _buttonsHBox->addWidget( _cancelButton );

    _layout->addLayout( _buttonsHBox );

    setLayout( _layout );
    setSizeGripEnabled( false );
}

CalibrationDialog::~CalibrationDialog( ) {
    debug( "+ CalibrationDialog::`dtor: destroying instance at %p\n", this );
}

void CalibrationDialog::_prevButton_clicked( bool ) {
    /*empty*/
}

void CalibrationDialog::_nextButton_clicked( bool ) {
    /*empty*/
}

void CalibrationDialog::_cancelButton_clicked( bool ) {
    reject( );
}
