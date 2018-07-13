#include "UBEditable.h"

#include "UBHorizontalHandle.h"

UBHorizontalHandle::UBHorizontalHandle()
{
}

UBHorizontalHandle::UBHorizontalHandle(UBHorizontalHandle* const src):
    UBAbstractHandle(src)
{

}

void UBHorizontalHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF p = mapToParent(event->pos());

    this->setPos(p.x(), pos().y());

    mEditableObject->updateHandle(this);
}

void UBHorizontalHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    UBAbstractHandle::mousePressEvent(event);
}

void UBHorizontalHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    UBAbstractHandle::mouseReleaseEvent(event);
}

UBItem *UBHorizontalHandle::deepCopy() const
{
    UBHorizontalHandle * copy = new UBHorizontalHandle();

    copyItemParameters(copy);

    return copy;
}

void UBHorizontalHandle::copyItemParameters(UBItem *copy) const
{
    UBHorizontalHandle *cp = dynamic_cast<UBHorizontalHandle*>(copy);
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
