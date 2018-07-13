#include "UBAbstractHandle.h"

#include "UBEditable.h"

UBAbstractHandle::UBAbstractHandle()
{
    mId = 0;
    mClick = false;
    mRadius = 7;
    mEditableObject = 0;

    //setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

UBAbstractHandle::UBAbstractHandle(UBAbstractHandle* const src)
{
    mId = src->mId;
    mClick = src->mClick;
    mRadius = src->mRadius;
    mEditableObject = src->mEditableObject;

    setPos(src->pos());

    setFlags(src->flags());
    setData(UBGraphicsItemData::itemLayerType, src->data(UBGraphicsItemData::itemLayerType));
}

void UBAbstractHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);

    if(mEditableObject){
        mEditableObject->updateHandle(this);
    }
}

void UBAbstractHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mClick = true;
    QGraphicsItem::mousePressEvent(event);

    if(mEditableObject){
        mEditableObject->focusHandle(this);
    }
}

void UBAbstractHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mClick = false;
    QGraphicsItem::mouseReleaseEvent(event);
}


void UBAbstractHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QBrush brush(Qt::white);

    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);

    painter->drawEllipse(-mRadius, -mRadius, mRadius*2, mRadius*2);
}

QRectF UBAbstractHandle::boundingRect() const
{
    int d = mRadius*2, x = -mRadius;
    int y = x;

    return QRectF(x, y, d, d);
}
