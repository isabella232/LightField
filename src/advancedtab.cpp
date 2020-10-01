#include "pch.h"

#include "advancedtab.h"

#include "pngdisplayer.h"
#include "printjob.h"
#include "printmanager.h"
#include "firmwarecontroller.h"
#include "advancedtabselectionmodel.h"
#include "paramslider.h"
#include "ordermanifestmanager.h"

#include <iostream>

namespace {

    auto DefaultPrintBedTemperature = 60;

}

AdvancedTab::AdvancedTab(QWidget* parent): TabBase(parent) {
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto fontAwesome = ModifyFont( origFont, "Font Awesome 5 Brands", LargeFontSize );

    _forms[0] = _generalForm;
    _forms[1] = _basePumpForm;
    _forms[2] = _bodyPumpForm;

    QWidget::connect(_addBasePumpCheckbox, &QCheckBox::clicked, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_addBodyPumpCheckbox, &QCheckBox::clicked, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_distanceSlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_basePumpUpVelocitySlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_basePumpDownVelocitySlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_upPauseSlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_downPauseSlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_baseNoPumpUpVelocitySlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_bodyPumpEveryNthLayer, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_bodyDistanceSlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_bodyPumpUpVelocitySlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_bodyPumpDownVelocitySlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_bodyUpPauseSlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_bodyDownPauseSlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);
    QWidget::connect(_bodyNoPumpUpVelocitySlider, &ParamSlider::valueChanged, this, &AdvancedTab::updatePrintProfile);

    _setUpLeftMenu(fontAwesome);
    _setUpGeneralForm(boldFont, fontAwesome);
    _setUpTemperaturelForm(boldFont);
    _setUpBasePumpForm(boldFont);
    _setUpBodyPumpForm(boldFont);

    for(int i = 1; i < FORMS_COUNT; ++i)
        _forms[i]->setVisible(false);

    _rightColumn->setLayout(WrapWidgetsInVBox(
        _generalForm,
        _basePumpForm,
        _bodyPumpForm,
        nullptr
    ));

    setLayout( WrapWidgetsInHBox(
        WrapWidgetsInVBox( _leftMenu, _temperatureForm ),
        _rightColumn, nullptr
        )
    );
}

AdvancedTab::~AdvancedTab()
{
    /*empty*/
}

void AdvancedTab::chbox_addBodyPumpChanged(int state)
{
    _bodyPumpEveryNthLayer->setEnabled(false);
    _bodyDistanceSlider->setEnabled(state);
    _bodyPumpUpVelocitySlider->setEnabled(state);
    _bodyPumpDownVelocitySlider->setEnabled(state);
    _bodyUpPauseSlider->setEnabled(state);
    _bodyDownPauseSlider->setEnabled(state);
    _bodyNoPumpUpVelocitySlider->setEnabled(state);
}

void AdvancedTab::chbox_addBasePumpCheckChanged(int state)
{
    _distanceSlider->setEnabled(state);
    _basePumpUpVelocitySlider->setEnabled(state);
    _basePumpDownVelocitySlider->setEnabled(state);
    _upPauseSlider->setEnabled(state);
    _downPauseSlider->setEnabled(state);
    _baseNoPumpUpVelocitySlider->setEnabled(state);
}

void AdvancedTab::_connectFirmwareController()
{
    if (_firmwareController) {
        QObject::connect(_firmwareController, &FirmwareController::printerOnline, this,
            &AdvancedTab::printer_online);
        QObject::connect(_firmwareController, &FirmwareController::printerOffline, this,
            &AdvancedTab::printer_offline);
        QObject::connect(_firmwareController, &FirmwareController::printerPositionReport, this,
            &AdvancedTab::printer_positionReport);
        QObject::connect(_firmwareController, &FirmwareController::printerTemperatureReport, this,
            &AdvancedTab::printer_temperatureReport);
    }
}

