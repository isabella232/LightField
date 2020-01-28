#include "advancedtabselectionmodel.h"
#include <iostream>


void AdvancedTabSelectionModel::onclick(const QModelIndex &index)
{
    QStandardItem *item = this->itemFromIndex(index);
    
    int row = index.row();
    int column = index.column();

    std::cout << row << std::endl;
    std::cout << column << std::endl;

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
