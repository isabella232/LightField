#include "pch.h"

#include "advancedtab.h"

#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"
#include "strings.h"
#include "utils.h"

namespace {

    auto DefaultPrintBedTemperature = 30;

    QChar FA_Check { L'\uF00C' };
    QChar FA_Times { L'\uF00D' };

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


    _printBedHeatingButton->setCheckable( true );
    _printBedHeatingButton->setChecked( false );
    _printBedHeatingButton->setFont( fontAwesome );
    _printBedHeatingButton->setFixedSize( 37, 38 );
    _printBedHeatingButton->setText( FA_Times );
    QObject::connect( _printBedHeatingButton, &QPushButton::clicked, this, &AdvancedTab::printBedHeatingButton_clicked );

    _printBedHeatingLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    _printBedHeatingLabel->setText( "Print bed heating" );

    _printBedHeatingLayout = WrapWidgetsInHBox( { _printBedHeatingButton, _printBedHeatingLabel, nullptr } );
    _printBedHeatingLayout->setContentsMargins( { } );

    _printBedTemperatureLabel->setText( "Print bed temperature:" );

    _printBedTemperatureValue->setAlignment( Qt::AlignRight );
    _printBedTemperatureValue->setFont( boldFont );
    _printBedTemperatureValue->setText( QString { "%1 °C" }.arg( DefaultPrintBedTemperature ) );

    _printBedTemperatureValueLayout = WrapWidgetsInHBox( { _printBedTemperatureLabel, nullptr, _printBedTemperatureValue } );
    _printBedTemperatureValueLayout->setContentsMargins( { } );

    _printBedTemperatureSlider->setMinimum( 30 );
    _printBedTemperatureSlider->setMaximum( 50 );
    _printBedTemperatureSlider->setOrientation( Qt::Horizontal );
    _printBedTemperatureSlider->setTickInterval( 1 );
    _printBedTemperatureSlider->setTickPosition( QSlider::TicksBothSides );
    _printBedTemperatureSlider->setValue( DefaultPrintBedTemperature );
    QObject::connect( _printBedTemperatureSlider, &QSlider::valueChanged, this, &AdvancedTab::printBedTemperatureSlider_valueChanged );

    _printBedTemperatureLabel->setEnabled( false );
    _printBedTemperatureSlider->setEnabled( false );
    _printBedTemperatureValue->setEnabled( false );
    _printBedTemperatureValueLayout->setEnabled( false );


    _rightColumnLayout = new QVBoxLayout { this };
    _rightColumnLayout->addLayout( _printBedHeatingLayout );
    _rightColumnLayout->addLayout( _printBedTemperatureValueLayout );
    _rightColumnLayout->addWidget( _printBedTemperatureSlider );
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
    _printBedHeatingButton->setText( checked ? FA_Check : FA_Times );
    _printBedTemperatureLabel->setEnabled( checked );
    _printBedTemperatureSlider->setEnabled( checked );
    _printBedTemperatureValue->setEnabled( checked );
    _printBedTemperatureValueLayout->setEnabled( checked );

    if ( checked ) {
        _shepherd->doSend( QString { "M104 S%d" }.arg( _printBedTemperatureSlider->value( ) ) );
    } else {
        _shepherd->doSend( QString { "M104" } );
    }
}

void AdvancedTab::printBedTemperatureSlider_valueChanged( int value ) {
    debug( "+ AdvancedTab::printBedTemperatureSlider_valueChanged: new value %d °C\n", value );
    _printBedTemperatureValue->setText( QString { "%1 °C" }.arg( value ) );

    _shepherd->doSend( QString { "M104 S%d" }.arg( value ) );
}

void AdvancedTab::shepherd_sendComplete( bool const success ) {
    debug( "+ AdvancedTab::shepherd_sendComplete: action %s\n", ToString( success ) );
}
