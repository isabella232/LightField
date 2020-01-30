#ifndef __ADVANCEDTAB_H__
#define __ADVANCEDTAB_H__

#define ENABLE_TEMPERATURE_SETTING

#include "tabbase.h"
#include "paramslider.h"

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

    QLabel*       _offsetLabel                     { new QLabel      };
    QLabel*       _offsetValue                     { new QLabel      };
    QSlider*      _offsetSlider                    { new QSlider     };

    QGroupBox*    _buildPlatformOffsetGroup        { new QGroupBox   };

    QPushButton*  _bedHeatingButton                { new QPushButton };
    QLabel*       _bedHeatingButtonLabel           { new QLabel      };

    QPushButton*  _projectBlankImageButton         { new QPushButton };
    QLabel*       _projectBlankImageButtonLabel    { new QLabel      };

    QPushButton*  _projectFocusImageButton         { new QPushButton };
    QLabel*       _projectFocusImageButtonLabel    { new QLabel      };

    QLabel*       _powerLevelLabel                 { new QLabel      };
    QLabel*       _powerLevelValue                 { new QLabel      };
    QHBoxLayout*  _powerLevelValueLayout           {                 };
    QSlider*      _powerLevelSlider                { new QSlider     };

    QGroupBox*    _projectImageButtonsGroup        { new QGroupBox   };

    QWidget*      _leftColumn                      { new QWidget     };
    QWidget*      _rightColumn                     { new QWidget     };

    //Left panel
    QListView*    _leftMenu                        { new QListView   };

    //General form
    QGroupBox*    _generalForm                     { new QGroupBox   };

    QGroupBox*    _bedHeatingGroup                 { new QGroupBox   };

    //Temperature form
    QWidget*      _temperatureForm                  { new QWidget     };

    QLabel*       _currentTemperatureLabel         { new QLabel      };
    QLabel*       _targetTemperatureLabel          { new QLabel      };
    QLabel*       _heatingElementLabel             { new QLabel      };
    QLabel*       _zPositionLabel                  { new QLabel      };

    QLabel*       _currentTemperature              { new QLabel      };
    QLabel*       _targetTemperature               { new QLabel      };
    QLabel*       _heatingElement                  { new QLabel      };
    QLabel*       _zPosition                       { new QLabel      };

#if defined ENABLE_TEMPERATURE_SETTING
    QLabel*       _bedTemperatureLabel             { new QLabel      };
    QLabel*       _bedTemperatureValue             { new QLabel      };
    QHBoxLayout*  _bedTemperatureValueLayout       {                 };
    QSlider*      _bedTemperatureSlider            { new QSlider     };
