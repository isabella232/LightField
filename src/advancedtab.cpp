#include "pch.h"

#include "advancedtab.h"

#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"
#include "strings.h"
#include "utils.h"

namespace {

    auto DefaultPrintBedTemperature = 30;

}

AdvancedTab::AdvancedTab( QWidget* parent ): TabBase( parent ) {
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto fontAwesome = ModifyFont( ModifyFont( origFont, "FontAwesome" ), 22.0 );


    _currentTemperatureLabel->setText( "Current temperature:" );
    _targetTemperatureLabel ->setText( "Target temperature:"  );
    _pwmLabel               ->setText( "Heater PWM:"          );
    _zPositionLabel         ->setText( "Z position:"          );


    _currentTemperature->setAlignment( Qt::AlignRight );
    _currentTemperature->setFont( boldFont );
    _currentTemperature->setText( EmDash );

    _targetTemperature ->setAlignment( Qt::AlignRight );
    _targetTemperature ->setFont( boldFont );
    _targetTemperature ->setText( EmDash );

    _pwm               ->setAlignment( Qt::AlignRight );
    _pwm               ->setFont( boldFont );
    _pwm               ->setText( EmDash );

    _zPosition         ->setAlignment( Qt::AlignRight );
    _zPosition         ->setFont( boldFont );
    _zPosition         ->setText( EmDash );


    _leftColumnLayout  = new QVBoxLayout { this };
    _leftColumnLayout->addLayout( WrapWidgetsInHBox( { _currentTemperatureLabel, nullptr, _currentTemperature } ) );
    _leftColumnLayout->addLayout( WrapWidgetsInHBox( { _targetTemperatureLabel,  nullptr, _targetTemperature  } ) );
    _leftColumnLayout->addLayout( WrapWidgetsInHBox( { _pwmLabel,                nullptr, _pwm                } ) );
    _leftColumnLayout->addLayout( WrapWidgetsInHBox( { _zPositionLabel,          nullptr, _zPosition          } ) );
    _leftColumnLayout->addStretch( );

    _leftColumn->setContentsMargins( { } );
    _leftColumn->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _leftColumn->setFixedWidth( MainButtonSize.width( ) );
    _leftColumn->setLayout( _leftColumnLayout );


    _bedHeatingButton->setCheckable( true );
    _bedHeatingButton->setChecked( false );
    _bedHeatingButton->setFont( fontAwesome );
    _bedHeatingButton->setFixedSize( 37, 38 );
    _bedHeatingButton->setText( FA_Times );
    QObject::connect( _bedHeatingButton, &QPushButton::clicked, this, &AdvancedTab::printBedHeatingButton_clicked );

    _bedHeatingButtonLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    _bedHeatingButtonLabel->setText( "Print bed heating" );

    _bedHeatingButtonLayout = WrapWidgetsInHBox( { _bedHeatingButton, _bedHeatingButtonLabel, nullptr } );
    _bedHeatingButtonLayout->setContentsMargins( { } );

    _bedTemperatureLabel->setText( "Print bed temperature:" );

    _bedTemperatureValue->setAlignment( Qt::AlignRight );
    _bedTemperatureValue->setFont( boldFont );
    _bedTemperatureValue->setText( QString { "%1 °C" }.arg( DefaultPrintBedTemperature ) );

    _bedTemperatureValueLayout = WrapWidgetsInHBox( { _bedTemperatureLabel, nullptr, _bedTemperatureValue } );
    _bedTemperatureValueLayout->setContentsMargins( { } );

    _bedTemperatureSlider->setMinimum( 30 );
    _bedTemperatureSlider->setMaximum( 50 );
    _bedTemperatureSlider->setOrientation( Qt::Horizontal );
    _bedTemperatureSlider->setPageStep( 5 );
    _bedTemperatureSlider->setSingleStep( 1 );
    _bedTemperatureSlider->setTickInterval( 5 );
    _bedTemperatureSlider->setTickPosition( QSlider::TicksBothSides );
    _bedTemperatureSlider->setValue( DefaultPrintBedTemperature );
    QObject::connect( _bedTemperatureSlider, &QSlider::valueChanged, this, &AdvancedTab::printBedTemperatureSlider_valueChanged );

    _bedTemperatureLabel->setEnabled( false );
    _bedTemperatureSlider->setEnabled( false );
    _bedTemperatureValue->setEnabled( false );
    _bedTemperatureValueLayout->setEnabled( false );


    _bedTemperatureLayout->addLayout( _bedHeatingButtonLayout );
    _bedTemperatureLayout->addLayout( _bedTemperatureValueLayout );
    _bedTemperatureLayout->addWidget( _bedTemperatureSlider );

    _bedHeatingGroup->setContentsMargins( { } );
    _bedHeatingGroup->setLayout( _bedTemperatureLayout );


    _rightColumnLayout = new QVBoxLayout { this };
    _rightColumnLayout->addWidget( _bedHeatingGroup );
    _rightColumnLayout->addStretch( );

    _rightColumn->setContentsMargins( { } );
    _rightColumn->setMinimumSize( MaximalRightHandPaneSize );
    _rightColumn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _rightColumn->setLayout( _rightColumnLayout );


    _layout = WrapWidgetsInHBox( { _leftColumn, _rightColumn } );
    _layout->setContentsMargins( { } );

    setLayout( _layout );
}

