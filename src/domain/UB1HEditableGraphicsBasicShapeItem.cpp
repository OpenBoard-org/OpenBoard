#include "UB1HEditableGraphicsBasicShapeItem.h"

#include "UBAbstractHandlesBuilder.h"

UB1HEditableGraphicsBasicShapeItem::UB1HEditableGraphicsBasicShapeItem(QGraphicsItem *parent):
    UBAbstractEditableGraphicsShapeItem(parent)
{
    UB1HandleBuilder::buildHandles(mHandles);

    for(int i = 0; i < mHandles.size(); i++){
        mHandles.at(i)->setEditableObject(this);
        mHandles.at(i)->setParentItem(this);
    }
}

QRectF UB1HEditableGraphicsBasicShapeItem::adjustBoundingRect(QRectF rect) const
{
    rect = UBAbstractEditableGraphicsShapeItem::adjustBoundingRect(rect);

    if(isInEditMode()){
        qreal r = getHandle()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}
