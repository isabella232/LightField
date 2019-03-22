#ifndef __PRINTTAB_H__
#define __PRINTTAB_H__

#include "tabbase.h"

enum class BuildPlatformState {
    Lowered,
    Raising,
    Raised,
    Lowering,
};

class PrintTab: public TabBase {

    Q_OBJECT

public:

    PrintTab( QWidget* parent = nullptr );
    virtual ~PrintTab( ) override;

    bool isPrintButtonEnabled( ) const { return _printButton->isEnabled( ); }

protected:

private:

    BuildPlatformState _buildPlatformState                 { BuildPlatformState::Lowered };

    QLabel*            _exposureTimeLabel                  { new QLabel      };
    QLabel*            _exposureTimeValue                  { new QLabel      };
    QHBoxLayout*       _exposureTimeValueLayout            {                 };
    QSlider*           _exposureTimeSlider                 { new QSlider     };

    QLabel*            _exposureTimeScaleFactorValue       { new QLabel      };
    QLabel*            _exposureTimeScaleFactorLabel       { new QLabel      };
    QHBoxLayout*       _exposureTimeScaleFactorValueLayout {                 };
    QSlider*           _exposureTimeScaleFactorSlider      { new QSlider     };

    QVBoxLayout*       _exposureTimeLayout                 { new QVBoxLayout };
    QVBoxLayout*       _exposureTimeScaleFactorLayout      { new QVBoxLayout };

    QGridLayout*       _exposureLayout                     { new QGridLayout };

    QLabel*            _powerLevelLabel                    { new QLabel      };
    QLabel*            _powerLevelValue                    { new QLabel      };
    QHBoxLayout*       _powerLevelValueLayout              {                 };
    QSlider*           _powerLevelSlider                   { new QSlider     };

    QVBoxLayout*       _optionsLayout                      { new QVBoxLayout };
    QWidget*           _optionsContainer                   { new QWidget     };

    QPushButton*       _printButton                        { new QPushButton };

    QPushButton*       _raiseOrLowerButton                 { new QPushButton };
    QPushButton*       _homeButton                         { new QPushButton };

    QGroupBox*         _adjustmentsGroup                   { new QGroupBox   };

    QGridLayout*       _layout                             { new QGridLayout };

    virtual void _initialShowEvent( QShowEvent* showEvent ) override;

    virtual void _connectPrintJob( ) override;

signals:

    void printButtonClicked( );

public slots:

    void setAdjustmentButtonsEnabled( bool const value );
    void setPrintButtonEnabled( bool const value );

    void raiseBuildPlatform_moveToComplete( bool const success );
    void lowerBuildPlatform_moveToComplete( bool const success );
    void home_homeComplete( bool const success );

protected slots:

private slots:

    void exposureTimeSlider_valueChanged( int value );
    void exposureTimeScaleFactorSlider_valueChanged( int value );
    void powerLevelSlider_valueChanged( int value );
    void printButton_clicked( bool );
    void raiseOrLowerButton_clicked( bool );
    void homeButton_clicked( bool );

};

#endif // __PRINTTAB_H__
