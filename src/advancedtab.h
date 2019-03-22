#ifndef __ADVANCEDTAB_H__
#define __ADVANCEDTAB_H__

#include "tabbase.h"

class AdvancedTab: public TabBase {

    Q_OBJECT

public:

    AdvancedTab( QWidget* parent = nullptr );
    virtual ~AdvancedTab( ) override;

protected:

    virtual void _connectShepherd( )     override;

private:

    QLabel*      _currentTemperatureLabel { new QLabel  };
    QLabel*      _targetTemperatureLabel  { new QLabel  };
    QLabel*      _pwmLabel                { new QLabel  };
    QLabel*      _zPositionLabel          { new QLabel  };

    QLabel*      _currentTemperature      { new QLabel  };
    QLabel*      _targetTemperature       { new QLabel  };
    QLabel*      _pwm                     { new QLabel  };
    QLabel*      _zPosition               { new QLabel  };

    QWidget*     _leftColumn              { new QWidget };
    QWidget*     _rightColumn             { new QWidget };

    QVBoxLayout* _leftColumnLayout        { };
    QVBoxLayout* _rightColumnLayout       { };
    QHBoxLayout* _layout                  { };

signals:

public slots:

    void printer_positionReport( double const px, double const py, double const pz, double const pe, double const cx, double const cy, double const cz );
    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

protected slots:

private slots:

};

#endif // __ADVANCEDTAB_H__
