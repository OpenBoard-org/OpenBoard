#include "UBEditable.h"

#include "UBVerticalHandle.h"

UBVerticalHandle::UBVerticalHandle()
{
}

UBVerticalHandle::UBVerticalHandle(UBVerticalHandle* const src):
    UBAbstractHandle(src)
{

}

void UBVerticalHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF p = mapToParent(event->pos());

    this->setPos(pos().x(), p.y());

    mEditableObject->updateHandle(this);
}

void UBVerticalHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    UBAbstractHandle::mousePressEvent(event);
}

void UBVerticalHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    UBAbstractHandle::mouseReleaseEvent(event);
}

UBItem *UBVerticalHandle::deepCopy() const
{
    UBVerticalHandle * copy = new UBVerticalHandle();

    copyItemParameters(copy);

    return copy;
}

void UBVerticalHandle::copyItemParameters(UBItem *copy) const
{
    UBVerticalHandle *cp = dynamic_cast<UBVerticalHandle*>(copy);
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
