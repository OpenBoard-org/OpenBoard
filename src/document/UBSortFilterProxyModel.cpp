#include "UBSortFilterProxyModel.h"
#include "UBDocumentController.h"

UBSortFilterProxyModel::UBSortFilterProxyModel():
    QSortFilterProxyModel()
{
    setDynamicSortFilter(false);
    setSortCaseSensitivity(Qt::CaseInsensitive);
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
