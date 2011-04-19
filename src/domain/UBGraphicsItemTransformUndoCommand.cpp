/*
 * UBGraphicsItemTransformUndoCommand.cpp
 *
 *  Created on: July 7, 2009
 *      Author: Jerome
 */

#include "UBGraphicsItemTransformUndoCommand.h"
#include "UBResizableGraphicsItem.h"


UBGraphicsItemTransformUndoCommand::UBGraphicsItemTransformUndoCommand(QGraphicsItem* pItem,
     const QPointF& prevPos, const QTransform& prevTransform, const qreal& prevZValue,
     const QSizeF& prevSize)
{
    mItem = pItem;
    mPreviousTransform = prevTransform;
    mCurrentTransform = pItem->transform();

    mPreviousPosition = prevPos;
    mCurrentPosition = pItem->pos();

    mPreviousZValue = prevZValue;
    mCurrentZValue = pItem->zValue();

    mPreviousSize = prevSize;
    UBResizableGraphicsItem* resizableItem = dynamic_cast<UBResizableGraphicsItem*>(pItem);

    if (resizableItem)
        mCurrentSize = resizableItem->size();
}

UBGraphicsItemTransformUndoCommand::~UBGraphicsItemTransformUndoCommand()
{
    // NOOP
}

void UBGraphicsItemTransformUndoCommand::undo()
{
    mItem->setPos(mPreviousPosition);
    mItem->setTransform(mPreviousTransform);
    mItem->setZValue(mPreviousZValue);

    UBResizableGraphicsItem* resizableItem = dynamic_cast<UBResizableGraphicsItem*>(mItem);

    if (resizableItem)
        resizableItem->resize(mPreviousSize);
}

void UBGraphicsItemTransformUndoCommand::redo()
{
    mItem->setPos(mCurrentPosition);
    mItem->setTransform(mCurrentTransform);
    mItem->setZValue(mCurrentZValue);

    UBResizableGraphicsItem* resizableItem = dynamic_cast<UBResizableGraphicsItem*>(mItem);

    if (resizableItem)
        resizableItem->resize(mCurrentSize);
}
