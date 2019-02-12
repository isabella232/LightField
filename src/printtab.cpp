#include "pch.h"

#include "printtab.h"

#include "printjob.h"

namespace {

    QStringList ExposureScaleFactorStringList { "1×", "2×", "3×", "4×", "5×" };
    double      ExposureScaleFactorValues[]   { 1.0,  2.0,  3.0,  4.0,  5.0  };

}

PrintTab::PrintTab( QWidget* parent ): QWidget( parent ) {
    exposureTime->setAlignment( Qt::AlignRight );
    exposureTime->setText( "1.0" );
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
    powerLevelSlider->setValue( 50 );
    QObject::connect( powerLevelSlider, &QSlider::valueChanged, this, &PrintTab::powerLevelSlider_valueChanged );

    powerLevelLabel->setText( "Projector power level:" );
    powerLevelLabel->setBuddy( powerLevelSlider );

    powerLevelValue->setText( "50%" );
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

    _placeHolder->setContentsMargins( { } );
    _placeHolder->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _placeHolder->setMinimumSize( 600, 400 );

    _layout = new QGridLayout;
    _layout->setContentsMargins( { } );
    _layout->addWidget( optionsContainer, 0, 0, 1, 1 );
    _layout->addWidget( printButton,      1, 0, 1, 1 );
    _layout->addWidget( _placeHolder,     0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
}

PrintTab::~PrintTab( ) {
    /*empty*/
}

void PrintTab::exposureTime_editingFinished( ) {
    bool valueOk = false;
    double value = exposureTime->validator( )->locale( ).toDouble( exposureTime->text( ), &valueOk );
    if ( valueOk ) {
        debug( "+ PrintTab::exposureTime_editingFinished: new value %f\n", value );
        _printJob->exposureTime = value;
    } else {
        debug( "+ PrintTab::exposureTime_editingFinished: bad value\n" );
    }
}

void PrintTab::exposureScaleFactorComboBox_currentIndexChanged( int index ) {
    debug( "+ PrintTab::exposureScaleFactorComboBox_currentIndexChanged: new value: %f×\n", ExposureScaleFactorValues[index] );
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

void PrintTab::setPrintJob( PrintJob* printJob ) {
    _printJob = printJob;
}
