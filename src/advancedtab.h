#ifndef __ADVANCEDTAB_H__
#define __ADVANCEDTAB_H__

#include "tabbase.h"

class AdvancedTab: public TabBase {

    Q_OBJECT

public:

    AdvancedTab( QWidget* parent = nullptr );
    virtual ~AdvancedTab( ) override;

protected:

    virtual void _connectShepherd( ) override;

private:

    QLabel*      _currentTemperatureLabel   { new QLabel      };
    QLabel*      _targetTemperatureLabel    { new QLabel      };
    QLabel*      _pwmLabel                  { new QLabel      };
    QLabel*      _zPositionLabel            { new QLabel      };

    QLabel*      _currentTemperature        { new QLabel      };
    QLabel*      _targetTemperature         { new QLabel      };
    QLabel*      _pwm                       { new QLabel      };
    QLabel*      _zPosition                 { new QLabel      };

    QPushButton* _bedHeatingButton          { new QPushButton };
    QLabel*      _bedHeatingButtonLabel     { new QLabel      };
    QHBoxLayout* _bedHeatingButtonLayout    {                 };

    QLabel*      _bedTemperatureLabel       { new QLabel      };
    QLabel*      _bedTemperatureValue       { new QLabel      };
    QHBoxLayout* _bedTemperatureValueLayout {                 };
    QSlider*     _bedTemperatureSlider      { new QSlider     };
    QVBoxLayout* _bedTemperatureLayout      { new QVBoxLayout };

    QGroupBox*   _bedHeatingGroup           { new QGroupBox   };

    QWidget*     _leftColumn                { new QWidget     };
    QGroupBox*   _rightColumn               { new QGroupBox   };

    QVBoxLayout* _leftColumnLayout          { };
    QVBoxLayout* _rightColumnLayout         { };
    QHBoxLayout* _layout                    { };

signals:

public slots:

    void printer_positionReport( double const px, double const py, double const pz, double const pe, double const cx, double const cy, double const cz );
    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void printBedHeatingButton_clicked( bool checked );
    void printBedTemperatureSlider_valueChanged( int value );

    void shepherd_sendComplete( bool const success );

protected slots:

private slots:

};

#endif // __ADVANCEDTAB_H__
