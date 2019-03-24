#ifndef __MAINTENANCETAB_H__
#define __MAINTENANCETAB_H__

#include "tabbase.h"

class MaintenanceTab: public TabBase {

    Q_OBJECT

public:

    MaintenanceTab( QWidget* parent = nullptr );
    virtual ~MaintenanceTab( ) override;

protected:

    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    QLabel*      _logoLabel                { new QLabel      };
    QLabel*      _versionLabel             { new QLabel      };

    QLabel*      _copyrightsLabel          { new QLabel      };

    QPushButton* _restartButton            { new QPushButton };
    QPushButton* _shutDownButton           { new QPushButton };

    QVBoxLayout* _mainLayout               { new QVBoxLayout };
    QWidget*     _mainContent              { new QWidget     };


    QLabel*      _confirmRestartLabel      { new QLabel      };
    QPushButton* _confirmRestartYesButton  { new QPushButton };
    QPushButton* _confirmRestartNoButton   { new QPushButton };

    QVBoxLayout* _confirmRestartLayout     {                 };
    QWidget*     _confirmRestartContent    { new QWidget     };


    QLabel*      _confirmShutdownLabel     { new QLabel      };
    QPushButton* _confirmShutdownYesButton { new QPushButton };
    QPushButton* _confirmShutdownNoButton  { new QPushButton };

    QVBoxLayout* _confirmShutdownLayout    {                 };
    QWidget*     _confirmShutdownContent   { new QWidget     };


    QVBoxLayout* _layout                   {                 };

signals:

public slots:

protected slots:

private slots:

    void restartButton_clicked( bool );
    void shutDownButton_clicked( bool );
    void confirmRestartYesButton_clicked( bool );
    void confirmRestartNoButton_clicked( bool );
    void confirmShutdownYesButton_clicked( bool );
    void confirmShutdownNoButton_clicked( bool );

};

#endif // __MAINTENANCETAB_H__
