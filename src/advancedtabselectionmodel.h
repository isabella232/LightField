#ifndef __ADVANCEDTABSELECTIONMODEL_H__
#define __ADVANCEDTABSELECTIONMODEL_H__

#include "tabbase.h"

class AdvancedTabSelectionModel: public QStandardItemModel
{
    Q_OBJECT
    private:
        QWidget** _relatedPanel;
        int       _panelsCount;

    public:
        AdvancedTabSelectionModel(int r,int c, QWidget** relatedPanel, int panelsCount);
        ~AdvancedTabSelectionModel();

    public slots:
        void onclick(const QModelIndex &index);
};

#endif // __ADVANCEDTABSELECTIONMODEL_H__
