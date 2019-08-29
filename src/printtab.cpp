#include "pch.h"

#include "printtab.h"

#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"

namespace {

    QString const RaiseBuildPlatformText { "Raise build platform" };
    QString const LowerBuildPlatformText { "Lower build platform" };

}

PrintTab::PrintTab( QWidget* parent ): InitialShowEventMixin<PrintTab, TabBase>( parent ) {
#if defined _DEBUG
    _isPrinterPrepared = g_settings.pretendPrinterIsPrepared;
#endif // _DEBUG

    auto boldFont = ModifyFont( font( ), QFont::Bold );


    _exposureTimeLabel->setAlignment( Qt::AlignTop | Qt::AlignLeft );
    _exposureTimeLabel->setText( "Exposure time (seconds):" );

    _exposureTimeValue->setAlignment( Qt::AlignTop | Qt::AlignRight );
    _exposureTimeValue->setFont( boldFont );


    _exposureTimeSlider->setMinimum( 1 );
    _exposureTimeSlider->setMaximum( 120 );
    _exposureTimeSlider->setOrientation( Qt::Horizontal );
    _exposureTimeSlider->setPageStep( 4 );
    _exposureTimeSlider->setSingleStep( 1 );
    _exposureTimeSlider->setTickInterval( 8 );
    _exposureTimeSlider->setTickPosition( QSlider::TicksBothSides );
    QObject::connect( _exposureTimeSlider, &QSlider::valueChanged, this, &PrintTab::exposureTimeSlider_valueChanged );


    _exposureTimeScaleFactorLabel->setAlignment( Qt::AlignTop | Qt::AlignLeft );
    _exposureTimeScaleFactorLabel->setText( "First layers time scale factor:" );

    _exposureTimeScaleFactorValue->setAlignment( Qt::AlignTop | Qt::AlignRight );
    _exposureTimeScaleFactorValue->setFont( boldFont );

    _exposureTimeScaleFactorSlider->setMinimum( 1 );
    _exposureTimeScaleFactorSlider->setMaximum( 5 );
    _exposureTimeScaleFactorSlider->setOrientation( Qt::Horizontal );
    _exposureTimeScaleFactorSlider->setPageStep( 1 );
    _exposureTimeScaleFactorSlider->setSingleStep( 1 );
    _exposureTimeScaleFactorSlider->setTickInterval( 1 );
    _exposureTimeScaleFactorSlider->setTickPosition( QSlider::TicksBothSides );
    QObject::connect( _exposureTimeScaleFactorSlider, &QSlider::valueChanged, this, &PrintTab::exposureTimeScaleFactorSlider_valueChanged );


    _exposureLayout->addLayout( WrapWidgetsInVBoxDM(
        WrapWidgetsInHBox( _exposureTimeLabel, nullptr, _exposureTimeValue ),
        _exposureTimeSlider
    ), 8 );
    _exposureLayout->addStretch( 1 );
    _exposureLayout->addLayout( WrapWidgetsInVBoxDM(
        WrapWidgetsInHBox( _exposureTimeScaleFactorLabel, nullptr, _exposureTimeScaleFactorValue ),
        _exposureTimeScaleFactorSlider
    ), 4 );


    _powerLevelLabel->setText( "Projector power level:" );

    _powerLevelValue->setAlignment( Qt::AlignRight );
    _powerLevelValue->setFont( boldFont );

    _powerLevelSlider->setMinimum( 20 );
    _powerLevelSlider->setMaximum( 100 );
    _powerLevelSlider->setOrientation( Qt::Horizontal );
    _powerLevelSlider->setPageStep( 5 );
    _powerLevelSlider->setSingleStep( 1 );
    _powerLevelSlider->setTickInterval( 5 );
    _powerLevelSlider->setTickPosition( QSlider::TicksBothSides );
    QObject::connect( _powerLevelSlider, &QSlider::valueChanged,   this, &PrintTab::powerLevelSlider_valueChanged   );
    QObject::connect( _powerLevelSlider, &QSlider::sliderReleased, this, &PrintTab::powerLevelSlider_sliderReleased );


#if defined ENABLE_SPEED_SETTING
    _printSpeedLabel->setText( "Print speed:" );

    _printSpeedValue->setAlignment( Qt::AlignRight );
    _printSpeedValue->setFont( boldFont );

    _printSpeedSlider->setMinimum( 50 );
    _printSpeedSlider->setMaximum( 200 );
    _printSpeedSlider->setOrientation( Qt::Horizontal );
    _printSpeedSlider->setPageStep( 50 );
    _printSpeedSlider->setSingleStep( 25 );
    _printSpeedSlider->setTickInterval( 10 );
    _printSpeedSlider->setTickPosition( QSlider::TicksBothSides );
    QObject::connect( _printSpeedSlider, &QSlider::valueChanged, this, &PrintTab::printSpeedSlider_valueChanged );
#endif // defined ENABLE_SPEED_SETTING


    _optionsGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _optionsGroup->setLayout( WrapWidgetsInVBoxDM(
        _exposureLayout,
        WrapWidgetsInHBoxDM( _powerLevelLabel, nullptr, _powerLevelValue ),
        _powerLevelSlider,
#if defined ENABLE_SPEED_SETTING
        WrapWidgetsInHBoxDM( _printSpeedLabel, nullptr, _printSpeedValue ),
        _printSpeedSlider,
#endif // defined ENABLE_SPEED_SETTING
        nullptr
    ) );
    _optionsGroup->setTitle( "Print settings" );

    _printButton->setEnabled( false );
    _printButton->setFixedSize( MainButtonSize );
    _printButton->setFont( ModifyFont( _printButton->font( ), LargeFontSize ) );
    _printButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _printButton->setText( "Continue…" );
    QObject::connect( _printButton, &QPushButton::clicked, this, &PrintTab::printButton_clicked );

    _raiseOrLowerButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _raiseOrLowerButton->setText( RaiseBuildPlatformText );
    QObject::connect( _raiseOrLowerButton, &QPushButton::clicked, this, &PrintTab::raiseOrLowerButton_clicked );

    _homeButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _homeButton->setText( "Home" );
    QObject::connect( _homeButton, &QPushButton::clicked, this, &PrintTab::homeButton_clicked );

    _adjustmentsGroup->setFixedHeight( MainButtonSize.height( ) );
    _adjustmentsGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    _adjustmentsGroup->setLayout( WrapWidgetsInHBox( nullptr, _homeButton, nullptr, _raiseOrLowerButton, nullptr ) );
    _adjustmentsGroup->setTitle( "Adjustments" );

    _layout->setContentsMargins( { } );
    _layout->addWidget( _optionsGroup,     0, 0, 1, 2 );
    _layout->addWidget( _printButton,      1, 0, 1, 1 );
    _layout->addWidget( _adjustmentsGroup, 1, 1, 1, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
}

PrintTab::~PrintTab( ) {
    /*empty*/
}

void PrintTab::_connectPrintJob( ) {
    debug( "+ PrintTab::setPrintJob: _printJob %p\n", _printJob );

    {
        _exposureTimeSlider->setValue( _printJob->exposureTime * 4.0 );
        _exposureTimeValue->setText( QString( "%1 s" ).arg( _printJob->exposureTime, 0, 'f', 2 ) );
    }

    _exposureTimeScaleFactorSlider->setValue( _printJob->exposureTimeScaleFactor );
    _exposureTimeScaleFactorValue->setText( QString( "%1×" ).arg( _printJob->exposureTimeScaleFactor ) );

    int powerLevelValue = RawPowerLevelToPercentage( _printJob->powerLevel );
    _powerLevelSlider->setValue( powerLevelValue );
    _powerLevelValue->setText( QString( "%1%" ).arg( powerLevelValue ) );

#if defined ENABLE_SPEED_SETTING
    auto printSpeedValue = static_cast<int>( _printJob->printSpeed + 0.5 );
    _printSpeedSlider->setValue( printSpeedValue );
    _printSpeedValue->setText( QString( "%1 mm/min" ).arg( printSpeedValue ) );
#endif // defined ENABLE_SPEED_SETTING

    update( );
}

void PrintTab::_connectShepherd( ) {
    QObject::connect( _shepherd, &Shepherd::printer_online,  this, &PrintTab::printer_online  );
    QObject::connect( _shepherd, &Shepherd::printer_offline, this, &PrintTab::printer_offline );
}

void PrintTab::_updateUiState( ) {
    bool isEnabled = _isPrinterOnline && _isPrinterAvailable;

    _optionsGroup      ->setEnabled( isEnabled );
    _printButton       ->setEnabled( isEnabled && _isPrinterPrepared && _isModelRendered );
    _raiseOrLowerButton->setEnabled( isEnabled );
    _homeButton        ->setEnabled( isEnabled );

    update( );
}

void PrintTab::_initialShowEvent( QShowEvent* event ) {
    auto size = maxSize( _raiseOrLowerButton->size( ), _homeButton->size( ) ) + ButtonPadding;

    auto fm = _raiseOrLowerButton->fontMetrics( );
    auto raiseSize = fm.size( Qt::TextSingleLine | Qt::TextShowMnemonic, RaiseBuildPlatformText );
    auto lowerSize = fm.size( Qt::TextSingleLine | Qt::TextShowMnemonic, LowerBuildPlatformText );
    if ( lowerSize.width( ) > raiseSize.width( ) ) {
        size.setWidth( size.width( ) + lowerSize.width( ) - raiseSize.width( ) );
    }

    _raiseOrLowerButton->setFixedSize( size );
    _homeButton        ->setFixedSize( size );

    event->accept( );

    update( );
}

void PrintTab::exposureTimeSlider_valueChanged( int value ) {
    _printJob->exposureTime = value / 4.0;
    _exposureTimeValue->setText( QString( "%1 s" ).arg( _printJob->exposureTime, 0, 'f', 2 ) );

    update( );
}

void PrintTab::exposureTimeScaleFactorSlider_valueChanged( int value ) {
    _printJob->exposureTimeScaleFactor = value;
    _exposureTimeScaleFactorValue->setText( QString( "%1×" ).arg( value ) );

    update( );
}

void PrintTab::powerLevelSlider_sliderReleased( ) {
    _printJob->powerLevel = PercentagePowerLevelToRawLevel( _powerLevelSlider->value( ) );

    emit projectorPowerLevelChanged( _powerLevelSlider->value( ) );
}

void PrintTab::powerLevelSlider_valueChanged( int percentage ) {
    _printJob->powerLevel = PercentagePowerLevelToRawLevel( percentage );
    _powerLevelValue->setText( QString( "%1%" ).arg( percentage ) );

    update( );
}

void PrintTab::projectorPowerLevel_changed( int const percentage ) {
    _powerLevelSlider->setValue( percentage );
    _powerLevelValue->setText( QString( "%1%" ).arg( percentage ) );

    update( );
}

#if defined ENABLE_SPEED_SETTING
void PrintTab::printSpeedSlider_valueChanged( int value ) {
    _printJob->printSpeed = value;
    _printSpeedValue->setText( QString( "%1 mm/min" ).arg( value ) );

    update( );
}
#endif // defined ENABLE_SPEED_SETTING

void PrintTab::printButton_clicked( bool ) {
    debug( "+ PrintTab::printButton_clicked\n" );
    emit printRequested( );
    emit uiStateChanged( TabIndex::Print, UiState::PrintStarted );

    update( );
}

void PrintTab::raiseOrLowerButton_clicked( bool ) {
    debug( "+ PrintTab::raiseOrLowerButton_clicked: build platform state %s [%d]\n", ToString( _buildPlatformState ), _buildPlatformState );

    switch ( _buildPlatformState ) {
        case BuildPlatformState::Lowered:
        case BuildPlatformState::Raising:
            _buildPlatformState = BuildPlatformState::Raising;

            QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintTab::raiseBuildPlatform_moveAbsoluteComplete );
            _shepherd->doMoveAbsolute( PrinterRaiseToMaximumZ, PrinterDefaultHighSpeed );
            break;

        case BuildPlatformState::Raised:
        case BuildPlatformState::Lowering:
            _buildPlatformState = BuildPlatformState::Lowering;

            QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintTab::lowerBuildPlatform_moveAbsoluteComplete );
            _shepherd->doMoveAbsolute( std::max( 100, _printJob->layerThickness ) / 1000.0, PrinterDefaultHighSpeed );
            break;
    }

    setPrinterAvailable( false );
    emit printerAvailabilityChanged( false );

    update( );
}