void AdvancedTab::tab_uiStateChanged(TabIndex const sender, UiState const state)
{
    debug("+ AdvancedTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString(sender),
        ToString(_uiState), ToString(state));
    _uiState = state;

    switch (_uiState) {
    case UiState::SelectStarted:
        _setEnabled(true);
        break;

    case UiState::PrintStarted:
        setPrinterAvailable(false);
        _setEnabled(false);
        emit printerAvailabilityChanged(false);
        break;

    case UiState::PrintCompleted:
        setPrinterAvailable(true);
        _setEnabled(true);
        emit printerAvailabilityChanged(true);
        break;
    default:
        break;
    }
}

void AdvancedTab::printer_positionReport(double const px, int const cx)
{
    debug( "+ AdvancedTab::printer_positionReport: px %.2f mm, cx %d\n", px, cx );
    _zPosition->setText( QString { "%1 mm" }.arg( px, 0, 'f', 2 ) );

    update( );
}

void AdvancedTab::printer_temperatureReport(double bedCurrentTemperature,
    double bedTargetTemperature, int const bedPwm)
{
    _currentTemperature->setText(QString("%1 °C").arg(bedCurrentTemperature, 0, 'f', 2));
    if (bedTargetTemperature < 30) {
        _targetTemperature->setText(QString("OFF"));
    } else {
        _targetTemperature->setText(QString("%1 °C").arg(bedTargetTemperature, 0, 'f', 2));
    }
    _heatingElement->setText(bedPwm ? "ON" : "OFF");

    update( );
}


void AdvancedTab::offsetSliderValueChanged()
{
    _printProfileManager->activeProfile()->setBuildPlatformOffset(_offsetSlider->getValue());

    update();
}

void AdvancedTab::printBedHeatingButton_clicked( bool checked ) {
    _bedHeatingButton->setText( checked ? FA_Check : FA_Times );
#if defined ENABLE_TEMPERATURE_SETTING
    _bedTemperatureLabel->setEnabled( checked );
    _bedTemperatureSlider->setEnabled( checked );
    _bedTemperatureValue->setEnabled( checked );
    _bedTemperatureValueLayout->setEnabled( checked );
#endif

    QObject::connect(_firmwareController, &FirmwareController::printerSetTemperatureCompleted, this,
        &AdvancedTab::setTemperatureComplete);

#if defined ENABLE_TEMPERATURE_SETTING
    _firmwareController->setTemperature(checked ? _bedTemperatureSlider->value() : 0);
#else
    _firmwareController->setTemperature(checked ? DefaultPrintBedTemperature : 0);
#endif

    update();
}

#if defined ENABLE_TEMPERATURE_SETTING
void AdvancedTab::printBedTemperatureSlider_sliderReleased()
{
    QObject::connect(_firmwareController, &FirmwareController::printerSetTemperatureCompleted, this,
        &AdvancedTab::setTemperatureComplete);
    _firmwareController->setTemperature(_bedTemperatureSlider->value());
}

