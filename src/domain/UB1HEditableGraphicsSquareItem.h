#ifndef UB1HEDITABLEGRAPHICSSQUAREITEM_H
#define UB1HEDITABLEGRAPHICSSQUAREITEM_H

#include <QPoint>
#include "UB1HEditableGraphicsBasicShapeItem.h"

class UB1HEditableGraphicsSquareItem : public UB1HEditableGraphicsBasicShapeItem
{
public:
    UB1HEditableGraphicsSquareItem(QGraphicsItem *parent = 0);

    virtual ~UB1HEditableGraphicsSquareItem();

    enum { Type = UBGraphicsItemType::GraphicsShapeItemType };
    virtual int type() const { return Type; }

    virtual UBItem* deepCopy() const;

    virtual void copyItemParameters(UBItem *copy) const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void updateHandle(UBAbstractHandle *handle);

    virtual QRectF boundingRect() const;

    QPainterPath shape() const;

    virtual void onActivateEditionMode();

    void setRect(QRectF rect);

    QRectF rect() const;

private:
    int mSide;
    bool hIsNeg;
    bool wIsNeg;
};

#endif // UB1HEDITABLEGRAPHICSSQUAREITEM_H
