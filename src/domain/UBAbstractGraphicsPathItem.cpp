#include "UBAbstractGraphicsPathItem.h"
#include <QBrush>
#include <QTransform>
#include <QtCore/qmath.h>

UBAbstractGraphicsPathItem::UBAbstractGraphicsPathItem(QGraphicsItem *parent):
    UBAbstractGraphicsItem(parent)
  , mStartArrowType(ArrowType_None)
  , mStartArrowGraphicsItem(NULL)
  , mEndArrowType(ArrowType_None)
  , mEndArrowGraphicsItem(NULL)
{

}

QRectF UBAbstractGraphicsPathItem::boundingRect() const
{
    QRectF rect = path().boundingRect();

    rect = adjustBoundingRect(rect);

    return rect;
}

QPainterPath UBAbstractGraphicsPathItem::shape() const
{
    return path();
}

void UBAbstractGraphicsPathItem::copyItemParameters(UBItem *copy) const
{
    UBAbstractGraphicsItem::copyItemParameters(copy);

    UBAbstractGraphicsPathItem* cp = dynamic_cast<UBAbstractGraphicsPathItem*>(copy);

    if(!cp) return;

    cp->setPath(path());

    // Apply arrow type to the copy :
    cp->setStartArrowType(mStartArrowType);
    cp->setEndArrowType(mEndArrowType);

}

void UBAbstractGraphicsPathItem::setStartArrowType(UBAbstractGraphicsPathItem::ArrowType arrowType)
{
    if (mStartArrowType != arrowType)
    {
        mStartArrowType = arrowType;
        delete mStartArrowGraphicsItem; // Will be re-created
        mStartArrowGraphicsItem = NULL;
    }
}

void UBAbstractGraphicsPathItem::setEndArrowType(UBAbstractGraphicsPathItem::ArrowType arrowType)
{
    if (mEndArrowType != arrowType)
    {
        mEndArrowType = arrowType;
        delete mEndArrowGraphicsItem;   // Will be re-created
        mEndArrowGraphicsItem = NULL;
    }
}

/**
 * @brief UBAbstractGraphicsPathItem::arrowAngle
 * @return Angle of arrow to draw at extremity of the path.
 * @param iArrowPosition : 0 for StartArrow, 1 for EndArrow
 */
qreal UBAbstractGraphicsPathItem::arrowAngle(ArrowPosition arrowPosition)
{
    qreal angle = 0;

    if (arrowPosition == 0)
        angle = - path().angleAtPercent(0);
    else if (arrowPosition == 1)
        angle = - path().angleAtPercent(1);
    // for other cases, leave angle to zero.

    return angle;
}

void UBAbstractGraphicsPathItem::drawArrows()
{
    int nbElements = path().elementCount();

    if (mStartArrowType != ArrowType_None)
    {
        if (nbElements > 1)
        {
            if (mStartArrowGraphicsItem == NULL)
            {
                QPainterPath pathArrow;
                switch (mStartArrowType) {
                case ArrowType_Arrow:
                    pathArrow.moveTo(6, -3);
                    pathArrow.lineTo(-2, 0);
                    pathArrow.lineTo(6, 3);
                    pathArrow.closeSubpath();
                    break;
                case ArrowType_Round:
                    pathArrow.addEllipse(QPointF(0, 0), 2, 2);
                    break;
                default:
                    break;
                }
                mStartArrowGraphicsItem = new QGraphicsPathItem(pathArrow, this);// QGraphicsItem::scene());
            }

            QPainterPath::Element firstElement = path().elementAt(0);
            mStartArrowGraphicsItem->setPos(firstElement.x, firstElement.y);

            QTransform transform;
            transform.rotate(arrowAngle(StartArrow));

            if (hasStrokeProperty())
            {
                int width = pen().width();
                transform.scale(width, width);

                QColor c = pen().color();
                mStartArrowGraphicsItem->setPen(c);
                mStartArrowGraphicsItem->setBrush(c);
            }

            mStartArrowGraphicsItem->setTransform(transform);
        }
    }

    if (mEndArrowType != ArrowType_None)
    {
        // Draw Arrow on extremity :
        if (nbElements > 1)
        {
            if (mEndArrowGraphicsItem == NULL)
            {
                QPainterPath pathArrow;

                switch (mEndArrowType) {
                case ArrowType_Arrow:
                    pathArrow.moveTo(-6, -3);
                    pathArrow.lineTo(2, 0);
                    pathArrow.lineTo(-6, 3);
                    pathArrow.closeSubpath();
                    break;
                case ArrowType_Round:
                    pathArrow.addEllipse(QPointF(0, 0), 2, 2);
                    break;
                default:
                    break;
                }
                mEndArrowGraphicsItem = new QGraphicsPathItem(pathArrow, this);//, QGraphicsItem::scene());
            }

            QPainterPath::Element lastElement = path().elementAt(nbElements-1);
            mEndArrowGraphicsItem->setPos(QPointF(lastElement.x, lastElement.y));

            QTransform transform;
            transform.rotate(arrowAngle(EndArrow));
            if (hasStrokeProperty())
            {
                int width = pen().width();
                transform.scale(width, width);

                QColor c = pen().color();
                mEndArrowGraphicsItem->setPen(c);
                mEndArrowGraphicsItem->setBrush(c);
            }

            mEndArrowGraphicsItem->setTransform(transform);

        }
    }
}
