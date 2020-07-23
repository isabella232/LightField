#ifndef __ADVANCEDTAB_H__
#define __ADVANCEDTAB_H__

#define ENABLE_TEMPERATURE_SETTING

#include <QtCore>
#include <QtWidgets>
#include "tabbase.h"
#include "paramslider.h"
#include "printprofile.h"
#include "printprofilemanager.h"
class PngDisplayer;

class AdvancedTab: public TabBase
{
    Q_OBJECT

public:

    AdvancedTab(QWidget* parent = nullptr);
    virtual ~AdvancedTab() override;
    virtual TabIndex tabIndex() const override
    {
        return TabIndex::Advanced;
    }

protected:
    virtual void _connectShepherd() override;

private:
    QCheckBox* _offsetDisregardFirstLayer { new QCheckBox("Disregard first layer height") };
    ParamSlider* _offsetSlider { new ParamSlider("Build platform offset", "µm", 0, 1000, 50, 0) };

    QPushButton*  _bedHeatingButton                   { new QPushButton };
    QLabel*       _bedHeatingButtonLabel              { new QLabel      };

    QPushButton*  _projectBlankImageButton            { new QPushButton };
    QLabel*       _projectBlankImageButtonLabel       { new QLabel      };

    QPushButton*  _projectFocusImageButton            { new QPushButton };
    QLabel*       _projectFocusImageButtonLabel       { new QLabel      };

    QLabel*       _powerLevelLabel                    { new QLabel      };
    QLabel*       _powerLevelValue                    { new QLabel      };
    QHBoxLayout*  _powerLevelValueLayout              {                 };
    QSlider*      _powerLevelSlider                   { new QSlider     };

    QGroupBox*    _projectImageButtonsGroup           { new QGroupBox   };

    QWidget*      _leftColumn                         { new QWidget     };
    QWidget*      _rightColumn                        { new QWidget     };

    //Left panel
    QListView*    _leftMenu                           { new QListView   };

    //General form
    QGroupBox*    _generalForm                        { new QGroupBox   };

    QGroupBox*    _bedHeatingGroup                    { new QGroupBox   };

    //Temperature form
    QWidget*      _temperatureForm                    { new QWidget     };

    QLabel*       _currentTemperatureLabel            { new QLabel      };
    QLabel*       _targetTemperatureLabel             { new QLabel      };
    QLabel*       _heatingElementLabel                { new QLabel      };
    QLabel*       _zPositionLabel                     { new QLabel      };

    QLabel*       _currentTemperature                 { new QLabel      };
    QLabel*       _targetTemperature                  { new QLabel      };
    QLabel*       _heatingElement                     { new QLabel      };
    QLabel*       _zPosition                          { new QLabel      };

#if defined ENABLE_TEMPERATURE_SETTING
    QLabel*       _bedTemperatureLabel                { new QLabel      };
    QLabel*       _bedTemperatureValue                { new QLabel      };
    QHBoxLayout*  _bedTemperatureValueLayout          {                 };
    QSlider*      _bedTemperatureSlider               { new QSlider     };
#endif

    //Base Pump Form
    QScrollArea*  _basePumpForm                       { new QScrollArea                                                          };
    QCheckBox*    _addBasePumpCheckbox                { new QCheckBox( "Enable pumping for base layers" )                        };

    ParamSlider*  _distanceSlider                     { new ParamSlider( "Base Pump Distance",        "µm",    1000, 8000, 250, 250 ) };
    ParamSlider*  _basePumpUpVelocitySlider           { new ParamSlider( "Base Pump Up Speed",        "mm/min",   5,   50,   5, 5 ) };
    ParamSlider*  _basePumpDownVelocitySlider         { new ParamSlider( "Base Pump Down Speed",      "mm/min",   5,   50,   5, 5 ) };
    ParamSlider*  _upPauseSlider                      { new ParamSlider( "Base Pump Up Pause",        "ms",    1000, 8000,   50, 50 ) };
    ParamSlider*  _downPauseSlider                    { new ParamSlider( "Base Pump Down Pause",      "ms",    1000, 8000,   50, 50 ) };
    ParamSlider*  _baseNoPumpUpVelocitySlider         { new ParamSlider( "Base Prepare Speed",        "mm/min",   5, 50,     5, 5 ) };

    //Body Pump Form
    QScrollArea*  _bodyPumpForm                       { new QScrollArea                                                          };
    QCheckBox*    _addBodyPumpCheckbox                { new QCheckBox( "Enable pumping for body layers" )                        };

    ParamSlider*  _bodyPumpEveryNthLayer              { new ParamSlider( "Body Pump Every Nth Layer", "",         5,   20,   1 ) };
    ParamSlider*  _bodyDistanceSlider                 { new ParamSlider( "Body Pump Distance",        "µm",    1000, 8000, 250, 250 ) };
    ParamSlider*  _bodyPumpUpVelocitySlider           { new ParamSlider( "Body Pump Up Speed",        "mm/min",   5,   50,   5,      5 ) };
    ParamSlider*  _bodyPumpDownVelocitySlider         { new ParamSlider( "Body Pump Down Speed",      "mm/min",   5,   50,   5,      5 ) };
    ParamSlider*  _bodyUpPauseSlider                  { new ParamSlider( "Body Pump Up Pause",        "ms",    1000, 8000,   50, 50 ) };
    ParamSlider*  _bodyDownPauseSlider                { new ParamSlider( "Body Pump Down Pause",      "ms",    1000, 8000,   50, 50) };
    ParamSlider*  _bodyNoPumpUpVelocitySlider         { new ParamSlider( "Body Prepare Speed",        "mm/min",   5,   50,   5,      5 ) };

    static const int FORMS_COUNT                      { 3 };
    QWidget*         _forms[FORMS_COUNT];

    PngDisplayer*    _pngDisplayer                    { };

    bool             _isPrinterOnline                 { false };
    bool             _isPrinterAvailable              { true  };
    bool             _isProjectorOn                   { false };
    std::atomic_bool _loadingPrintProfile             { false };

    void _updateControlGroups( );
    void _projectImage( char const* fileName );
    void _setUpLeftMenu( QFont fontAwesome );
    void _setUpGeneralForm( QFont fontBold, QFont fontAwesome );
    void _setUpTemperaturelForm( QFont fontBold );
    void _setUpBasePumpForm( QFont fontBold );
    void _setUpBodyPumpForm( QFont fontBold );
    void _setEnabled( bool enabled );

signals:
    void basicControlsChanged(bool enabled);
    void printerAvailabilityChanged( bool const available );
    void projectorPowerLevelChanged( int const value );
    void advancedExposureTimeChanged();

public slots:
    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;
    virtual void printJobChanged() override;

    void setPngDisplayer( PngDisplayer* pngDisplayer );
    void setPrinterAvailable( bool const value );

    void projectorPowerLevel_changed( int const percentage );
    void loadPrintProfile (QSharedPointer<PrintProfile> profile);
    void updatePrintProfile();

private slots:

    void printer_online( );
    void printer_offline( );

    void printer_positionReport( double const px, int const cx );
    void printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const bedPwm );

    void offsetSliderValueChanged();

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

    void offsetDisregardFirstLayerStateChanged(int state);
    void setWidgetsIds();
};

#endif // __ADVANCEDTAB_H__
