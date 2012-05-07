#ifndef UBGRAPHICSGROUPCONTAINERITEMDELEGATE_H
#define UBGRAPHICSGROUPCONTAINERITEMDELEGATE_H

#include "domain/UBGraphicsItemDelegate.h"

class UBGraphicsGroupContainerItem;

class UBGraphicsGroupContainerItemDelegate : public UBGraphicsItemDelegate
{
Q_OBJECT

public:
    UBGraphicsGroupContainerItemDelegate(QGraphicsItem* pDelegated, QObject * parent = 0);
    UBGraphicsGroupContainerItem *delegated();

protected:
    virtual void decorateMenu(QMenu *menu);
    virtual void buildButtons();

private:
    DelegateButton *mDestroyGroupButton;
};

#endif // UBGRAPHICSGROUPCONTAINERITEMDELEGATE_H
