#ifndef __PRINTTAB_H__
#define __PRINTTAB_H__

#include "tabbase.h"

#undef ENABLE_SPEED_SETTING

enum class BuildPlatformState {
    Lowered,
    Raising,
    Raised,
    Lowering,
};

inline constexpr int operator+( BuildPlatformState const value ) { return static_cast<int>( value ); }

char const* ToString( BuildPlatformState const value );

class PrintTab: public InitialShowEventMixin<PrintTab, TabBase> {

    Q_OBJECT

public:

    PrintTab( QWidget* parent = nullptr );
    virtual ~PrintTab( ) override;

    bool             isPrintButtonEnabled( ) const          { return _printButton->isEnabled( ); }

    virtual TabIndex tabIndex( )             const override { return TabIndex::Print;            }

protected:

    virtual void _connectPrintJob( )                    override;
    virtual void _connectShepherd( )                    override;
    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    bool               _isPrinterOnline                    { false };
    bool               _isPrinterAvailable                 { true  };
    bool               _isPrinterPrepared                  { false };
    bool               _isModelRendered                    { false };
    BuildPlatformState _buildPlatformState                 { BuildPlatformState::Lowered };

    QLabel*            _exposureTimeLabel                  { new QLabel      };
    QLabel*            _exposureTimeValue                  { new QLabel      };
    QHBoxLayout*       _exposureTimeValueLayout            {                 };

    QSlider*           _exposureTimeSlider                 { new QSlider     };

    QVBoxLayout*       _exposureTimeLayout                 { new QVBoxLayout };


    QLabel*            _exposureTimeScaleFactorLabel       { new QLabel      };
    QLabel*            _exposureTimeScaleFactorValue       { new QLabel      };
    QHBoxLayout*       _exposureTimeScaleFactorValueLayout {                 };

    QSlider*           _exposureTimeScaleFactorSlider      { new QSlider     };

    QVBoxLayout*       _exposureTimeScaleFactorLayout      { new QVBoxLayout };


    QHBoxLayout*       _exposureLayout                     { new QHBoxLayout };


    QLabel*            _powerLevelLabel                    { new QLabel      };
    QLabel*            _powerLevelValue                    { new QLabel      };
    QSlider*           _powerLevelSlider                   { new QSlider     };

#if defined ENABLE_SPEED_SETTING
    QLabel*            _printSpeedLabel                    { new QLabel      };
    QLabel*            _printSpeedValue                    { new QLabel      };
    QSlider*           _printSpeedSlider                   { new QSlider     };
#endif // defined ENABLE_SPEED_SETTING

    QVBoxLayout*       _optionsLayout                      { new QVBoxLayout };
    QGroupBox*         _optionsGroup                       { new QGroupBox   };

    QPushButton*       _printButton                        { new QPushButton };

    QPushButton*       _raiseOrLowerButton                 { new QPushButton };
    QPushButton*       _homeButton                         { new QPushButton };

    QGroupBox*         _adjustmentsGroup                   { new QGroupBox   };

    QGridLayout*       _layout                             { new QGridLayout };

    void _updateUiState( );

signals:

    void printerAvailabilityChanged( bool const available );
    void printRequested( );

public slots:

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void setModelRendered( bool const value );
    void setPrinterPrepared( bool const value );
    void clearPrinterPrepared( );

    void setPrinterAvailable( bool const value );

protected slots:

private slots:

    void printer_online( );
    void printer_offline( );

    void raiseBuildPlatform_moveAbsoluteComplete( bool const success );
    void lowerBuildPlatform_moveAbsoluteComplete( bool const success );
    void home_homeComplete( bool const success );

    void exposureTimeSlider_valueChanged( int value );
    void exposureTimeScaleFactorSlider_valueChanged( int value );
    void powerLevelSlider_valueChanged( int value );
#if defined ENABLE_SPEED_SETTING
    void printSpeedSlider_valueChanged( int value );
#endif // defined ENABLE_SPEED_SETTING
    void printButton_clicked( bool );
    void raiseOrLowerButton_clicked( bool );
    void homeButton_clicked( bool );

};

#endif // __PRINTTAB_H__
