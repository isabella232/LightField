#ifndef __MAINTENANCETAB_H__
#define __MAINTENANCETAB_H__

#include "tabbase.h"

class MaintenanceTab: public TabBase {

    Q_OBJECT

public:

    MaintenanceTab( QWidget* parent = nullptr );
    virtual ~MaintenanceTab( ) override;

protected:

    //virtual void _connectPrintManager( ) override;
    //virtual void _connectShepherd( )     override;

private:

signals:

public slots:

protected slots:

private slots:

};

#endif // __MAINTENANCETAB_H__
