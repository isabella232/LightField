#ifndef __ADVANCEDTAB_H__
#define __ADVANCEDTAB_H__

#undef ENABLE_TEMPERATURE_SETTING

#include "tabbase.h"

class PngDisplayer;

class AdvancedTab: public TabBase {

    Q_OBJECT

public:

    AdvancedTab( QWidget* parent = nullptr );
    virtual ~AdvancedTab( ) override;

    virtual TabIndex tabIndex( ) const override { return TabIndex::Advanced; }

protected:

    virtual void _connectShepherd( ) override;

private:

    QLabel*       _currentTemperatureLabel         { new QLabel      };
    QLabel*       _targetTemperatureLabel          { new QLabel      };
    QLabel*       _pwmLabel                        { new QLabel      };
    QLabel*       _zPositionLabel                  { new QLabel      };

    QLabel*       _currentTemperature              { new QLabel      };
    QLabel*       _targetTemperature               { new QLabel      };
    QLabel*       _pwm                             { new QLabel      };
    QLabel*       _zPosition                       { new QLabel      };

    QPushButton*  _bedHeatingButton                { new QPushButton };
    QLabel*       _bedHeatingButtonLabel           { new QLabel      };
    QHBoxLayout*  _bedHeatingButtonLayout          {                 };

#if defined ENABLE_TEMPERATURE_SETTING
    QLabel*       _bedTemperatureLabel             { new QLabel      };
    QLabel*       _bedTemperatureValue             { new QLabel      };
    QHBoxLayout*  _bedTemperatureValueLayout       {                 };
    QSlider*      _bedTemperatureSlider            { new QSlider     };
#endif
    QVBoxLayout*  _bedTemperatureLayout            { new QVBoxLayout };

    QGroupBox*    _bedHeatingGroup                 { new QGroupBox   };

    QPushButton*  _projectorFloodlightButton       { new QPushButton };
    QLabel*       _projectorFloodlightButtonLabel  { new QLabel      };
    QHBoxLayout*  _projectorFloodlightButtonLayout {                 };

    QLabel*       _powerLevelLabel                 { new QLabel      };
    QLabel*       _powerLevelValue                 { new QLabel      };
    QHBoxLayout*  _powerLevelValueLayout           {                 };
    QSlider*      _powerLevelSlider                { new QSlider     };
    QVBoxLayout*  _powerLevelLayout                { new QVBoxLayout };

    QGroupBox*    _projectorFloodlightGroup        { new QGroupBox   };

    QWidget*      _leftColumn                      { new QWidget     };
    QGroupBox*    _rightColumn                     { new QGroupBox   };

    QVBoxLayout*  _leftColumnLayout                { };
    QVBoxLayout*  _rightColumnLayout               { };
    QHBoxLayout*  _layout                          { };

    PngDisplayer* _pngDisplayer                    { };

    bool          _isPrinterOnline                 { false };
    bool          _isPrinterAvailable              { true  };
    bool          _isFloodlightOn                  { false };

    void          _updateControlGroups( );

signals:

    void printerAvailabilityChanged( bool const available );

    void projectorPowerLevelChanged( int const value );

public slots:

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void setPngDisplayer( PngDisplayer* pngDisplayer );
    void setPrinterAvailable( bool const value );

    void projectorPowerLevel_changed( int const percentage );

protected slots:

private slots:

    void printer_online( );
    void printer_offline( );

    void printer_positionReport( double const px, double const py, double const pz, double const pe, double const cx, double const cy, double const cz );
    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void printBedHeatingButton_clicked( bool checked );
#if defined ENABLE_TEMPERATURE_SETTING
    void printBedTemperatureSlider_sliderReleased( );
    void printBedTemperatureSlider_valueChanged( int value );
#endif

    void projectorFloodlightButton_clicked( bool checked );

    void powerLevelSlider_sliderReleased( );
    void powerLevelSlider_valueChanged( int percentage );

    void shepherd_sendComplete( bool const success );

};

#endif // __ADVANCEDTAB_H__
