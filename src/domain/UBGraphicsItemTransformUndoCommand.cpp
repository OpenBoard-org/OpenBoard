/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBGraphicsItemTransformUndoCommand.h"
#include "UBResizableGraphicsItem.h"
#include "domain/UBItem.h"

#include "core/memcheck.h"

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
