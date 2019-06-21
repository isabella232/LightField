#ifndef __SYSTEMTAB_H__
#define __SYSTEMTAB_H__

#include "tabbase.h"

class UpgradeKitInfo;
class UpgradeManager;
class UpgradeSelector;

class SystemTab: public InitialShowEventMixin<SystemTab, TabBase> {

    Q_OBJECT

public:

    SystemTab( QWidget* parent = nullptr );
    virtual ~SystemTab( ) override;

    virtual TabIndex tabIndex( ) const override { return TabIndex::System; }

protected:

    virtual void _connectShepherd( )                    override;
    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    bool             _isPrinterOnline            { false           };
    bool             _isPrinterAvailable         { true            };
    bool             _isFirmwareUpgradeAvailable { false           };
    bool             _isSoftwareUpgradeAvailable { false           };
    UpgradeManager*  _upgradeManager             {                 };
    UpgradeSelector* _upgradeSelector            {                 };

    QLabel*          _logoLabel                  { new QLabel      };
    QLabel*          _versionLabel               { new QLabel      };

    QLabel*          _copyrightsLabel            { new QLabel      };

    QPushButton*     _updateSoftwareButton       { new QPushButton };
    QPushButton*     _updateFirmwareButton       { new QPushButton };

    QPushButton*     _restartButton              { new QPushButton };
    QPushButton*     _shutDownButton             { new QPushButton };

    QVBoxLayout*     _layout                     { new QVBoxLayout };

    void _updateButtons( );
    bool _yesNoPrompt( QString const& title, QString const& text );

signals:
    ;

    void printerAvailabilityChanged( bool const available );

public slots:
    ;

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void upgradeManager_upgradeCheckComplete( bool const upgradesFound );
    void upgradeManager_upgradeFailed( );

    void upgradeSelector_canceled( );
    void upgradeSelector_kitSelected( UpgradeKitInfo const& kit );

    void setPrinterAvailable( bool const value );
    void setUpgradeManager( UpgradeManager* upgradeManager );

protected slots:
    ;

private slots:
    ;

    void printer_online( );
    void printer_offline( );

    void shepherd_firmwareVersionReport( QString const& version );

    void updateSoftwareButton_clicked( bool );
    void updateFirmwareButton_clicked( bool );

    void restartButton_clicked( bool );
    void shutDownButton_clicked( bool );

};

#endif // __SYSTEMTAB_H__
