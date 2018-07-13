#ifndef UBGRAPHICSFREEHANDITEM_H
#define UBGRAPHICSFREEHANDITEM_H

#include "UBAbstractGraphicsPathItem.h"

class UBGraphicsFreehandItem : public UBAbstractGraphicsPathItem
{

    bool mIsInCreationMode;

    int HANDLE_SIZE; //in pixel

public:
    UBGraphicsFreehandItem(QGraphicsItem *parent = 0);

    virtual void addPoint(const QPointF &point);

    UBItem *deepCopy() const;

    QRectF boundingRect() const;

    void copyItemParameters(UBItem *copy) const;

    void setClosed(bool closed);

    enum { Type = UBGraphicsItemType::GraphicsFreehandItemType};
    virtual int type() const { return Type; }
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QPointF mStartEndPoint[2];
    void setIsInCreationMode(bool mode);

    virtual qreal arrowAngle(ArrowPosition arrowPosition);
};

#endif // UBGRAPHICSFREEHANDITEM_H
