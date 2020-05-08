#ifndef SLICESORDERPOPUP_H
#define SLICESORDERPOPUP_H

#include <QtCore>
#include <QtWidgets>

#include "ordermanifestmanager.h"

class SlicesOrderPopup: public QDialog {
    Q_OBJECT
private:
    QRadioButton*           _alphaNum                        { new QRadioButton( "Alphanumeric ")       };
    QRadioButton*           _numerical                       { new QRadioButton( "Numeric" )            };
    QRadioButton*           _custom                          { new QRadioButton( "Custom" )             };
    QPushButton*            _okButton                        { new QPushButton ( "Confirm" )            };
    QPushButton*            _arrowUp                         { new QPushButton ( "Up" )                 };
    QPushButton*            _arrowDown                       { new QPushButton ( "Down" )               };
    QTableView*             _list                            { new QTableView                           };
    QStandardItemModel*     _model                           { new QStandardItemModel                   };
    OrderManifestManager*   _manifestManager;

    void fillModel();
public:
    SlicesOrderPopup() { }
    SlicesOrderPopup(OrderManifestManager* manifestManager);

public slots:

    void oklClicked_clicked(bool);
    void arrowUp_clicked(bool);
    void arrowDown_clicked(bool);
    void alphaNum_clicked(bool);
    void numerical_clicked(bool);
    void custom_clicked(bool);

signals:
    void okclicked();
};

#endif // SLICESORDERPOPUP_H
