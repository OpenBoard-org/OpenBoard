#ifndef UBSORTFILTERPROXYMODEL_H
#define UBSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class UBSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    UBSortFilterProxyModel();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};
#endif // UBSORTFILTERPROXYMODEL_H
