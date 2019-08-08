#ifndef __UPGRADESELECTOR_H__
#define __UPGRADESELECTOR_H__

#include "gesturelistview.h"
#include "initialshoweventmixin.h"
#include "upgrademanager.h"

class UpgradeManager;

class UpgradeSelector: public InitialShowEventMixin<UpgradeSelector, QMainWindow> {

    Q_OBJECT

public:

    UpgradeSelector( UpgradeManager* upgradeManager, QWidget* parent = nullptr );
    virtual ~UpgradeSelector( ) override;

    void showInProgressMessage( );
    void showFailedMessage( );

protected:

    virtual void _initialShowEvent( QShowEvent* event ) override;

private:

    UpgradeManager*    _upgradeManager          { };

    QPushButton*       _upgradeButton           { };
    QPushButton*       _cancelButton            { };
    QLabel*            _description             { };
    QHBoxLayout*       _upgradeSelectLayout     { };

    QHBoxLayout*       _upgradeInProgressLayout { };

    QLabel*            _aptOutput               { };
    QHBoxLayout*       _upgradeFailedLayout     { };

    UpgradeKitInfoList _availableKits;

    int                _currentSelection        { -1 };

signals:
    ;

    void canceled( );
    void kitSelected( UpgradeKitInfo const& kit );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void kitsListView_clicked( QModelIndex const& index );

};

#endif // __UPGRADESELECTOR_H__
