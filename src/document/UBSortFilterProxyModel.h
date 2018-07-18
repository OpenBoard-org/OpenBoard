#ifndef UBSORTFILTERPROXYMODEL_H
#define UBSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "core/UBPersistenceManager.h"

class UBSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    UBSortFilterProxyModel();

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // UBSORTFILTERPROXYMODEL_H
