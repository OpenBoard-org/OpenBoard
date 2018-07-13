#include "UB3HandlesEditable.h"

#include "UBAbstractHandlesBuilder.h"

UB3HEditablesGraphicsBasicShapeItem::UB3HEditablesGraphicsBasicShapeItem(QGraphicsItem *parent):
    UBAbstractEditableGraphicsShapeItem(parent)
{
    UB3HandlesBuilder::buildHandles(mHandles);

    for(int i = 0; i < mHandles.size(); i++){
        mHandles.at(i)->setEditableObject(this);
        mHandles.at(i)->setParentItem(this);
    }
}

QRectF UB3HEditablesGraphicsBasicShapeItem::adjustBoundingRect(QRectF rect) const
{
    rect = UBAbstractEditableGraphicsShapeItem::adjustBoundingRect(rect);

    if(isInEditMode()){
        qreal r = horizontalHandle()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

