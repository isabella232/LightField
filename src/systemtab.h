#ifndef __SYSTEMTAB_H__
#define __SYSTEMTAB_H__

#include <QtCore>
#include <QtWidgets>
#include "tabbase.h"

class DebugLogCopier;
class UpgradeKitInfo;
class UpgradeManager;
class UpgradeSelector;
class UsbMountManager;

class SystemTab: public InitialShowEventMixin<SystemTab, TabBase> {

    Q_OBJECT

public:

    SystemTab(QWidget* parent = nullptr);
    virtual ~SystemTab( ) override;

    virtual TabIndex tabIndex( ) const override { return TabIndex::System; }

protected:

    virtual void _connectShepherd( )                    override;
    virtual void _initialShowEvent( QShowEvent* event ) override;

    virtual void _connectUsbMountManager( )             override;

private:

    bool             _isPrinterOnline            { false           };
    bool             _isPrinterAvailable         { true            };
    bool             _isFirmwareUpgradeAvailable { false           };
    bool             _isSoftwareUpgradeAvailable { false           };

    UpgradeManager*  _upgradeManager             {                 };
    UpgradeSelector* _upgradeSelector            {                 };

    QString          _mountPoint                 {                 };

    QLabel*          _logoLabel                  { new QLabel      };
    QLabel*          _versionLabel               { new QLabel      };

    QLabel*          _copyrightsLabel            { new QLabel      };

    QPushButton*     _updateSoftwareButton       { new QPushButton };
    QPushButton*     _copyLogsButton             { new QPushButton };

    QPushButton*     _restartButton              { new QPushButton };
    QPushButton*     _shutDownButton             { new QPushButton };

    QVBoxLayout*     _layout                     { new QVBoxLayout };

    void _updateButtons( );

signals:
    ;

    void printerAvailabilityChanged( bool const available );

public slots:
    ;

    void setPrinterAvailable( bool const value );
    void setUpgradeManager( UpgradeManager* upgradeManager );

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;
    virtual void printJobChanged() override;

protected slots:
    ;

private slots:
    ;

    void usbMountManager_filesystemMounted( QString const& mountPoint );
    void usbMountManager_filesystemUnmounted( QString const& mountPoint );

    void upgradeManager_upgradeCheckStarting( );
    void upgradeManager_upgradeCheckComplete( bool const upgradesFound );
    void upgradeManager_upgradeFailed( );

    void upgradeSelector_canceled( );
    void upgradeSelector_kitSelected( UpgradeKitInfo const& kit );

    void printer_online( );
    void printer_offline( );

    void shepherd_firmwareVersionReport( QString const& version );

    void updateSoftwareButton_clicked( bool );
    void copyLogsButton_clicked( bool );

    void restartButton_clicked( bool );
    void shutDownButton_clicked( bool );

};

#endif // __SYSTEMTAB_H__
