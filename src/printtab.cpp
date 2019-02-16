#include "pch.h"

#include "printtab.h"

#include "printjob.h"
#include "bedheightadjustment.h"
#include "strings.h"

namespace {

    QStringList ExposureTimeScaleFactorStringList { "1×", "2×", "3×", "4×", "5×" };
    double      ExposureTimeScaleFactorValues[]   { 1.0,  2.0,  3.0,  4.0,  5.0  };

    char const* BuildPlatformStateStrings[] { "Extended", "Retracting", "Retracted", "Extending" };

    char const* ToString( BuildPlatformState const value ) {
#if defined _DEBUG
        if ( ( value >= BuildPlatformState::Extended ) && ( value <= BuildPlatformState::Extending ) ) {
#endif
            return BuildPlatformStateStrings[static_cast<int>( value )];
#if defined _DEBUG
        } else {
            return nullptr;
        }
#endif
    }

}

PrintTab::PrintTab( QWidget* parent ): QWidget( parent ) {
    exposureTimeDial->setMinimum(  1 );
    exposureTimeDial->setMaximum( 40 );
    exposureTimeDial->setNotchesVisible( true );
    exposureTimeDial->setWrapping( false );
    QObject::connect( exposureTimeDial, &QDial::valueChanged, this, &PrintTab::exposureTimeDial_valueChanged );

    exposureTimeLabel->setText( "Exposure time (seconds):" );
    exposureTimeLabel->setBuddy( exposureTimeDial );

    exposureTimeValue->setAlignment( Qt::AlignRight );
    exposureTimeValue->setFrameShadow( QFrame::Sunken );
    exposureTimeValue->setFrameStyle( QFrame::StyledPanel );

    exposureTimeValueLayout->setContentsMargins( { } );
    exposureTimeValueLayout->addWidget( exposureTimeLabel );
    exposureTimeValueLayout->addStretch( );
    exposureTimeValueLayout->addWidget( exposureTimeValue );

    exposureTimeValueContainer->setContentsMargins( { } );
    exposureTimeValueContainer->setLayout( exposureTimeValueLayout );

    exposureTimeDialLeftLabel->setContentsMargins( { } );
    exposureTimeDialLeftLabel->setText( "0.5 s" );
    exposureTimeDialLeftLabel->setAlignment( Qt::AlignLeft );

    exposureTimeDialRightLabel->setContentsMargins( { } );
    exposureTimeDialRightLabel->setText( "20 s" );
    exposureTimeDialRightLabel->setAlignment( Qt::AlignRight );

    exposureTimeDialLabelsLayout->setContentsMargins( { } );
    exposureTimeDialLabelsLayout->addStretch( );
    exposureTimeDialLabelsLayout->addWidget( exposureTimeDialLeftLabel );
    exposureTimeDialLabelsLayout->addStretch( );
    exposureTimeDialLabelsLayout->addWidget( exposureTimeDialRightLabel );
    exposureTimeDialLabelsLayout->addStretch( );

    exposureTimeDialLabelsContainer->setContentsMargins( { } );
    exposureTimeDialLabelsContainer->setLayout( exposureTimeDialLabelsLayout );

    exposureTimeScaleFactorComboBox->setEditable( false );
    exposureTimeScaleFactorComboBox->setMaxVisibleItems( ExposureTimeScaleFactorStringList.count( ) );
    exposureTimeScaleFactorComboBox->addItems( ExposureTimeScaleFactorStringList );
    QObject::connect( exposureTimeScaleFactorComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &PrintTab::exposureTimeScaleFactorComboBox_currentIndexChanged );

    exposureTimeScaleFactorLabel->setText( "First layers time scale factor:" );
    exposureTimeScaleFactorLabel->setBuddy( exposureTimeScaleFactorComboBox );

    powerLevelDial->setMinimum( 20 );
    powerLevelDial->setMaximum( 100 );
    powerLevelDial->setNotchesVisible( true );
    powerLevelDial->setWrapping( false );
    QObject::connect( powerLevelDial, &QDial::valueChanged, this, &PrintTab::powerLevelDial_valueChanged );

    powerLevelLabel->setText( "Projector power level:" );
    powerLevelLabel->setBuddy( powerLevelDial );

    powerLevelValue->setAlignment( Qt::AlignRight );
    powerLevelValue->setFrameShadow( QFrame::Sunken );
    powerLevelValue->setFrameStyle( QFrame::StyledPanel );

    powerLevelValueLayout->setContentsMargins( { } );
    powerLevelValueLayout->addWidget( powerLevelLabel );
    powerLevelValueLayout->addStretch( );
    powerLevelValueLayout->addWidget( powerLevelValue );

    powerLevelValueContainer->setContentsMargins( { } );
    powerLevelValueContainer->setLayout( powerLevelValueLayout );

    powerLevelDialLeftLabel->setContentsMargins( { } );
    powerLevelDialLeftLabel->setText( "20%" );
    powerLevelDialLeftLabel->setAlignment( Qt::AlignLeft );

    powerLevelDialRightLabel->setContentsMargins( { } );
    powerLevelDialRightLabel->setText( "100%" );
    powerLevelDialRightLabel->setAlignment( Qt::AlignRight );

    powerLevelDialLabelsLayout->setContentsMargins( { } );
    powerLevelDialLabelsLayout->addStretch( );
    powerLevelDialLabelsLayout->addWidget( powerLevelDialLeftLabel );
    powerLevelDialLabelsLayout->addStretch( );
    powerLevelDialLabelsLayout->addWidget( powerLevelDialRightLabel );
    powerLevelDialLabelsLayout->addStretch( );

    powerLevelDialLabelsContainer->setContentsMargins( { } );
    powerLevelDialLabelsContainer->setLayout( powerLevelDialLabelsLayout );

    optionsLayout->setContentsMargins( { } );
    optionsLayout->addWidget( exposureTimeValueContainer );
    optionsLayout->addWidget( exposureTimeDial );
    optionsLayout->addWidget( exposureTimeDialLabelsContainer );
    optionsLayout->addWidget( exposureTimeScaleFactorLabel );
    optionsLayout->addWidget( exposureTimeScaleFactorComboBox );
    optionsLayout->addWidget( powerLevelValueContainer );
    optionsLayout->addWidget( powerLevelDial );
    optionsLayout->addWidget( powerLevelDialLabelsContainer );
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

    _retractOrExtendButton->setText( "Retract\nBuild Platform" );
    _retractOrExtendButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QObject::connect( _retractOrExtendButton, &QPushButton::clicked, this, &PrintTab::_retractOrExtendButton_clicked );

    _moveUpButton->setText( "Move Up\n100 µm" );
    _moveUpButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QObject::connect( _moveUpButton, &QPushButton::clicked, this, &PrintTab::_moveUpButton_clicked );

    _moveDownButton->setText( "Move Down\n100 µm" );
    _moveDownButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QObject::connect( _moveDownButton, &QPushButton::clicked, this, &PrintTab::_moveDownButton_clicked );

    _adjustmentsHBox->addWidget( _adjustBedHeightButton );
    _adjustmentsHBox->addStretch( );
    _adjustmentsHBox->addWidget( _retractOrExtendButton );
    _adjustmentsHBox->addStretch( );
    _adjustmentsHBox->addWidget( _moveUpButton );
    _adjustmentsHBox->addWidget( _moveDownButton );

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

void PrintTab::exposureTimeDial_valueChanged( int value ) {
    _printJob->exposureTime = value / 2.0;
    exposureTimeValue->setText( QString( "%1 s" ).arg( _printJob->exposureTime, 0, 'f', 1 ) );
}

void PrintTab::exposureTimeScaleFactorComboBox_currentIndexChanged( int index ) {
    _printJob->exposureTimeScaleFactor = ExposureTimeScaleFactorValues[index];
}

void PrintTab::powerLevelDial_valueChanged( int value ) {
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
    setAdjustmentButtonsEnabled( false );

    BedHeightAdjustmentDialog adjustDialog { this };
    int rc = adjustDialog.exec( );
    debug( "  + adjustDialog->exec returned %s [%d]\n", ToString( static_cast<QDialog::DialogCode>( rc ) ), rc );

    double newBedHeight = adjustDialog.newBedHeight( );
    debug( "  + new bed height: %f\n", newBedHeight );

    emit adjustBedHeight( newBedHeight );
}

void PrintTab::adjustBedHeightComplete( bool const success ) {
    debug( "+ PrintTab::adjustBedHeightComplete: %s\n", success ? "succeeded" : "failed" );
    setAdjustmentButtonsEnabled( true );
}

void PrintTab::_retractOrExtendButton_clicked( bool /*checked*/ ) {
    debug( "+ PrintTab::_retractOrExtendButton_clicked: _buildPlatformState %s [%d]\n", ToString( _buildPlatformState ), _buildPlatformState );
    setAdjustmentButtonsEnabled( false );

    switch ( _buildPlatformState ) {
        case BuildPlatformState::Extended:
            _buildPlatformState = BuildPlatformState::Retracting;
            emit retractBuildPlatform( );
            break;

        case BuildPlatformState::Retracted:
            _buildPlatformState = BuildPlatformState::Extending;
            emit extendBuildPlatform( );
            break;

        default:
            return;
    }
}

void PrintTab::retractBuildPlatformComplete( bool const success ) {
    debug( "+ PrintTab::retractBuildPlatformComplete: %s\n", success ? "succeeded" : "failed" );
    _buildPlatformState = BuildPlatformState::Retracted;
    _retractOrExtendButton->setText( "Extend\nBuild Platform" );
    _retractOrExtendButton->setEnabled( true );
}

void PrintTab::extendBuildPlatformComplete( bool const success ) {
    debug( "+ PrintTab::extendBuildPlatformComplete: %s\n", success ? "succeeded" : "failed" );
    _buildPlatformState = BuildPlatformState::Extended;
    _retractOrExtendButton->setText( "Retract\nBuild Platform" );
    setAdjustmentButtonsEnabled( true );
}

void PrintTab::_moveUpButton_clicked( bool /*checked*/ ) {
    setAdjustmentButtonsEnabled( false );
    emit moveBuildPlatformUp( );
}

void PrintTab::moveBuildPlatformUpComplete( bool const success ) {
    debug( "+ PrintTab::moveBuildPlatformUpComplete: %s\n", success ? "succeeded" : "failed" );
    setAdjustmentButtonsEnabled( true );
}

void PrintTab::_moveDownButton_clicked( bool /*checked*/ ) {
    setAdjustmentButtonsEnabled( false );
    emit moveBuildPlatformDown( );
}

void PrintTab::moveBuildPlatformDownComplete( bool const success ) {
    debug( "+ PrintTab::moveBuildPlatformDownComplete: %s\n", success ? "succeeded" : "failed" );
    setAdjustmentButtonsEnabled( true );
}

void PrintTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ PrintTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;

    int value = _printJob->exposureTime / 0.5;
    _printJob->exposureTime = value / 2.0;
    exposureTimeDial->setValue( value );

    auto exposureTimeScaleFactorText = FormatDouble( _printJob->exposureTimeScaleFactor ) + QString( "×" );
    int index = exposureTimeScaleFactorComboBox->findText( exposureTimeScaleFactorText );
    if ( -1 == index ) {
        debug( "  + couldn't find exposureTimeScaleFactorComboBox entry for %f => '%s'\n", _printJob->exposureTimeScaleFactor, exposureTimeScaleFactorText.toUtf8( ).data( ) );
    } else {
        exposureTimeScaleFactorComboBox->setCurrentIndex( index );
    }

    powerLevelDial->setValue( _printJob->powerLevel / 255.0 * 100.0 + 0.5 );
}

void PrintTab::setPrintButtonEnabled( bool const value ) {
    debug( "+ PrintTab::setPrintButtonEnabled: value %s\n", value ? "enabled" : "disabled" );
    printButton->setEnabled( value );
}

void PrintTab::setAdjustmentButtonsEnabled( bool const value ) {
    debug( "+ PrintTab::setAdjustmentButtonsEnabled: value %s\n", value ? "enabled" : "disabled" );
    _adjustBedHeightButton->setEnabled( value );
    _retractOrExtendButton->setEnabled( value );
    _moveUpButton         ->setEnabled( value );
    _moveDownButton       ->setEnabled( value );
}