void AdvancedTab::printBedTemperatureSlider_valueChanged( int value ) {
    debug( "+ AdvancedTab::printBedTemperatureSlider_valueChanged: new value %d °C\n", value );
    _bedTemperatureValue->setText( QString { "%1 °C" }.arg( value ) );
    _printProfileManager->activeProfile()->setHeatingTemperature(value);
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

void AdvancedTab::powerLevelSlider_valueChanged(int percentage)
{
    _printProfileManager->activeProfile()->baseLayerParameters().setPowerLevel(percentage);
    _printProfileManager->activeProfile()->bodyLayerParameters().setPowerLevel(percentage);
    _powerLevelValue->setText(QString { "%1%" }.arg( percentage));

    //if value is greater than 80% then set font color to red
    if(percentage>80) {
        _powerLevelLabel->setStyleSheet(QString("QLabel { color : red; }"));
        _powerLevelValue->setStyleSheet(QString("QLabel { color : red; }"));
    } else {
        _powerLevelLabel->setStyleSheet(QString("QLabel { color : white; }"));
        _powerLevelValue->setStyleSheet(QString("QLabel { color : white; }"));
    }

    update();
}

void AdvancedTab::projectorPowerLevel_changed( int const percentage ) {
    _powerLevelSlider->setValue( percentage );
    _powerLevelValue->setText( QString( "%1%" ).arg( percentage ) );

    update( );
}

void AdvancedTab::setTemperatureComplete(bool const success) {
    debug( "+ AdvancedTab::setTemperatureComplete: action %s\n", SucceededString(success));
    QObject::disconnect(_firmwareController, &FirmwareController::printerSetTemperatureCompleted,
        this, &AdvancedTab::setTemperatureComplete);
}

void AdvancedTab::_updateControlGroups( ) {
    _projectImageButtonsGroup->setEnabled( _isProjectorOn || ( _isPrinterOnline && _isPrinterAvailable && ( _firmwareController != nullptr ) && ( _pngDisplayer != nullptr ) ) );

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

void AdvancedTab::_setUpLeftMenu(QFont fontAwesome) {
    AdvancedTabSelectionModel* model = new AdvancedTabSelectionModel(3, 1, _forms, FORMS_COUNT);

    QStandardItem* item = new QStandardItem(QString("General"));
    QStandardItem* generalItem = item;
    model->setItem(0, 0, item);

    item = new QStandardItem(QString("Base Pump"));
    model->setItem(1, 0, item);

    item = new QStandardItem(QString("Body Pump"));
    model->setItem(2, 0, item);

    QItemSelectionModel* selectionModel = new QItemSelectionModel(model);
    QObject::connect(
        _leftMenu, &QTreeView::pressed, model, &AdvancedTabSelectionModel::onclick
    );

    _leftMenu->setModel( model );
    _leftMenu->setSelectionModel( selectionModel );
    _leftMenu->setFont( fontAwesome );
    _leftMenu->setVisible( true );
    _leftMenu->setSelectionBehavior(QAbstractItemView::SelectRows);
    _leftMenu->setCurrentIndex(model->indexFromItem(generalItem));
    _leftMenu->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding );
}

void AdvancedTab::_setUpGeneralForm(QFont boldFont, QFont fontAwesome)
{
    QObject::connect(_offsetSlider, &ParamSlider::valueChanged, this,
        &AdvancedTab::offsetSliderValueChanged);
    QObject::connect(_offsetDisregardFirstLayer, &QCheckBox::stateChanged, this,
        &AdvancedTab::offsetDisregardFirstLayerStateChanged);

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


    _generalForm->setContentsMargins( { } );
    _generalForm->setMinimumSize( MaximalRightHandPaneSize );
    _generalForm->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _generalForm->setLayout(WrapWidgetsInVBoxDM(
        _offsetDisregardFirstLayer,
        _offsetSlider,
        _bedHeatingGroup,
        _projectImageButtonsGroup,
        nullptr
    ));
}

void AdvancedTab::_setUpTemperaturelForm(QFont boldFont)
{
    _currentTemperatureLabel->setText("Current temperature:");
    _targetTemperatureLabel->setText("Target temperature:");
    _heatingElementLabel->setText("Heating element:");
    _zPositionLabel->setText("Z position:");


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

    _temperatureForm->setContentsMargins( { } );
    _temperatureForm->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum );
    _temperatureForm->setLayout( WrapWidgetsInVBoxDM(
        WrapWidgetsInHBox( _currentTemperatureLabel, nullptr, _currentTemperature ),
        WrapWidgetsInHBox( _targetTemperatureLabel,  nullptr, _targetTemperature  ),
        WrapWidgetsInHBox( _heatingElementLabel,     nullptr, _heatingElement     ),
        WrapWidgetsInHBox( _zPositionLabel,          nullptr, _zPosition          )
    ) );
}

