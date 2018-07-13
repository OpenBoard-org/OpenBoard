#ifndef UB1HEDITABLEGRAPHICSCIRCLEITEM_H
#define UB1HEDITABLEGRAPHICSCIRCLEITEM_H

#include "UB1HEditableGraphicsBasicShapeItem.h"

class UB1HEditableGraphicsCircleItem : public UB1HEditableGraphicsBasicShapeItem
{
public:
    UB1HEditableGraphicsCircleItem(QGraphicsItem *parent = 0);

    virtual ~UB1HEditableGraphicsCircleItem();

    enum { Type = UBGraphicsItemType::GraphicsShapeItemType };

    virtual int type() const { return Type; }

    virtual UBItem* deepCopy() const;

    virtual void copyItemParameters(UBItem *copy) const;

    QPointF center() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void updateHandle(UBAbstractHandle *handle);

    virtual QRectF boundingRect() const;

    QPainterPath shape() const;

    virtual void onActivateEditionMode();

    void setRadius(qreal radius);

    void setRect(QRectF rect);

    QRectF rect() const;

    qreal radius() const;

private:
    qreal mRadius;
    bool wIsNeg;
    bool hIsNeg;
};

#endif // UB1HEDITABLEGRAPHICSCIRCLEITEM_H
