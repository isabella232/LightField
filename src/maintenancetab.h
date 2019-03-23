#ifndef __MAINTENANCETAB_H__
#define __MAINTENANCETAB_H__

#include "tabbase.h"

class MaintenanceTab: public TabBase {

    Q_OBJECT

public:

    MaintenanceTab( QWidget* parent = nullptr );
    virtual ~MaintenanceTab( ) override;

protected:

private:

    QLabel*      _logoLabel      { new QLabel      };
    QLabel*      _versionLabel   { new QLabel      };

    QPushButton* _restartButton  { new QPushButton };
    QPushButton* _shutDownButton { new QPushButton };

    QVBoxLayout* _layout         {                 };

signals:

public slots:

protected slots:

private slots:

    void restartButton_clicked( bool );
    void shutDownButton_clicked( bool );

};

#endif // __MAINTENANCETAB_H__