void AdvancedTab::_setUpBasePumpForm(QFont boldFont)
{
    _basePumpForm->setMinimumSize(QSize(MaximalRightHandPaneSize.width() + 35, MaximalRightHandPaneSize.height() + 25));
    _basePumpForm->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    QWidget* container = new QWidget();
    container->setMinimumSize(MaximalRightHandPaneSize);
    container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    QObject::connect( _addBasePumpCheckbox, &QCheckBox::stateChanged, this, &AdvancedTab::chbox_addBasePumpCheckChanged );

    _addBasePumpCheckbox->setFont(boldFont);
    _addBasePumpCheckbox->setChecked(true);

    QObject::connect( _addBasePumpCheckbox, &QCheckBox::stateChanged, this, &AdvancedTab::chbox_addBodyPumpChanged );

    QGroupBox* addBasePumpGroup = new QGroupBox();
    addBasePumpGroup->setLayout(WrapWidgetsInVBox(_addBasePumpCheckbox, nullptr));
    addBasePumpGroup->setContentsMargins( { } );


    container->setLayout(
        WrapWidgetsInVBox(
           addBasePumpGroup,
           _distanceSlider,
           _basePumpUpVelocitySlider,
           _upPauseSlider,
           _basePumpDownVelocitySlider,
           _downPauseSlider,
           _baseNoPumpUpVelocitySlider,
           nullptr
        ));

    _basePumpForm->setWidget(container);
}


void AdvancedTab::offsetDisregardFirstLayerStateChanged(int state)
{
    _printProfileManager->activeProfile()->setDisregardFirstLayerHeight(static_cast<bool>(state));
}

void AdvancedTab::_setUpBodyPumpForm(QFont boldFont)
{
    QWidget* container = new QWidget();

    _bodyPumpForm->setMinimumSize(QSize(MaximalRightHandPaneSize.width() + 35, MaximalRightHandPaneSize.height() + 25));
    _bodyPumpForm->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    container = new QWidget();
    container->setMinimumSize(MaximalRightHandPaneSize);
    container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    _addBodyPumpCheckbox->setFont(boldFont);
    _addBodyPumpCheckbox->setChecked(true);
    _bodyPumpEveryNthLayer->setEnabled(false);

    QObject::connect( _addBodyPumpCheckbox, &QCheckBox::stateChanged, this, &AdvancedTab::chbox_addBodyPumpChanged );

    QGroupBox* addBodyPumpGroup = new QGroupBox();
    addBodyPumpGroup->setLayout(WrapWidgetsInVBox(_addBodyPumpCheckbox, nullptr));
    addBodyPumpGroup->setContentsMargins( { } );

    container->setLayout(
         WrapWidgetsInVBox(
            addBodyPumpGroup,
            _bodyPumpEveryNthLayer,
            _bodyDistanceSlider,
            _bodyPumpUpVelocitySlider,
            _bodyUpPauseSlider,
            _bodyPumpDownVelocitySlider,
            _bodyDownPauseSlider,
            _bodyNoPumpUpVelocitySlider,
            nullptr
         )
    );

    _bodyPumpForm->setWidget(container);
}

