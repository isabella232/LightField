#include "pch.h"

#include "advancedtab.h"

#include "pngdisplayer.h"
#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"

namespace {

    auto DefaultPrintBedTemperature = 65;

}

AdvancedTab::AdvancedTab( QWidget* parent ): TabBase( parent ) {
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome", LargeFontSize );


    _currentTemperatureLabel->setText( "Current temperature:" );
    _targetTemperatureLabel ->setText( "Target temperature:"  );
    _heatingElementLabel    ->setText( "Heating element:"     );
    _zPositionLabel         ->setText( "Z position:"          );


    _currentTemperature->setAlignment( Qt::AlignRight );
    _currentTemperature->setFont( boldFont );
    _currentTemperature->setText( EmDash );

    _targetTemperature ->setAlignment( Qt::AlignRight );
    _targetTemperature ->setFont( boldFont );
    _targetTemperature ->setText( EmDash );

    _heatingElement    ->setAlignment( Qt::AlignRight );
    _heatingElement    ->setFont( boldFont );
    _heatingElement    ->setText( EmDash );

    _zPosition         ->setAlignment( Qt::AlignRight );
    _zPosition         ->setFont( boldFont );
    _zPosition         ->setText( EmDash );


    _leftColumn->setContentsMargins( { } );
    _leftColumn->setFixedWidth( MainButtonSize.width( ) );
    _leftColumn->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _leftColumn->setLayout( WrapWidgetsInVBox(
        WrapWidgetsInHBox( _currentTemperatureLabel, nullptr, _currentTemperature ),
        WrapWidgetsInHBox( _targetTemperatureLabel,  nullptr, _targetTemperature  ),
        WrapWidgetsInHBox( _heatingElementLabel,     nullptr, _heatingElement     ),
        WrapWidgetsInHBox( _zPositionLabel,          nullptr, _zPosition          ),
        nullptr
    ) );


    _offsetLabel->setText( "Build platform offset:" );

    _offsetValue->setAlignment( Qt::AlignRight );
    _offsetValue->setFont( boldFont );
    _offsetValue->setText( QString { "%1 µm" }.arg( g_settings.buildPlatformOffset ) );

    _offsetSlider->setMinimum( 0 );
    _offsetSlider->setMaximum( 40 );
    _offsetSlider->setOrientation( Qt::Horizontal );
    _offsetSlider->setPageStep( 4 );
    _offsetSlider->setSingleStep( 1 );
    _offsetSlider->setTickInterval( 4 );
    _offsetSlider->setTickPosition( QSlider::TicksBothSides );
    _offsetSlider->setValue( g_settings.buildPlatformOffset / 25 );
    QObject::connect( _offsetSlider, &QSlider::sliderReleased, this, &AdvancedTab::offsetSlider_sliderReleased );
    QObject::connect( _offsetSlider, &QSlider::valueChanged,   this, &AdvancedTab::offsetSlider_valueChanged   );


    _buildPlatformOffsetGroup->setContentsMargins( { } );
    _buildPlatformOffsetGroup->setLayout( WrapWidgetsInVBoxDM(
        WrapWidgetsInHBox( _offsetLabel, nullptr, _offsetValue ),
        _offsetSlider
    ) );


    _bedHeatingButton->setCheckable( true );
    _bedHeatingButton->setChecked( false );
    _bedHeatingButton->setFont( fontAwesome );
    _bedHeatingButton->setFixedSize( 37, 38 );
    _bedHeatingButton->setText( FA_Times );
    QObject::connect( _bedHeatingButton, &QPushButton::clicked, this, &AdvancedTab::printBedHeatingButton_clicked );

    _bedHeatingButtonLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    _bedHeatingButtonLabel->setText( "Print bed heating" );

#if defined ENABLE_TEMPERATURE_SETTING
    _bedTemperatureLabel->setEnabled( false );
    _bedTemperatureLabel->setText( "Print bed temperature:" );

    _bedTemperatureValue->setAlignment( Qt::AlignRight );
    _bedTemperatureValue->setEnabled( false );
    _bedTemperatureValue->setFont( boldFont );
    _bedTemperatureValue->setText( QString { "%1 °C" }.arg( DefaultPrintBedTemperature ) );

    _bedTemperatureValueLayout = WrapWidgetsInHBox( _bedTemperatureLabel, nullptr, _bedTemperatureValue );
    _bedTemperatureValueLayout->setEnabled( false );

    _bedTemperatureSlider->setEnabled( false );
    _bedTemperatureSlider->setMinimum( 30 );
    _bedTemperatureSlider->setMaximum( 75 );
    _bedTemperatureSlider->setOrientation( Qt::Horizontal );
    _bedTemperatureSlider->setPageStep( 1 );
    _bedTemperatureSlider->setSingleStep( 1 );
    _bedTemperatureSlider->setTickInterval( 5 );
    _bedTemperatureSlider->setTickPosition( QSlider::TicksBothSides );
    _bedTemperatureSlider->setValue( DefaultPrintBedTemperature );
    QObject::connect( _bedTemperatureSlider, &QSlider::sliderReleased, this, &AdvancedTab::printBedTemperatureSlider_sliderReleased );
    QObject::connect( _bedTemperatureSlider, &QSlider::valueChanged,   this, &AdvancedTab::printBedTemperatureSlider_valueChanged   );
#endif

    auto bedTemperatureLayout = WrapWidgetsInVBoxDM(
        WrapWidgetsInHBox( _bedHeatingButton, _bedHeatingButtonLabel, nullptr )
    );
#if defined ENABLE_TEMPERATURE_SETTING
    bedTemperatureLayout->addLayout( _bedTemperatureValueLayout );
    bedTemperatureLayout->addWidget( _bedTemperatureSlider );
#endif

    _bedHeatingGroup->setContentsMargins( { } );
    _bedHeatingGroup->setLayout( bedTemperatureLayout );


    _projectBlankImageButton->setCheckable( true );
    _projectBlankImageButton->setChecked( false );
    _projectBlankImageButton->setFont( fontAwesome );
    _projectBlankImageButton->setFixedSize( 37, 38 );
    _projectBlankImageButton->setText( FA_Times );
    QObject::connect( _projectBlankImageButton, &QPushButton::clicked, this, &AdvancedTab::projectBlankImageButton_clicked );

    _projectBlankImageButtonLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    _projectBlankImageButtonLabel->setText( "Project blank image" );

    _projectFocusImageButton->setCheckable( true );
    _projectFocusImageButton->setChecked( false );
    _projectFocusImageButton->setFont( fontAwesome );
    _projectFocusImageButton->setFixedSize( 37, 38 );
    _projectFocusImageButton->setText( FA_Times );
    QObject::connect( _projectFocusImageButton, &QPushButton::clicked, this, &AdvancedTab::projectFocusImageButton_clicked );

    _projectFocusImageButtonLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    _projectFocusImageButtonLabel->setText( "Project focus image" );

    _powerLevelLabel->setText( "Projector power level:" );

    _powerLevelValue->setAlignment( Qt::AlignRight );
    _powerLevelValue->setFont( boldFont );
    _powerLevelValue->setText( "50%" );

    _powerLevelValueLayout = WrapWidgetsInHBox( _powerLevelLabel, nullptr, _powerLevelValue );

    _powerLevelSlider->setEnabled( false );
    _powerLevelSlider->setMinimum( ProjectorMinPercent );
    _powerLevelSlider->setMaximum( ProjectorMaxPercent );
    _powerLevelSlider->setOrientation( Qt::Horizontal );
    _powerLevelSlider->setPageStep( 5 );
    _powerLevelSlider->setSingleStep( 1 );
    _powerLevelSlider->setTickInterval( 5 );
    _powerLevelSlider->setTickPosition( QSlider::TicksBothSides );
    _powerLevelSlider->setValue( 50 );
    QObject::connect( _powerLevelSlider, &QSlider::sliderReleased, this, &AdvancedTab::powerLevelSlider_sliderReleased );
    QObject::connect( _powerLevelSlider, &QSlider::valueChanged,   this, &AdvancedTab::powerLevelSlider_valueChanged   );

    _powerLevelLabel->setEnabled( false );
    _powerLevelSlider->setEnabled( false );
    _powerLevelValue->setEnabled( false );
    _powerLevelValueLayout->setEnabled( false );


    _projectImageButtonsGroup->setContentsMargins( { } );
    _projectImageButtonsGroup->setLayout( WrapWidgetsInVBoxDM(
        WrapWidgetsInHBox( _projectBlankImageButton, _projectBlankImageButtonLabel, nullptr, _projectFocusImageButton, _projectFocusImageButtonLabel, nullptr ),
        _powerLevelValueLayout,
        _powerLevelSlider
    ) );


    _rightColumn->setContentsMargins( { } );
    _rightColumn->setMinimumSize( MaximalRightHandPaneSize );
    _rightColumn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _rightColumn->setLayout( WrapWidgetsInVBoxDM(
        _buildPlatformOffsetGroup,
        _bedHeatingGroup,
        _projectImageButtonsGroup,
        nullptr
    ) );


    setLayout( WrapWidgetsInHBox( _leftColumn, _rightColumn ) );
}

