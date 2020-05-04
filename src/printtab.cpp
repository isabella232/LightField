#include "pch.h"

#include "printtab.h"

#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"
#include "ordermanifestmanager.h"

namespace {

    QString const RaiseBuildPlatformText { "Raise build platform" };
    QString const LowerBuildPlatformText { "Lower build platform" };

}

PrintTab::PrintTab( QWidget* parent ): InitialShowEventMixin<PrintTab, TabBase>( parent ) {
#if defined _DEBUG
    _isPrinterPrepared = g_settings.pretendPrinterIsPrepared;
#endif // _DEBUG

    auto boldFont = ModifyFont( font( ), QFont::Bold );


    _powerLevelLabel->setText( "Projector power level:" );

    _powerLevelValue->setAlignment( Qt::AlignRight );
    _powerLevelValue->setFont( boldFont );

    _powerLevelSlider->setMinimum( ProjectorMinPercent );
    _powerLevelSlider->setMaximum( ProjectorMaxPercent );
    _powerLevelSlider->setOrientation( Qt::Horizontal );
    _powerLevelSlider->setPageStep( 5 );
    _powerLevelSlider->setSingleStep( 1 );
    _powerLevelSlider->setTickInterval( 5 );
    _powerLevelSlider->setTickPosition( QSlider::TicksBothSides );
    QObject::connect( _powerLevelSlider, &QSlider::valueChanged,   this, &PrintTab::powerLevelSlider_valueChanged   );
    QObject::connect( _powerLevelSlider, &QSlider::sliderReleased, this, &PrintTab::powerLevelSlider_sliderReleased );


    _optionsGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _optionsGroup->setLayout( WrapWidgetsInVBoxDM(
        WrapWidgetsInHBoxDM( _powerLevelLabel, nullptr, _powerLevelValue ),
        _powerLevelSlider,
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

    int powerLevelValue = _printJob->printProfile->baseLayerParameters( ).powerLevel( );

    _powerLevelSlider->setValue( powerLevelValue );
    _powerLevelValue->setText( QString( "%1%" ).arg( powerLevelValue ) );

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

void PrintTab::powerLevelSlider_sliderReleased( ) {
    _printJob->printProfile->baseLayerParameters( ).setPowerLevel( _powerLevelSlider->value( ) );
    _printJob->printProfile->bodyLayerParameters( ).setPowerLevel( _powerLevelSlider->value( ) );

    emit projectorPowerLevelChanged( _powerLevelSlider->value( ) );
}

void PrintTab::powerLevelSlider_valueChanged( int percentage ) {
    _printJob->printProfile->baseLayerParameters( ).setPowerLevel( _powerLevelSlider->value( ) );
    _printJob->printProfile->bodyLayerParameters( ).setPowerLevel( _powerLevelSlider->value( ) );
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
            _shepherd->doMoveAbsolute( std::max( 100, ( ( _printJob->printProfile->baseLayerCount( ) > 0 ) ? _printJob->printProfile->baseLayerParameters( ) : _printJob->printProfile->bodyLayerParameters( ) ).layerThickness( ) ) / 1000.0, PrinterDefaultHighSpeed );
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
        case UiState::TilingClicked:
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
