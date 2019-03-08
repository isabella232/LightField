#ifndef __PRINTTAB_H__
#define __PRINTTAB_H__

enum class BuildPlatformState {
    Lowered,
    Raising,
    Raised,
    Lowering,
};

class PrintJob;
class Shepherd;

class PrintTab: public QWidget {

    Q_OBJECT

public:

    PrintTab( QWidget* parent = nullptr );
    virtual ~PrintTab( ) override;

    bool      isPrintButtonEnabled( ) const { return printButton->isEnabled( ); }
    PrintJob* printJob( )             const { return _printJob;                 }
    Shepherd* shepherd( )             const { return _shepherd;                 }

protected:

    void showEvent( QShowEvent* event );

private:

    BuildPlatformState _buildPlatformState                { BuildPlatformState::Lowered };
    PrintJob*          _printJob                          { };
    Shepherd*          _shepherd                          { };

    QLabel*            exposureTimeLabel                  { new QLabel      };
    QLabel*            exposureTimeValue                  { new QLabel      };
    QHBoxLayout*       exposureTimeValueLayout            {                 };
    QSlider*           exposureTimeSlider                 { new QSlider     };

    QLabel*            exposureTimeScaleFactorValue       { new QLabel      };
    QLabel*            exposureTimeScaleFactorLabel       { new QLabel      };
    QHBoxLayout*       exposureTimeScaleFactorValueLayout {                 };
    QSlider*           exposureTimeScaleFactorSlider      { new QSlider     };

    QVBoxLayout*       exposureTimeLayout                 { new QVBoxLayout };
    QVBoxLayout*       exposureTimeScaleFactorLayout      { new QVBoxLayout };

    QGridLayout*       exposureLayout                     { new QGridLayout };

    QLabel*            powerLevelLabel                    { new QLabel      };
    QLabel*            powerLevelValue                    { new QLabel      };
    QHBoxLayout*       powerLevelValueLayout              {                 };
    QSlider*           powerLevelSlider                   { new QSlider     };

    QVBoxLayout*       optionsLayout                      { new QVBoxLayout };
    QWidget*           optionsContainer                   { new QWidget     };

    QPushButton*       printButton                        { new QPushButton };

    QPushButton*       _raiseOrLowerButton                { new QPushButton };
    QPushButton*       _homeButton                        { new QPushButton };

    QGroupBox*         _adjustmentsGroup                  { new QGroupBox   };

    QGridLayout*       _layout                            { new QGridLayout };

    std::function<void( QShowEvent* )> _initialShowEventFunc;

    void _initialShowEvent( QShowEvent* event );

signals:

    void printButtonClicked( );

public slots:

    void setAdjustmentButtonsEnabled( bool const value );
    void setPrintButtonEnabled( bool const value );
    void setPrintJob( PrintJob* printJob );
    void setShepherd( Shepherd* shepherd );

    void raiseBuildPlatform_moveToComplete( bool const success );
    void lowerBuildPlatform_moveToComplete( bool const success );
    void home_homeComplete( bool const success );

protected slots:

private slots:

    void exposureTimeSlider_valueChanged( int value );
    void exposureTimeScaleFactorSlider_valueChanged( int value );
    void powerLevelSlider_valueChanged( int value );
    void printButton_clicked( bool );
    void _raiseOrLowerButton_clicked( bool );
    void _homeButton_clicked( bool );

};

#endif // __PRINTTAB_H__
