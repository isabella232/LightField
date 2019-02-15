#include "pch.h"

#include "bedheightadjustment.h"

BedHeightAdjustmentDialog::BedHeightAdjustmentDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ) {
    debug( "+ BedHeightAdjustmentDialog::`ctor: constructing instance at %p\n", this );

    _bedHeightLabel->setText( "Bed height (mm):" );
    _bedHeightLabel->setBuddy( _bedHeightLineEdit );

    _bedHeightLineEdit->setAlignment( Qt::AlignRight );
    _bedHeightLineEdit->setText( "0.0" );
    _bedHeightLineEdit->setValidator( new QDoubleValidator( -100.0, 100.0, 5 ) );
    QObject::connect( _bedHeightLineEdit, &QLineEdit::editingFinished, this, &BedHeightAdjustmentDialog::_bedHeightLineEdit_editingFinished );

    _bedHeightHBox->addWidget( _bedHeightLabel    );
    _bedHeightHBox->addWidget( _bedHeightLineEdit );

    _okButton->setText( "OK" );
    _okButton->setAutoDefault( true );
    QObject::connect( _okButton, &QPushButton::clicked, this, &BedHeightAdjustmentDialog::_okButton_clicked );

    _cancelButton->setText( "Cancel" );
    QObject::connect( _cancelButton, &QPushButton::clicked, this, &BedHeightAdjustmentDialog::_cancelButton_clicked );

    _buttonsHBox->addWidget( _okButton );
    _buttonsHBox->addWidget( _cancelButton );

    _layout->addLayout( _bedHeightHBox );
    _layout->addLayout( _buttonsHBox );

    setLayout( _layout );
    setSizeGripEnabled( false );
}

BedHeightAdjustmentDialog::~BedHeightAdjustmentDialog( ) {
    debug( "+ BedHeightAdjustmentDialog::`dtor: destroying instance at %p\n", this );
}

void BedHeightAdjustmentDialog::_bedHeightLineEdit_editingFinished( ) {
    bool valueOk = false;
    double value = _bedHeightLineEdit->text( ).toDouble( &valueOk );
    if ( valueOk ) {
        _newBedHeight = value;
    } else {
        debug( "+ BedHeightAdjustmentDialog::_bedHeightLineEdit_editingFinished: bad value\n" );
    }
}

void BedHeightAdjustmentDialog::_okButton_clicked( bool /*checked*/ ) {
    accept( );
}

void BedHeightAdjustmentDialog::_cancelButton_clicked( bool /*checked*/ ) {
    reject( );
}
