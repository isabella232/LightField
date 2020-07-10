#include "pch.h"

#include "printtab.h"

#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"
#include "ordermanifestmanager.h"
#include "spoiler.h"

namespace {

    QString const RaiseBuildPlatformText { "Raise build platform" };
    QString const LowerBuildPlatformText { "Lower build platform" };

}

PrintTab::PrintTab( QWidget* parent ): InitialShowEventMixin<PrintTab, TabBase>( parent ) {
#if defined _DEBUG
    _isPrinterPrepared = g_settings.pretendPrinterIsPrepared;
#endif // _DEBUG

    auto boldFont = ModifyFont( font( ), QFont::Bold );

    QObject::connect( _powerLevelSlider, &ParamSlider::valueChanged,   this, &PrintTab::powerLevelSlider_valueChanged   );

    connectBasicExpoTimeCallback( true );

    _powerLevelSlider->innerSlider()->setPageStep( 5 );
    _powerLevelSlider->innerSlider()->setSingleStep( 1 );
    _powerLevelSlider->innerSlider()->setTickInterval( 5 );

    _expoDisabledTilingWarning->setMinimumSize(400, 50);

    _optionsGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
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


    QScrollArea* advArea = new QScrollArea();

    _basicExpoTimeGroup = new Spoiler("Basic exposure time controll");
    _advancedExpoTimeGroup = new Spoiler("Advanced exposure time controll");

    QObject::connect(_basicExpoTimeGroup, &Spoiler::collapseStateChanged, [this](bool checked) {
        _advancedExpoTimeGroup->setCollapsed(checked);

        connectBasicExpoTimeCallback(checked);
        connectAdvanExpoTimeCallback(!checked);

        if(checked)
            basicExposureTime_update();
        else
            advancedExposureTime_update();

    });

    QObject::connect(_advancedExpoTimeGroup, &Spoiler::collapseStateChanged, [this](bool checked) {
        _basicExpoTimeGroup->setCollapsed(checked);

        connectBasicExpoTimeCallback(!checked);
        connectAdvanExpoTimeCallback(checked);

        if(checked)
            basicExposureTime_update();
        else
            advancedExposureTime_update();
    });

    _basicExpoTimeGroup->setCollapsed(false);
    _basicExpoTimeGroup->setContentLayout(
        WrapWidgetsInVBox(
            _bodyExposureTimeSlider,
            _baseExposureTimeSlider
        )
    );

    QVBoxLayout* container =
        WrapWidgetsInVBox(
                  WrapWidgetsInHBox(_advBodyExpoCorse, _advBodyExpoFine),
                  WrapWidgetsInHBox(_advBaseExpoCorse, _advBaseExpoFine)
        );

    _advancedExpoTimeGroup->setCollapsed(true);
    _advancedExpoTimeGroup->setContentLayout(
        container
    );


    int scrolbarWidth = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    int contentWidth = MainWindowSize.width() - ButtonPadding.width() - scrolbarWidth;

    _advancedExpoTimeGroup->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    _advancedExpoTimeGroup->setMinimumWidth(contentWidth);
    _basicExpoTimeGroup->setMinimumWidth(contentWidth);

    QWidget* widget = new QWidget(advArea);
    widget->setLayout( WrapWidgetsInVBox(_basicExpoTimeGroup, _advancedExpoTimeGroup, nullptr));


    advArea->setWidget(widget);

    _optionsGroup->setLayout(
        WrapWidgetsInVBox(
              advArea,
              _expoDisabledTilingWarning,
              nullptr
        )
    );

    setLayout(
        WrapWidgetsInVBox(
            _powerLevelSlider,
            _optionsGroup,
            nullptr,
            WrapWidgetsInHBox(_printButton, _adjustmentsGroup)
        )
    );
}

PrintTab::~PrintTab( ) {
    /*empty*/
}

void PrintTab::_connectPrintJob()
{
    debug( "+ PrintTab::setPrintJob: _printJob %p\n", _printJob );

    int powerLevelValue = _printJob->baseLayerParameters().powerLevel();

    _powerLevelSlider->setValue( powerLevelValue );
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

    _baseExposureTimeSlider->setEnabled(_printJob->hasExposureControlsEnabled());
    _bodyExposureTimeSlider->setEnabled(_printJob->hasExposureControlsEnabled());
    _expoDisabledTilingWarning->setVisible(_printJob->isTiled());

    event->accept( );

    update( );
}