AdvancedTab::~AdvancedTab( ) {
    /*empty*/
}

void AdvancedTab::_connectShepherd( ) {
    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_online,            this, &AdvancedTab::printer_online            );
        QObject::connect( _shepherd, &Shepherd::printer_offline,           this, &AdvancedTab::printer_offline           );
        QObject::connect( _shepherd, &Shepherd::printer_positionReport,    this, &AdvancedTab::printer_positionReport    );
        QObject::connect( _shepherd, &Shepherd::printer_temperatureReport, this, &AdvancedTab::printer_temperatureReport );
    }
}

void AdvancedTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ AdvancedTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch ( _uiState ) {
        case UiState::SelectStarted:
        case UiState::SelectCompleted:
        case UiState::SliceStarted:
        case UiState::SliceCompleted:
            break;

        case UiState::PrintStarted:
            setPrinterAvailable( false );
            emit printerAvailabilityChanged( false );
            break;

        case UiState::PrintCompleted:
            setPrinterAvailable( true );
            emit printerAvailabilityChanged( true );
            break;

        case UiState::SelectedDirectory:
            break;
    }
}

void AdvancedTab::printer_positionReport( double const px, int const cx ) {
    debug( "+ AdvancedTab::printer_positionReport: px %.2f mm, cx %d\n", px, cx );
    _zPosition->setText( QString { "%1 mm" }.arg( px, 0, 'f', 2 ) );

    update( );
}