#endif

    //Base Pump Form
    QScrollArea*  _basePumpForm                    { new QScrollArea     };
    QCheckBox*    _addBasePumpCheckbox             { new QCheckBox("add base pump") };

    ParamSlider*  _distanceSlider                  { new ParamSlider("Base Pump Distance", "µm", 1000, 2000, 1)     };
    ParamSlider*  _upTimeSlider                    { new ParamSlider("Base Pump Up Time", "ms", 1000, 2000, 1)      };
    ParamSlider*  _upPauseSlider                   { new ParamSlider("Base Pump Up Pause", "ms", 1000, 2000, 1)     };
    ParamSlider*  _downTimeSlider                  { new ParamSlider("Base Pump Down Time", "ms", 1000, 2000, 1)    };
    ParamSlider*  _downPauseSlider                 { new ParamSlider("Base Pump Down Pause", "ms", 1000, 2000, 1)   };
    ParamSlider*  _upVelocitySlider                { new ParamSlider("Base Pump Up Velocity", "µm/ms", 1000, 2000, 1) };


    //Base Layer Form
    QWidget*      _baseLayerForm                   { new QWidget     };

    ParamSlider*  _numberOfBaseLayersSlider        { new ParamSlider("Number of Base Layer", "", 1, 20, 1)          };
    ParamSlider*  _baseThicknessSlider             { new ParamSlider("Base Layer Thickness", "µm", 100, 1000, 1)    };
    ParamSlider*  _baseExposureTimeSlider          { new ParamSlider("Base Pump Up Pause", "ms", 2000, 10000, 1)    };

    //Body Layers Form
    QWidget*      _bodyLayersForm                  { new QWidget     };

    ParamSlider*  _bodyThicknessSlider             { new ParamSlider("Body Layer Thickness", "µm", 20, 1000, 1)    };
    ParamSlider*  _bodyExposureTimeSlider          { new ParamSlider("Base Pump Up Pause", "ms", 2000, 10000, 1)    };

    //Body Pump Form
    QScrollArea*  _bodyPumpForm                    { new QScrollArea     };
    QCheckBox*    _addBodyPumpCheckbox             { new QCheckBox("add body pump") };

    ParamSlider*  _bodyPumpEveryNthLayer           { new ParamSlider("Body Pump Distance", "µm", 5, 40, 1)          };
    ParamSlider*  _bodyDistanceSlider              { new ParamSlider("Body Pump Distance", "µm", 1000, 2000, 1)     };
    ParamSlider*  _bodyUpTimeSlider                { new ParamSlider("Body Pump Up Time", "ms", 1000, 2000, 1)      };
    ParamSlider*  _bodyUpPauseSlider               { new ParamSlider("Body Pump Up Pause", "ms", 1000, 2000, 1)     };
    ParamSlider*  _bodyDownTimeSlider              { new ParamSlider("Body Pump Down Time", "ms", 1000, 2000, 1)    };
    ParamSlider*  _bodyDownPauseSlider             { new ParamSlider("Body Pump Down Pause", "ms", 1000, 2000, 1)   };
    ParamSlider*  _bodyUpVelocitySlider            { new ParamSlider("Body Pump Up Velocity", "µm/ms", 1000, 2000, 1) };


    static const int       FORMS_COUNT { 6 };
    QWidget*        _forms[FORMS_COUNT];

    PngDisplayer* _pngDisplayer                    { };

    bool          _isPrinterOnline                 { false };
    bool          _isPrinterAvailable              { true  };
    bool          _isProjectorOn                   { false };

    void          _updateControlGroups( );
    void          _projectImage( char const* fileName );
    void          _setupLeftMenu(QFont fontAwesome);
    void          _setupGeneralForm(QFont fontBold, QFont fontAwesome);
    void          _setupTemperaturelForm(QFont fontBold);
    void          _setupBasePumpForm(QFont fontBold);
    void          _setupBaseLayerForm();
    void          _setupBodyLayersForm();
    void          _setupBodyPumpForm(QFont fontBold);
signals:
    ;

    void printerAvailabilityChanged( bool const available );
    void projectorPowerLevelChanged( int const value );

public slots:
    ;

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void setPngDisplayer( PngDisplayer* pngDisplayer );
    void setPrinterAvailable( bool const value );

    void projectorPowerLevel_changed( int const percentage );

protected slots:
    ;

private slots:
    ;

    void printer_online( );
    void printer_offline( );

    void printer_positionReport( double const px, int const cx );
    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void offsetSlider_sliderReleased( );
    void offsetSlider_valueChanged( int value);

    void printBedHeatingButton_clicked( bool checked );
#if defined ENABLE_TEMPERATURE_SETTING
    void printBedTemperatureSlider_sliderReleased( );
    void printBedTemperatureSlider_valueChanged( int value );
#endif

    void projectBlankImageButton_clicked( bool checked );
    void projectFocusImageButton_clicked( bool checked );

    void powerLevelSlider_sliderReleased( );
    void powerLevelSlider_valueChanged( int percentage );

    void shepherd_sendComplete( bool const success );

    void chbox_addBodyPumpChanged(int);

    void chbox_addBasePumpCheckChanged(int state);

};

#endif // __ADVANCEDTAB_H__