void PrintTab::powerLevelSlider_valueChanged()
{
    double value = _powerLevelSlider->getValue();

    PrintParameters& baseParams = _printJob->baseLayerParameters();
    PrintParameters& bodyParams = _printJob->bodyLayerParameters();

    baseParams.setPowerLevel( value );
    bodyParams.setPowerLevel( value );

    update();
}

void PrintTab::projectorPowerLevel_changed(int percentage)
{
    _powerLevelSlider->setValue(percentage);
    update();
}


void PrintTab::basicExposureTime_update( ) {
    auto& bodyParams = _printJob->bodyLayerParameters();
    auto& baseParams = _printJob->baseLayerParameters();

    int bodyExpoTime = _bodyExposureTimeSlider->getValue();
    int baseExpoMulpl = _baseExposureTimeSlider->getValue();
    int baseExpoTime = baseExpoMulpl * bodyExpoTime;

    _advBodyExpoCorse->setValue(bodyExpoTime - (bodyExpoTime % 1000));
    _advBodyExpoFine->setValue(bodyExpoTime % 1000);

    _advBaseExpoCorse->setValue(baseExpoTime - (baseExpoTime % 1000));
    _advBaseExpoFine->setValue(baseExpoTime % 1000);

    bodyParams.setLayerExposureTime(bodyExpoTime);
    baseParams.setLayerExposureTime(baseExpoTime);
    _printJob->setAdvancedExposureControlsEnabled(false);
    update( );
}

void PrintTab::advancedExposureTime_update( ) {
    auto& bodyParams = _printJob->bodyLayerParameters();
    auto& baseParams = _printJob->baseLayerParameters();

    int bodyExpoTime = _advBodyExpoCorse->getValue() + _advBodyExpoFine->getValue();
    int baseExpoTime = _advBaseExpoCorse->getValue() + _advBaseExpoFine->getValue();

    int bodyExpoTimeRounded = round((bodyExpoTime * 4)/1000) * 250;
    int baseMultiplier = baseExpoTime / bodyExpoTimeRounded;

    if(baseMultiplier > 5)
        baseMultiplier = 5;
    else if (baseMultiplier < 1)
        baseMultiplier = 1;

    _bodyExposureTimeSlider->setValue(bodyExpoTimeRounded);
    _baseExposureTimeSlider->setValue(baseMultiplier);

    bodyParams.setLayerExposureTime(bodyExpoTime);
    baseParams.setLayerExposureTime(baseExpoTime);
    _printJob->setAdvancedExposureControlsEnabled(true);

    update( );
}

void PrintTab::connectBasicExpoTimeCallback(bool connect) {

    if(connect) {
        QObject::connect( _bodyExposureTimeSlider, &ParamSlider::valueChanged,   this, &PrintTab::basicExposureTime_update   );
        QObject::connect( _baseExposureTimeSlider, &ParamSlider::valueChanged,   this, &PrintTab::basicExposureTime_update   );
    } else {
        QObject::disconnect( _bodyExposureTimeSlider, &ParamSlider::valueChanged,   this, &PrintTab::basicExposureTime_update   );
        QObject::disconnect( _baseExposureTimeSlider, &ParamSlider::valueChanged,   this, &PrintTab::basicExposureTime_update   );
    }
}

