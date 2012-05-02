#ifndef UBGRAPHICSGROUPCONTAINERITEM_H
#define UBGRAPHICSGROUPCONTAINERITEM_H

#include <QGraphicsItem>

#include "domain/UBItem.h"

class UBGraphicsGroupContainerItem : public QGraphicsItemGroup, public UBItem, public UBGraphicsItem
{

public:
    UBGraphicsGroupContainerItem (QGraphicsItem *parent = 0);
    virtual UBGraphicsItemDelegate* Delegate() const { return mDelegate;}

    virtual UBGraphicsScene* scene();
    virtual UBGraphicsGroupContainerItem *deepCopy() const;
    virtual void remove();
    enum { Type = UBGraphicsItemType::groupContainerType };

    virtual int type() const
    {
        return Type;
    }

    void destroy();


protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:

};

#endif // UBGRAPHICSGROUPCONTAINERITEM_H
