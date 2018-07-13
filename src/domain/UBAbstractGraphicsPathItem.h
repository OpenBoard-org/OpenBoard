#ifndef UBABSTRACTGRAPHICSPATHITEM_H
#define UBABSTRACTGRAPHICSPATHITEM_H

#include "UBAbstractGraphicsItem.h"

class UBAbstractGraphicsPathItem : public UBAbstractGraphicsItem
{
public:
    UBAbstractGraphicsPathItem(QGraphicsItem *parent = 0);

    virtual void addPoint(const QPointF &point) = 0;

    QPainterPath path() const
    {
        return mPath;
    }

    void setPath(QPainterPath path)
    {
        mPath = path;
    }

    virtual QRectF boundingRect() const;

    virtual QPainterPath shape() const;

    virtual void copyItemParameters(UBItem *copy) const;

    enum ArrowType	// Do NOT INSERT new styles, only APPEND them, because those values are persisted in documents SVG files.
    {
        ArrowType_None,
        ArrowType_Round,
        ArrowType_Arrow
    };

    enum ArrowPosition
    {
        StartArrow,
        EndArrow
    };

    void setStartArrowType(ArrowType arrowType);
    ArrowType startArrowType() const {return mStartArrowType;}

    void setEndArrowType(ArrowType arrowType);
    ArrowType endArrowType() const {return mEndArrowType;}


protected:
    QPainterPath mPath;

    QGraphicsPathItem * startArrowGraphicsItem(){return mStartArrowGraphicsItem;}
    QGraphicsPathItem * endArrowGraphicsItem(){return mEndArrowGraphicsItem;}
    virtual void drawArrows();


private:
    ArrowType mStartArrowType;
    ArrowType mEndArrowType;

    QGraphicsPathItem * mStartArrowGraphicsItem;
    QGraphicsPathItem * mEndArrowGraphicsItem;
    virtual qreal arrowAngle(ArrowPosition arrowPosition);
};

#endif // UBABSTRACTGRAPHICSPATHITEM_H
