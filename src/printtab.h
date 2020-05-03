#ifndef __PRINTTAB_H__
#define __PRINTTAB_H__

#include "tabbase.h"

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


    QLabel*            _powerLevelLabel                    { new QLabel      };
    QLabel*            _powerLevelValue                    { new QLabel      };
    QSlider*           _powerLevelSlider                   { new QSlider     };

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

    void projectorPowerLevelChanged( int const value );

public slots:

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void setModelRendered( bool const value );
    void setPrinterPrepared( bool const value );

    void setPrinterAvailable( bool const value );

    void projectorPowerLevel_changed( int const percentage );

protected slots:

private slots:

    void printer_online( );
    void printer_offline( );

    void raiseBuildPlatform_moveAbsoluteComplete( bool const success );
    void lowerBuildPlatform_moveAbsoluteComplete( bool const success );
    void home_homeComplete( bool const success );

    void powerLevelSlider_sliderReleased( );
    void powerLevelSlider_valueChanged( int percentage );

    void printButton_clicked( bool );
    void raiseOrLowerButton_clicked( bool );
    void homeButton_clicked( bool );

};

#endif // __PRINTTAB_H__
