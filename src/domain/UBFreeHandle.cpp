#include "UBFreeHandle.h"
#include "UBEditable.h"

UBFreeHandle::UBFreeHandle()
{
}

UBFreeHandle::UBFreeHandle(UBFreeHandle* const src):
    UBAbstractHandle(src)
{

}

void UBFreeHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF p = mapToParent(event->pos());

    QPointF diff = p - pos();

    moveBy(diff.x(), diff.y());

    mEditableObject->updateHandle(this);
}

void UBFreeHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    UBAbstractHandle::mousePressEvent(event);
}

void UBFreeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    UBAbstractHandle::mouseReleaseEvent(event);
}

UBItem *UBFreeHandle::deepCopy() const
{
    UBFreeHandle * copy = new UBFreeHandle();

    copyItemParameters(copy);

    return copy;
}

void UBFreeHandle::copyItemParameters(UBItem *copy) const
{
    UBFreeHandle *cp = dynamic_cast<UBFreeHandle*>(copy);
    if (cp)
    {
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));

        cp->setPos(pos());
        cp->setEditableObject(cp->editableObject());
    }
}
