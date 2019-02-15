#include "pch.h"

#include "printtab.h"

#include "printjob.h"
#include "bedheightadjustment.h"
#include "strings.h"

namespace {

    QStringList ExposureScaleFactorStringList { "1×", "2×", "3×", "4×", "5×" };
    double      ExposureScaleFactorValues[]   { 1.0,  2.0,  3.0,  4.0,  5.0  };

}

PrintTab::PrintTab( QWidget* parent ): QWidget( parent ) {
    exposureTime->setAlignment( Qt::AlignRight );
    exposureTime->setValidator( new QDoubleValidator( 0.0, 1.0E10, 10 ) );
    QObject::connect( exposureTime, &QLineEdit::editingFinished, this, &PrintTab::exposureTime_editingFinished );

    exposureTimeLabel->setText( "Exposure time (seconds):" );
    exposureTimeLabel->setBuddy( exposureTime );

    exposureScaleFactorComboBox->setEditable( false );
    exposureScaleFactorComboBox->setMaxVisibleItems( ExposureScaleFactorStringList.count( ) );
    exposureScaleFactorComboBox->addItems( ExposureScaleFactorStringList );
    QObject::connect( exposureScaleFactorComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &PrintTab::exposureScaleFactorComboBox_currentIndexChanged );

    exposureScaleFactorLabel->setText( "First layers time scale factor:" );
    exposureScaleFactorLabel->setBuddy( exposureScaleFactorComboBox );

    powerLevelSlider->setOrientation( Qt::Orientation::Horizontal );
    powerLevelSlider->setTickPosition( QSlider::TickPosition::TicksBelow );
    powerLevelSlider->setMinimum( 20 );
    powerLevelSlider->setMaximum( 100 );
    QObject::connect( powerLevelSlider, &QSlider::valueChanged, this, &PrintTab::powerLevelSlider_valueChanged );

    powerLevelLabel->setText( "Projector power level:" );
    powerLevelLabel->setBuddy( powerLevelSlider );

    powerLevelValue->setAlignment( Qt::AlignRight );
    powerLevelValue->setFrameShadow( QFrame::Sunken );
    powerLevelValue->setFrameStyle( QFrame::StyledPanel );

    powerLevelValueLayout->setContentsMargins( { } );
    powerLevelValueLayout->addWidget( powerLevelLabel );
    powerLevelValueLayout->addStretch( );
    powerLevelValueLayout->addWidget( powerLevelValue );

    powerLevelValueContainer->setContentsMargins( { } );
    powerLevelValueContainer->setLayout( powerLevelValueLayout );

    powerLevelSliderLeftLabel->setContentsMargins( { } );
    powerLevelSliderLeftLabel->setText( "20%" );
    powerLevelSliderLeftLabel->setAlignment( Qt::AlignLeft );

    powerLevelSliderRightLabel->setContentsMargins( { } );
    powerLevelSliderRightLabel->setText( "100%" );
    powerLevelSliderRightLabel->setAlignment( Qt::AlignRight );

    powerLevelSliderLabelsLayout->setContentsMargins( { } );
    powerLevelSliderLabelsLayout->addWidget( powerLevelSliderLeftLabel );
    powerLevelSliderLabelsLayout->addStretch( );
    powerLevelSliderLabelsLayout->addWidget( powerLevelSliderRightLabel );

    powerLevelSliderLabelsContainer->setContentsMargins( { } );
    powerLevelSliderLabelsContainer->setLayout( powerLevelSliderLabelsLayout );

    optionsLayout->setContentsMargins( { } );
    optionsLayout->addWidget( exposureTimeLabel );
    optionsLayout->addWidget( exposureTime );
    optionsLayout->addWidget( exposureScaleFactorLabel );
    optionsLayout->addWidget( exposureScaleFactorComboBox );
    optionsLayout->addWidget( powerLevelValueContainer );
    optionsLayout->addWidget( powerLevelSlider );
    optionsLayout->addWidget( powerLevelSliderLabelsContainer );
    optionsLayout->addStretch( );

    optionsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    optionsContainer->setLayout( optionsLayout );

    printButton->setText( "Print" );
    {
        auto font { printButton->font( ) };
        font.setPointSizeF( 22.25 );
        printButton->setFont( font );
    }
    printButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    printButton->setEnabled( false );
    QObject::connect( printButton, &QPushButton::clicked, this, &PrintTab::printButton_clicked );

    _adjustBedHeightButton->setText( "Adjust\nBed Height" );
    _adjustBedHeightButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QObject::connect( _adjustBedHeightButton, &QPushButton::clicked, this, &PrintTab::_adjustBedHeightButton_clicked );

    _adjustmentsHBox->addWidget( _adjustBedHeightButton );
    _adjustmentsHBox->addStretch( );

    _adjustmentsVBox->addLayout( _adjustmentsHBox );
    _adjustmentsVBox->addStretch( );

    _adjustmentsGroup->setTitle( QString( "Adjustments" ) );
    _adjustmentsGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _adjustmentsGroup->setMinimumSize( MaximalRightHandPaneSize );
    _adjustmentsGroup->setLayout( _adjustmentsVBox );

    _layout->setContentsMargins( { } );
    _layout->addWidget( optionsContainer,  0, 0, 1, 1 );
    _layout->addWidget( printButton,       1, 0, 1, 1 );
    _layout->addWidget( _adjustmentsGroup, 0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
}

PrintTab::~PrintTab( ) {
    /*empty*/
}

void PrintTab::exposureTime_editingFinished( ) {
    bool valueOk = false;
    double value = exposureTime->text( ).toDouble( &valueOk );
    if ( valueOk ) {
        _printJob->exposureTime = value;
    } else {
        debug( "+ PrintTab::exposureTime_editingFinished: bad value\n" );
    }
}

void PrintTab::exposureScaleFactorComboBox_currentIndexChanged( int index ) {
    _printJob->exposureTimeScaleFactor = ExposureScaleFactorValues[index];
}

void PrintTab::powerLevelSlider_valueChanged( int value ) {
    int scaledValue = ( value / 100.0 * 255.0 ) + 0.5;
    _printJob->powerLevel = scaledValue;
    powerLevelValue->setText( QString( "%1%" ).arg( value ) );
}

void PrintTab::printButton_clicked( bool /*checked*/ ) {
    debug( "+ PrintTab::printButton_clicked\n" );
    emit printButtonClicked( );
}

void PrintTab::_adjustBedHeightButton_clicked( bool /*checked*/ ) {
    debug( "+ PrintTab::_adjustBedHeightButton_clicked\n" );

    BedHeightAdjustmentDialog adjustDialog { this };
    int rc = adjustDialog.exec( );
    debug( "  + adjustDialog->exec returned %s [%d]\n", ToString( static_cast<QDialog::DialogCode>( rc ) ), rc );

    double newBedHeight = adjustDialog.newBedHeight( );
    debug( "  + new bed height: %f\n", newBedHeight );

    emit adjustBedHeight( newBedHeight );
}

void PrintTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ PrintTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;

    exposureTime->setText( FormatDouble( _printJob->exposureTime ) );

    auto exposureTimeScaleFactorText = FormatDouble( _printJob->exposureTimeScaleFactor ) + QString( "×" );
    int index = exposureScaleFactorComboBox->findText( exposureTimeScaleFactorText );
    if ( -1 == index ) {
        debug( "  + couldn't find exposureScaleFactorComboBox entry for %f => '%s'\n", _printJob->exposureTimeScaleFactor, exposureTimeScaleFactorText.toUtf8( ).data( ) );
    } else {
        exposureScaleFactorComboBox->setCurrentIndex( index );
    }

    int scaledValue = ( _printJob->powerLevel / 255.0 * 100.0 ) + 0.5;
    debug( "  + power level: %d => %d\n", _printJob->powerLevel, scaledValue );
    powerLevelSlider->setValue( scaledValue );
    powerLevelValue->setText( QString( "%1%" ).arg( scaledValue ) );
}

void PrintTab::setPrintButtonEnabled( bool const value ) {
    debug( "+ PrintTab::setPrintButtonEnabled: value %s\n", value ? "enabled" : "disabled" );
    printButton->setEnabled( value );
}
