#include "UBSortFilterProxyModel.h"
#include "UBDocumentController.h"

UBSortFilterProxyModel::UBSortFilterProxyModel():
    QSortFilterProxyModel()
{
    setDynamicSortFilter(false);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setRecursiveFilteringEnabled(true);
    setAutoAcceptChildRows(true);
}

bool UBSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    UBDocumentTreeModel *model = dynamic_cast<UBDocumentTreeModel*>(sourceModel());

    if(model){
        //if it's a top level folder
        //in other words : myDocuments, models and trash folder
        if(model->isToplevel(left) || model->isToplevel(right))
        {
            return false;
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

bool UBSortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
