#include "advancedtabselectionmodel.h"

void AdvancedTabSelectionModel::onclick(const QModelIndex &index)
{
    int row = index.row();

    for(int i=0; i<_panelsCount; i++)
    {
        this->_relatedPanel[i]->setVisible(i==row);
    }
}

AdvancedTabSelectionModel::AdvancedTabSelectionModel(int r,int c, QWidget** relatedPanels, int panelsCount): QStandardItemModel::QStandardItemModel(r,c)
{
    this->_relatedPanel = relatedPanels;
    this->_panelsCount = panelsCount;
}

AdvancedTabSelectionModel::~AdvancedTabSelectionModel()
{ }