void AdvancedTab::printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm ) {
    _currentTemperature->setText( QString( "%1 °C" ).arg( bedCurrentTemperature, 0, 'f', 2 ) );
    _targetTemperature ->setText( QString( "%1 °C" ).arg( bedTargetTemperature,  0, 'f', 2 ) );
    _heatingElement    ->setText( bedPwm ? "ON" : "off"                                      );

    update( );
}

void AdvancedTab::offsetSlider_sliderReleased( ) {
    auto offset = _offsetSlider->value( ) * 25;
    debug( "+ AdvancedTab::offsetSlider_sliderReleased: new value %d µm\n", offset );
    g_settings.buildPlatformOffset = offset;
}

void AdvancedTab::offsetSlider_valueChanged( int value ) {
    debug( "+ AdvancedTab::offsetSlider_valueChanged: new value %d µm\n", value * 25 );
    _offsetValue->setText( QString { "%1 µm" }.arg( value * 25 ) );

    update( );
}

void AdvancedTab::printBedHeatingButton_clicked( bool checked ) {
    _bedHeatingButton->setText( checked ? FA_Check : FA_Times );
#if defined ENABLE_TEMPERATURE_SETTING
    _bedTemperatureLabel->setEnabled( checked );
    _bedTemperatureSlider->setEnabled( checked );
    _bedTemperatureValue->setEnabled( checked );
    _bedTemperatureValueLayout->setEnabled( checked );
#endif

    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &AdvancedTab::shepherd_sendComplete );

#if defined ENABLE_TEMPERATURE_SETTING
    _shepherd->doSend( QString { "M140 S%1" }.arg( checked ? _bedTemperatureSlider->value( ) : 0 ) );
#else
    _shepherd->doSend( QString { "M140 S%1" }.arg( checked ? DefaultPrintBedTemperature : 0 ) );
#endif

    update( );
}

#if defined ENABLE_TEMPERATURE_SETTING
void AdvancedTab::printBedTemperatureSlider_sliderReleased( ) {
    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &AdvancedTab::shepherd_sendComplete );
    _shepherd->doSend( QString { "M140 S%1" }.arg( _bedTemperatureSlider->value( ) ) );
}

void AdvancedTab::printBedTemperatureSlider_valueChanged( int value ) {
    debug( "+ AdvancedTab::printBedTemperatureSlider_valueChanged: new value %d °C\n", value );
    _bedTemperatureValue->setText( QString { "%1 °C" }.arg( value ) );

    update( );
}
#endif

