#include "pch.h"

#include "advancedtab.h"

#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"
#include "utils.h"

namespace {

    auto TemperaturePollInterval = 5000; // ms

}

AdvancedTab::AdvancedTab( QWidget* parent ): QWidget( parent ) {
    _currentTemperatureLabel->setText( "Current temperature:"   );
    _targetTemperatureLabel ->setText( "Target temperature:"    );
    _pwmLabel               ->setText( "Heater PWM duty cycle:" );
    _zPositionLabel         ->setText( "Z position:"            );

    _currentTemperature->setAlignment( Qt::AlignRight );
    _targetTemperature ->setAlignment( Qt::AlignRight );
    _pwm               ->setAlignment( Qt::AlignRight );
    _zPosition         ->setAlignment( Qt::AlignRight );

    _timer->setInterval( TemperaturePollInterval );
    _timer->setSingleShot( false );
    _timer->setTimerType( Qt::PreciseTimer );
    _resumeTimer( );

    auto layout = new QVBoxLayout( this );
    layout->addLayout( WrapWidgetsInHBox( { _currentTemperatureLabel, nullptr, _currentTemperature } ) );
    layout->addLayout( WrapWidgetsInHBox( { _targetTemperatureLabel,  nullptr, _targetTemperature  } ) );
    layout->addLayout( WrapWidgetsInHBox( { _pwmLabel,                nullptr, _pwm                } ) );
    layout->addLayout( WrapWidgetsInHBox( { _zPositionLabel,          nullptr, _zPosition          } ) );
    layout->addStretch( );

    setLayout( layout );
}

AdvancedTab::~AdvancedTab( ) {
    /*empty*/
}

void AdvancedTab::_pauseTimer( ) {
    QObject::disconnect( _timer, nullptr, this, nullptr );
    _timer->start( );
}

void AdvancedTab::_resumeTimer( ) {
    QObject::connect( _timer, &QTimer::timeout, this, &AdvancedTab::timer_pollTemperature );
    _timer->start( );
}

void AdvancedTab::setShepherd( Shepherd* newShepherd ) {
    if ( _shepherd ) {
        QObject::disconnect( _shepherd, nullptr, this, nullptr );
    }

    _shepherd = newShepherd;

    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_temperatureReport, this, &AdvancedTab::printer_temperatureReport );
    }
}

void AdvancedTab::setPrintManager( PrintManager* printManager ) {
    if ( _printManager ) {
        QObject::disconnect( _printManager, nullptr, this, nullptr );
    }

    _printManager = printManager;

    if ( _printManager ) {
        QObject::connect( _printManager, &PrintManager::printStarting, this, &AdvancedTab::printManager_printStarting );
        QObject::connect( _printManager, &PrintManager::printComplete, this, &AdvancedTab::printManager_printComplete );
        QObject::connect( _printManager, &PrintManager::printAborted,  this, &AdvancedTab::printManager_printAborted  );
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

void AdvancedTab::printManager_printStarting( ) {
    debug( "+ AdvancedTab::printManager_printStarting: pausing timer\n" );
    _pauseTimer( );
}

void AdvancedTab::printManager_printComplete( bool const success ) {
    debug( "+ AdvancedTab::printManager_printComplete: resuming timer\n" );
    _resumeTimer( );
}

void AdvancedTab::printManager_printAborted( ) {
    debug( "+ AdvancedTab::printManager_printAborted: resuming timer\n" );
    _resumeTimer( );
}

void AdvancedTab::timer_pollTemperature( ) {
    _shepherd->doSend( QString { "M105" } );
}