AdvancedTab::~AdvancedTab( ) {
    /*empty*/
}

void AdvancedTab::_connectShepherd( ) {
    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_positionReport,    this, &AdvancedTab::printer_positionReport    );
        QObject::connect( _shepherd, &Shepherd::printer_temperatureReport, this, &AdvancedTab::printer_temperatureReport );
        QObject::connect( _shepherd, &Shepherd::action_sendComplete,       this, &AdvancedTab::shepherd_sendComplete     );
    }
}

void AdvancedTab::printer_positionReport( double const px, double const py, double const pz, double const pe, double const cx, double const cy, double const cz ) {
    debug( "AdvancedTab::printer_positionReport: px %.2f mm, cx %d counts\n", px, cx );
    _zPosition->setText( QString { "%1 mm" }.arg( px, 0, 'f', 2 ) );
}

void AdvancedTab::printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm ) {
    debug(
        "+ AdvancedTab::printer_temperatureReport:\n"
        "  + current temperature: %.2f °C\n"
        "  + target temperature:  %.2f °C\n"
        "  + PWM:                 %d\n"
        "",
        bedCurrentTemperature,
        bedTargetTemperature,
        bedPwm
    );

    _currentTemperature->setText( QString( "%1 °C" ).arg( bedCurrentTemperature, 0, 'f', 2 ) );
    _targetTemperature ->setText( QString( "%1 °C" ).arg( bedTargetTemperature,  0, 'f', 2 ) );
    _pwm               ->setText( QString( "%1"    ).arg( bedPwm                           ) );
}

void AdvancedTab::printBedHeatingButton_clicked( bool checked ) {
    _bedHeatingButton->setText( checked ? FA_Check : FA_Times );
    _bedTemperatureLabel->setEnabled( checked );
    _bedTemperatureSlider->setEnabled( checked );
    _bedTemperatureValue->setEnabled( checked );
    _bedTemperatureValueLayout->setEnabled( checked );

    if ( checked ) {
        _shepherd->doSend( QString { "M104 S%d" }.arg( _bedTemperatureSlider->value( ) ) );
    } else {
        _shepherd->doSend( QString { "M104" } );
    }
}

void AdvancedTab::printBedTemperatureSlider_valueChanged( int value ) {
    debug( "+ AdvancedTab::printBedTemperatureSlider_valueChanged: new value %d °C\n", value );
    _bedTemperatureValue->setText( QString { "%1 °C" }.arg( value ) );

    _shepherd->doSend( QString { "M104 S%d" }.arg( value ) );
}

void AdvancedTab::shepherd_sendComplete( bool const success ) {
    debug( "+ AdvancedTab::shepherd_sendComplete: action %s\n", ToString( success ) );
}
