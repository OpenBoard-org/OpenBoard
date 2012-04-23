#ifndef UBGRAPHICSSTROKESGROUP_H
#define UBGRAPHICSSTROKESGROUP_H

#include <QGraphicsItemGroup>
#include <QGraphicsSceneMouseEvent>

#include "core/UB.h"
#include "UBItem.h"

class UBGraphicsStrokesGroup : public QObject, public QGraphicsItemGroup, public UBItem, public UBGraphicsItem
{
    Q_OBJECT
public:
    UBGraphicsStrokesGroup(QGraphicsItem* parent = 0);
    ~UBGraphicsStrokesGroup();
    virtual UBItem* deepCopy() const;
    virtual void remove();
    virtual UBGraphicsItemDelegate* Delegate() const {return mDelegate;}

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif // UBGRAPHICSSTROKESGROUP_H
