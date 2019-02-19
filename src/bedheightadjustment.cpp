#include "pch.h"

#include "bedheightadjustment.h"

#include "app.h"
#include "strings.h"

BedHeightAdjustmentDialog::BedHeightAdjustmentDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ) {
    debug( "+ BedHeightAdjustmentDialog::`ctor: construct at %p\n", this );

    _bedHeightLabel->setText( "Bed height:" );
    _bedHeightLabel->setBuddy( _bedHeightDial );

    _bedHeightValue->setAlignment( Qt::AlignRight );
    _bedHeightValue->setFrameShadow( QFrame::Sunken );
    _bedHeightValue->setFrameStyle( QFrame::StyledPanel );
    _bedHeightValue->setText( "0.000 mm" );

    _bedHeightLabelLayout->addWidget( _bedHeightLabel );
    _bedHeightLabelLayout->addWidget( _bedHeightValue );

    _bedHeightDial->setMinimum( -1000 * PrinterMaximumHeight );
    _bedHeightDial->setMaximum(  1000 * PrinterMaximumHeight );
    _bedHeightDial->setNotchesVisible( true );
    _bedHeightDial->setSingleStep( 100 );
    _bedHeightDial->setPageStep( 1000 );
    _bedHeightDial->setNotchTarget( 1 );
    _bedHeightDial->setWrapping( false );
    _bedHeightDial->setValue( 0 );
    QObject::connect( _bedHeightDial, &QDial::valueChanged, this, &BedHeightAdjustmentDialog::_bedHeightDial_valueChanged );

    _bedHeightDialTopLabel  ->setAlignment( Qt::AlignHCenter );
    _bedHeightDialLeftLabel ->setAlignment( Qt::AlignLeft    );
    _bedHeightDialRightLabel->setAlignment( Qt::AlignRight   );

    _bedHeightDialTopLabel  ->setText( QString( "0.000" ) );
    _bedHeightDialLeftLabel ->setText( QString(    "%1" ).arg( -PrinterMaximumHeight, 0, 'f', 3 ) );
    _bedHeightDialRightLabel->setText( QString(    "%1" ).arg(  PrinterMaximumHeight, 0, 'f', 3 ) );

    _bedHeightDialLowerLabelsLayout->addStretch( );
    _bedHeightDialLowerLabelsLayout->addWidget( _bedHeightDialLeftLabel );
    _bedHeightDialLowerLabelsLayout->addStretch( );
    _bedHeightDialLowerLabelsLayout->addWidget( _bedHeightDialRightLabel );
    _bedHeightDialLowerLabelsLayout->addStretch( );

    _okButton->setAutoDefault( true );
    _okButton->setText( "OK" );
    _cancelButton->setText( "Cancel" );

    QObject::connect( _okButton,     &QPushButton::clicked, this, &BedHeightAdjustmentDialog::accept );
    QObject::connect( _cancelButton, &QPushButton::clicked, this, &BedHeightAdjustmentDialog::reject );

    _buttonsHBox->addWidget( _okButton );
    _buttonsHBox->addWidget( _cancelButton );

    _layout->addLayout( _bedHeightLabelLayout );
    _layout->addWidget( _bedHeightDialTopLabel );
    _layout->addWidget( _bedHeightDial );
    _layout->addLayout( _bedHeightDialLowerLabelsLayout );
    _layout->addLayout( _buttonsHBox );

    //_frame->setContentsMargins( 2, 2, 2, 2 );
    //_frame->setFrameRect( QRect( 0, 0, width( ), height( ) ) );
    _frame->setFrameShape( QFrame::StyledPanel );
    _frame->setFrameStyle( QFrame::Raised );
    _frame->setLineWidth( 1 );
    _frame->setMidLineWidth( 3 );
    _frame->setLayout( _layout );
    _dialogLayout->addWidget( _frame );
    setLayout( _dialogLayout );

    setSizeGripEnabled( false );
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
}

BedHeightAdjustmentDialog::~BedHeightAdjustmentDialog( ) {
    debug( "+ BedHeightAdjustmentDialog::`dtor: destruct at %p\n", this );
}

void BedHeightAdjustmentDialog::resizeEvent( QResizeEvent* event ) {
    auto rc = rect( );
    debug(
        "+ BedHeightAdjustmentDialog::resizeEvent:\n"
        "  + spontaneous? %s\n"
        "  + old size:    %d×%d\n"
        "  + new size:    %d×%d\n"
        "  + rect:        (%d,%d)-(%d,%d) %d×%d\n"
        "  + status:      %s\n"
        "",
        ToString( event->spontaneous( ) ),
        event->oldSize( ).width( ), event->oldSize( ).height( ),
        event->   size( ).width( ), event->   size( ).height( ),
        rc.left( ),  rc.top( ),
        rc.right( ), rc.bottom( ),
        rc.width( ), rc.height( ),
        event->isAccepted( ) ? "accepted" : "ignored"
    );

    QDialog::resizeEvent( event );
    event->ignore( );
}

void BedHeightAdjustmentDialog::_bedHeightDial_valueChanged( int value ) {
    debug( "+ BedHeightAdjustmentDialog::_bedHeightDial_valueChanged: new value %6.3f\n", value / 1000.0 );
    _newBedHeight = value / 1000.0;
    _bedHeightValue->setText( QString( "%1 mm" ).arg( _newBedHeight, 7, 'f', 3 ) );
}