void PrintTab::raiseBuildPlatform_moveAbsoluteComplete( bool const success ) {
    debug( "+ PrintTab::raiseBuildPlatform_moveAbsoluteComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintTab::raiseBuildPlatform_moveAbsoluteComplete );

    if ( success ) {
        _buildPlatformState = BuildPlatformState::Raised;
        _raiseOrLowerButton->setText( LowerBuildPlatformText );
        _raiseOrLowerButton->setEnabled( true );
    } else {
        _buildPlatformState = BuildPlatformState::Lowered;
    }

    setPrinterAvailable( true );
    emit printerAvailabilityChanged( true );

    update( );
}

void PrintTab::lowerBuildPlatform_moveAbsoluteComplete( bool const success ) {
    debug( "+ PrintTab::lowerBuildPlatform_moveAbsoluteComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrintTab::lowerBuildPlatform_moveAbsoluteComplete );

    if ( success ) {
        _buildPlatformState = BuildPlatformState::Lowered;
        _raiseOrLowerButton->setText( RaiseBuildPlatformText );
        _raiseOrLowerButton->setEnabled( true );
    } else {
        _buildPlatformState = BuildPlatformState::Raised;
    }

    setPrinterAvailable( true );
    emit printerAvailabilityChanged( true );

    update( );
}

void PrintTab::homeButton_clicked( bool ) {
    debug( "+ PrintTab::homeButton_clicked\n" );

    QObject::connect( _shepherd, &Shepherd::action_homeComplete, this, &PrintTab::home_homeComplete );
    _shepherd->doHome( );

    setPrinterAvailable( false );
    emit printerAvailabilityChanged( false );

    update( );
}

void PrintTab::home_homeComplete( bool const success ) {
    debug( "+ PrintTab::home_homeComplete: %s\n", success ? "succeeded" : "failed" );

    setPrinterAvailable( true );
    emit printerAvailabilityChanged( true );

    update( );
}

void PrintTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ PrintTab::tab_uiStateChanged: from %sTab: %s => %s; PO? %s PA? %s PP? %s MR? %s\n", ToString( sender ), ToString( _uiState ), ToString( state ), YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );
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
    }

    update( );
}

void PrintTab::printer_online( ) {
    _isPrinterOnline = true;
    debug( "+ PrintTab::printer_online: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _updateUiState( );
}

void PrintTab::printer_offline( ) {
    _isPrinterOnline = false;
    debug( "+ PrintTab::printer_offline: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _updateUiState( );
}

void PrintTab::setModelRendered( bool const value ) {
    _isModelRendered = value;
    debug( "+ PrintTab::setModelRendered: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _updateUiState( );
}

void PrintTab::setPrinterPrepared( bool const value ) {
    _isPrinterPrepared = value;
    debug( "+ PrintTab::setPrinterPrepared: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _updateUiState( );
}

void PrintTab::setPrinterAvailable( bool const value ) {
    _isPrinterAvailable = value;
    debug( "+ PrintTab::setPrinterAvailable: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _updateUiState( );
}