void AdvancedTab::updatePrintProfile()
{
    if (_loadingPrintProfile)
        return;

    debug("+ AdvancedTab::updatePrintProfile\n");

    auto activeProfile = _printProfileManager->activeProfile();

    activeProfile->setBuildPlatformOffset(_offsetSlider->getValue());
    activeProfile->setDisregardFirstLayerHeight(_offsetDisregardFirstLayer->isChecked());
    activeProfile->setHeatingTemperature(_bedTemperatureSlider->value());

    PrintParameters baseParams = activeProfile->baseLayerParameters();

    baseParams.setPumpingEnabled(_addBasePumpCheckbox->isChecked());
    baseParams.setPumpUpDistance((static_cast<double>(_distanceSlider->getValue()) / 1000));
    baseParams.setPumpUpVelocity(_basePumpUpVelocitySlider->getValue());
    baseParams.setPumpUpPause(_upPauseSlider->getValue());
    baseParams.setPumpDownVelocity(_basePumpDownVelocitySlider->getValue());
    baseParams.setPumpDownPause(_downPauseSlider->getValue());
    baseParams.setNoPumpUpVelocity((static_cast<double>(_baseNoPumpUpVelocitySlider->getValue())));
    baseParams.setPumpEveryNthLayer(0);
    baseParams.setPowerLevel(_powerLevelSlider->value());

    activeProfile->setBaseLayerParameters(baseParams);

    PrintParameters bodyParams = activeProfile->bodyLayerParameters();
    bodyParams.setPumpingEnabled(_addBodyPumpCheckbox->isChecked());
    bodyParams.setPumpUpDistance((static_cast<double>(_bodyDistanceSlider->getValue()) / 1000 ));
    bodyParams.setPumpUpVelocity(_bodyPumpUpVelocitySlider->getValue());
    bodyParams.setPumpUpPause(_bodyUpPauseSlider->getValue());
    bodyParams.setPumpDownVelocity(_bodyPumpDownVelocitySlider->getValue());
    bodyParams.setPumpDownPause(_bodyDownPauseSlider->getValue());
    bodyParams.setNoPumpUpVelocity((static_cast<double>(_bodyNoPumpUpVelocitySlider->getValue())));
    bodyParams.setPumpEveryNthLayer(_bodyPumpEveryNthLayer->getValue());
    bodyParams.setPowerLevel(_powerLevelSlider->value());

    activeProfile->setBodyLayerParameters(bodyParams);
}

void AdvancedTab::loadPrintProfile(QSharedPointer<PrintProfile> profile)
{
    PrintParameters& baseParams = profile->baseLayerParameters();
    PrintParameters& bodyParams = profile->bodyLayerParameters();

    _loadingPrintProfile = true;
    _offsetSlider->setValue(profile->buildPlatformOffset());
    _offsetDisregardFirstLayer->setChecked(profile->disregardFirstLayerHeight());
    _bedTemperatureSlider->setValue(profile->heatingTemperature());
    _addBasePumpCheckbox->setChecked(profile->baseLayerParameters().isPumpingEnabled());
    _addBodyPumpCheckbox->setChecked(profile->bodyLayerParameters().isPumpingEnabled());

    _distanceSlider->setValue(static_cast<int>(baseParams.pumpUpDistance( ) * 1000.0));
    _basePumpUpVelocitySlider->setValue(static_cast<int>(baseParams.pumpUpVelocity_Effective()));
    _upPauseSlider->setValue(baseParams.pumpUpPause());
    _basePumpDownVelocitySlider->setValue(static_cast<int>(baseParams.pumpDownVelocity_Effective()));
    _downPauseSlider->setValue(baseParams.pumpDownPause());
    _baseNoPumpUpVelocitySlider->setValue(static_cast<int>(baseParams.noPumpUpVelocity()));
    _powerLevelSlider->setValue(static_cast<int>(baseParams.powerLevel()));

    _bodyDistanceSlider->setValue(static_cast<int>(bodyParams.pumpUpDistance() * 1000.0));
    _bodyPumpUpVelocitySlider->setValue(static_cast<int>(bodyParams.pumpUpVelocity_Effective()));
    _bodyUpPauseSlider->setValue(bodyParams.pumpUpPause());
    _bodyPumpDownVelocitySlider->setValue(static_cast<int>(bodyParams.pumpDownVelocity_Effective()));
    _bodyDownPauseSlider->setValue( bodyParams.pumpDownPause());
    _bodyNoPumpUpVelocitySlider->setValue(static_cast<int>(bodyParams.noPumpUpVelocity()));
    // assume body and base power level are the same for now
    //_bodyPowerLevelSlider->setValue( bodyParams.powerLevel( ) );
    _bodyPumpEveryNthLayer->setValue( bodyParams.pumpEveryNthLayer( ) );

    printJob.setPrintProfile(profile);
    _loadingPrintProfile = false;
}

void AdvancedTab::_setEnabled(bool enabled)
{
    _offsetDisregardFirstLayer->setEnabled(enabled);
    _offsetSlider->setEnabled(enabled);
    _basePumpForm->setEnabled(enabled);
    _bodyPumpForm->setEnabled(enabled);
}

void AdvancedTab::printJobChanged() {

}
