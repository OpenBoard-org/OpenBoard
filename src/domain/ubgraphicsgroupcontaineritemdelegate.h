#ifndef UBGRAPHICSGROUPCONTAINERITEMDELEGATE_H
#define UBGRAPHICSGROUPCONTAINERITEMDELEGATE_H

#include "domain/UBGraphicsItemDelegate.h"

class UBGraphicsGroupContainerItem;

class UBGraphicsGroupContainerItemDelegate : public UBGraphicsItemDelegate
{
public:
    UBGraphicsGroupContainerItemDelegate(QGraphicsItem* pDelegated, QObject * parent = 0);

    UBGraphicsGroupContainerItem *delegated();

protected:
    virtual void decorateMenu(QMenu *menu);
    virtual void buildButtons();




private slots:
    void destroyGroup();

private:
    DelegateButton *mDestroyGroupButton;
};

#endif // UBGRAPHICSGROUPCONTAINERITEMDELEGATE_H
