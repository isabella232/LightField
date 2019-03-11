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
    QObject::connect( _timer, &QTimer::timeout, this, &AdvancedTab::timer_pollTemperature );
    _timer->setInterval( TemperaturePollInterval );
    _timer->setSingleShot( false );
    _timer->setTimerType( Qt::PreciseTimer );
    _timer->start( );

    setLayout( WrapWidgetsInVBox( { _currentTemperature, _targetTemperature, _pwm, nullptr } ) );
}

AdvancedTab::~AdvancedTab( ) {
    /*empty*/
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

void AdvancedTab::printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm ) {
    _currentTemperature->setText( QString::asprintf( "Current temperature: %.2f °C", bedCurrentTemperature ) );
    _targetTemperature ->setText( QString::asprintf( "Target temperature: %.2f °C",  bedTargetTemperature  ) );
    _pwm               ->setText( QString::asprintf( "PWM: %d",                      bedPwm                ) );
}

void AdvancedTab::printManager_printStarting( ) {
    _timer->stop( );
}

void AdvancedTab::printManager_printComplete( bool const success ) {
    _timer->start( );
}

void AdvancedTab::printManager_printAborted( ) {
    _timer->start( );
}

void AdvancedTab::timer_pollTemperature( ) {
    _shepherd->doSend( QString { "M105" } );
}
