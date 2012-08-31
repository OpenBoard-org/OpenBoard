#ifndef UBGRAPHICSGROUPCONTAINERITEM_H
#define UBGRAPHICSGROUPCONTAINERITEM_H

#include <QGraphicsItem>

#include "domain/UBItem.h"
#include "frameworks/UBCoreGraphicsScene.h"

class UBGraphicsGroupContainerItem : public QGraphicsItem, public UBItem, public UBGraphicsItem
{

public:
    UBGraphicsGroupContainerItem (QGraphicsItem *parent = 0);
    virtual ~UBGraphicsGroupContainerItem();

    void addToGroup(QGraphicsItem *item);
    void removeFromGroup(QGraphicsItem *item);
    void setCurrentItem(QGraphicsItem *item){mCurrentItem = item;}
    QGraphicsItem *getCurrentItem() const {return mCurrentItem;}
    void deselectCurrentItem();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual UBGraphicsItemDelegate* Delegate() const { return mDelegate;}

    virtual UBCoreGraphicsScene *corescene();
    virtual UBGraphicsGroupContainerItem *deepCopy() const;
    virtual void copyItemParameters(UBItem *copy) const;

    virtual void remove();
    enum { Type = UBGraphicsItemType::groupContainerType };

    virtual int type() const
    {
        return Type;
    }

    virtual void setUuid(const QUuid &pUuid);
    void destroy();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void pRemoveFromGroup(QGraphicsItem *item);

private:

    QRectF itemsBoundingRect;
    QGraphicsItem *mCurrentItem;

};

#endif // UBGRAPHICSGROUPCONTAINERITEM_H
