#ifndef __MAINTENANCETAB_H__
#define __MAINTENANCETAB_H__

#include "tabbase.h"

class UpgradeManager;

class MaintenanceTab: public InitialShowEventMixin<MaintenanceTab, TabBase> {

    Q_OBJECT

public:

    MaintenanceTab( QWidget* parent = nullptr );
    virtual ~MaintenanceTab( ) override;

    virtual TabIndex tabIndex( ) const override { return TabIndex::Maintenance; }

protected:

    virtual void _connectShepherd( )                    override;
    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    bool            _isPrinterOnline          { false           };
    bool            _isPrinterAvailable       { true            };
    UpgradeManager* _upgradeManager           {                 };


    QLabel*         _logoLabel                { new QLabel      };
    QLabel*         _versionLabel             { new QLabel      };

    QLabel*         _copyrightsLabel          { new QLabel      };

    QPushButton*    _updateSoftwareButton     { new QPushButton };
    QPushButton*    _updateFirmwareButton     { new QPushButton };

    QPushButton*    _restartButton            { new QPushButton };
    QPushButton*    _shutDownButton           { new QPushButton };

    QVBoxLayout*    _mainLayout               { new QVBoxLayout };
    QWidget*        _mainContent              { new QWidget     };


    QLabel*         _confirmRestartLabel      { new QLabel      };
    QPushButton*    _confirmRestartYesButton  { new QPushButton };
    QPushButton*    _confirmRestartNoButton   { new QPushButton };

    QVBoxLayout*    _confirmRestartLayout     {                 };
    QWidget*        _confirmRestartContent    { new QWidget     };


    QLabel*         _confirmShutdownLabel     { new QLabel      };
    QPushButton*    _confirmShutdownYesButton { new QPushButton };
    QPushButton*    _confirmShutdownNoButton  { new QPushButton };

    QVBoxLayout*    _confirmShutdownLayout    {                 };
    QWidget*        _confirmShutdownContent   { new QWidget     };


    QVBoxLayout*    _layout                   {                 };

    void _updateButtons( );

signals:
    ;

    void printerAvailabilityChanged( bool const available );

public slots:
    ;

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void upgradeManager_upgradeCheckComplete( bool const upgradesFound );

    void setPrinterAvailable( bool const value );
    void setUpgradeManager( UpgradeManager* upgradeManager );

protected slots:
    ;

private slots:
    ;

    void printer_online( );
    void printer_offline( );

    void updateSoftwareButton_clicked( bool );
    void updateFirmwareButton_clicked( bool );

    void restartButton_clicked( bool );
    void shutDownButton_clicked( bool );

    void confirmRestartYesButton_clicked( bool );
    void confirmRestartNoButton_clicked( bool );

    void confirmShutdownYesButton_clicked( bool );
    void confirmShutdownNoButton_clicked( bool );

};

#endif // __MAINTENANCETAB_H__
