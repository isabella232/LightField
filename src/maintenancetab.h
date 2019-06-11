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


    QVBoxLayout*    _layout                   { new QVBoxLayout };

    void _updateButtons( );
    bool _yesNoPrompt( QString const& title, QString const& text );

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

};

#endif // __MAINTENANCETAB_H__