void AdvancedTab::_projectImage( char const* fileName ) {
    _powerLevelLabel      ->setEnabled( _isProjectorOn );
    _powerLevelSlider     ->setEnabled( _isProjectorOn );
    _powerLevelValue      ->setEnabled( _isProjectorOn );
    _powerLevelValueLayout->setEnabled( _isProjectorOn );

    if ( _isProjectorOn ) {
        _pngDisplayer->loadImageFile( fileName );
    } else {
        _pngDisplayer->clear( );
    }

    QProcess::startDetached( SetProjectorPowerCommand, { QString { "%1" }.arg( _isProjectorOn ? PercentagePowerLevelToRawLevel( _powerLevelSlider->value( ) ) : 0 ) } );

    setPrinterAvailable( !_isProjectorOn );
    emit printerAvailabilityChanged( _isPrinterAvailable );

    update( );
}

void AdvancedTab::projectBlankImageButton_clicked( bool checked ) {
    debug( "+ AdvancedTab::projectBlankImageButton_clicked: checked? %s\n", ToString( checked ) );
    _isProjectorOn = checked;
    _projectBlankImageButton->setText( _isProjectorOn ? FA_Check : FA_Times );
    if ( checked ) {
        _projectFocusImageButton->setChecked( false );
        _projectFocusImageButton->setText( FA_Times );
    }
    _projectImage( ":images/white-field.png" );
}

void AdvancedTab::projectFocusImageButton_clicked( bool checked ) {
    debug( "+ AdvancedTab::projectFocusImageButton_clicked: checked? %s\n", ToString( checked ) );
    _isProjectorOn = checked;
    _projectFocusImageButton->setText( _isProjectorOn ? FA_Check : FA_Times );
    if ( checked ) {
        _projectBlankImageButton->setChecked( false );
        _projectBlankImageButton->setText( FA_Times );
    }
    _projectImage( ":images/focus-image.png" );
}

void AdvancedTab::powerLevelSlider_sliderReleased( ) {
    QProcess::startDetached( SetProjectorPowerCommand, { QString { "%1" }.arg( _isProjectorOn ? PercentagePowerLevelToRawLevel( _powerLevelSlider->value( ) ) : 0 ) } );

    emit projectorPowerLevelChanged( _powerLevelSlider->value( ) );
}

void AdvancedTab::powerLevelSlider_valueChanged( int percentage ) {
    _printJob->powerLevel = PercentagePowerLevelToRawLevel( percentage );
    _powerLevelValue->setText( QString { "%1%" }.arg( percentage ) );

    update( );
}

void AdvancedTab::projectorPowerLevel_changed( int const percentage ) {
    _powerLevelSlider->setValue( percentage );
    _powerLevelValue->setText( QString( "%1%" ).arg( percentage ) );

    update( );
}

void AdvancedTab::shepherd_sendComplete( bool const success ) {
    debug( "+ AdvancedTab::shepherd_sendComplete: action %s\n", SucceededString( success ) );
    QObject::disconnect( _shepherd, &Shepherd::action_sendComplete, this, &AdvancedTab::shepherd_sendComplete );
}

void AdvancedTab::_updateControlGroups( ) {
    _bedHeatingGroup         ->setEnabled(                     _isPrinterOnline && _isPrinterAvailable && ( _shepherd != nullptr )                                   );
    _projectImageButtonsGroup->setEnabled( _isProjectorOn || ( _isPrinterOnline && _isPrinterAvailable && ( _shepherd != nullptr ) && ( _pngDisplayer != nullptr ) ) );

    update( );
}

void AdvancedTab::printer_online( ) {
    _isPrinterOnline = true;
    debug( "+ AdvancedTab::printer_online: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updateControlGroups( );
}

void AdvancedTab::printer_offline( ) {
    _isPrinterOnline = false;
    debug( "+ AdvancedTab::printer_offline: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updateControlGroups( );
}

void AdvancedTab::setPngDisplayer( PngDisplayer* pngDisplayer ) {
    _pngDisplayer = pngDisplayer;
}

void AdvancedTab::setPrinterAvailable( bool const value ) {
    _isPrinterAvailable = value;
    debug( "+ AdvancedTab::setPrinterAvailable: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updateControlGroups( );
}