void PrintTab::connectAdvanExpoTimeCallback(bool connect) {

    if(connect) {
        QObject::connect( _advBodyExpoCorse, &ParamSlider::valueChanged,   this, &PrintTab::advancedExposureTime_update   );
        QObject::connect( _advBodyExpoFine, &ParamSlider::valueChanged,   this, &PrintTab::advancedExposureTime_update   );
        QObject::connect( _advBaseExpoCorse, &ParamSlider::valueChanged,   this, &PrintTab::advancedExposureTime_update   );
        QObject::connect( _advBaseExpoFine, &ParamSlider::valueChanged,   this, &PrintTab::advancedExposureTime_update   );
    } else {
        QObject::disconnect( _advBodyExpoCorse, &ParamSlider::valueChanged,   this, &PrintTab::advancedExposureTime_update   );
        QObject::disconnect( _advBodyExpoFine, &ParamSlider::valueChanged,   this, &PrintTab::advancedExposureTime_update   );
        QObject::disconnect( _advBaseExpoCorse, &ParamSlider::valueChanged,   this, &PrintTab::advancedExposureTime_update   );
        QObject::disconnect( _advBaseExpoFine, &ParamSlider::valueChanged,   this, &PrintTab::advancedExposureTime_update   );
    }
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
    //exposureTime_update();
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

            _shepherd->doMoveAbsolute(std::max(100, _printJob->getLayerThicknessAt(0)) / 1000.0,
                PrinterDefaultHighSpeed);
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

    auto bodyParams = _printJob->bodyLayerParameters();
    auto baseParams = _printJob->baseLayerParameters();

    switch (_uiState) {
        case UiState::SelectCompleted:
            _baseExposureTimeSlider->setEnabled(_printJob->hasExposureControlsEnabled());
            _bodyExposureTimeSlider->setEnabled(_printJob->hasExposureControlsEnabled());

            if(!_printJob->isTiled())
                _expoDisabledTilingWarning->hide();
            else
                _expoDisabledTilingWarning->show();


            _bodyExposureTimeSlider->setValue(bodyParams.layerExposureTime());
            _baseExposureTimeSlider->setValue(baseParams.layerExposureTime() /
                bodyParams.layerExposureTime());

            break;

        case UiState::PrintJobReady:
            _baseExposureTimeSlider->setEnabled(_printJob->hasExposureControlsEnabled());
            _bodyExposureTimeSlider->setEnabled(_printJob->hasExposureControlsEnabled());

            if(!_printJob->isTiled()) {
                _expoDisabledTilingWarning->hide();
            } else {
                _expoDisabledTilingWarning->show();
            }

            break;

        case UiState::PrintStarted:
            setPrinterAvailable(false);
            emit printerAvailabilityChanged(false);
            break;

        case UiState::PrintCompleted:
            setPrinterAvailable(true);
            emit printerAvailabilityChanged(true);
            break;

        default:
            break;
    }

    update();
}

void PrintTab::changeExpoTimeSliders()
{
    bool enable = _printJob->hasExposureControlsEnabled();
    _baseExposureTimeSlider->setEnabled(enable);
    _bodyExposureTimeSlider->setEnabled(enable);

    if(enable) {
        auto bodyParams = _printJob->bodyLayerParameters();
        auto baseParams = _printJob->baseLayerParameters();
        bodyParams.setLayerExposureTime(_bodyExposureTimeSlider->getValue());
        baseParams.setLayerExposureTime(_bodyExposureTimeSlider->getValue() *
            _baseExposureTimeSlider->getValue());
    }
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

void PrintTab::activeProfileChanged(QSharedPointer<PrintProfile> newProfile) {
    (void)newProfile;

    syncFormWithPrintProfile();
}

void PrintTab::syncFormWithPrintProfile() {
    auto& bodyParams = _printJob->bodyLayerParameters();
    auto& baseParams = _printJob->baseLayerParameters();

    int bodyExpoTime = bodyParams.layerExposureTime();
    int baseExpoTime = baseParams.layerExposureTime();

    if(_printJob->getAdvancedExposureControlsEnabled()) {
        _advBodyExpoCorse->setValue(bodyExpoTime - (bodyExpoTime % 1000));
        _advBodyExpoFine->setValue(bodyExpoTime % 1000);

        _advBaseExpoCorse->setValue(baseExpoTime - (baseExpoTime % 1000));
        _advBaseExpoFine->setValue(baseExpoTime % 1000);

        _advancedExpoTimeGroup->setCollapsed(false);
        _basicExpoTimeGroup->setCollapsed(true);

        advancedExposureTime_update();
    } else {
        _bodyExposureTimeSlider->setValue(bodyParams.layerExposureTime());
        _baseExposureTimeSlider->setValue(baseParams.layerExposureTime() /
            bodyParams.layerExposureTime());

        _advancedExpoTimeGroup->setCollapsed(true);
        _basicExpoTimeGroup->setCollapsed(false);

        basicExposureTime_update();
    }
}
