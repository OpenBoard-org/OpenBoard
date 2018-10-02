/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBGraphicsItemTransformUndoCommand.h"
#include "UBResizableGraphicsItem.h"
#include "domain/UBItem.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

UBGraphicsItemTransformUndoCommand::UBGraphicsItemTransformUndoCommand(QGraphicsItem* pItem,
     const QPointF& prevPos, const QTransform& prevTransform, const qreal& prevZValue,
     const QSizeF& prevSize, bool setToBackground)
    : UBUndoCommand()
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

    mSetToBackground = setToBackground;
}

UBGraphicsItemTransformUndoCommand::~UBGraphicsItemTransformUndoCommand()
{
    // NOOP
}

void UBGraphicsItemTransformUndoCommand::undo()
{
    if (mSetToBackground) {
        UBGraphicsScene* scene = dynamic_cast<UBGraphicsScene*>(mItem->scene());
        if (scene && scene->isBackgroundObject(mItem)) {
            scene->unsetBackgroundObject();
        }
    }

    mItem->setPos(mPreviousPosition);
    mItem->setTransform(mPreviousTransform);
    mItem->setZValue(mPreviousZValue);

    UBResizableGraphicsItem* resizableItem = dynamic_cast<UBResizableGraphicsItem*>(mItem);

    if (resizableItem)
        resizableItem->resize(mPreviousSize);
}

void UBGraphicsItemTransformUndoCommand::redo()
{
    if (mSetToBackground) {
        UBGraphicsScene* scene = dynamic_cast<UBGraphicsScene*>(mItem->scene());
        if (scene)
            scene->setAsBackgroundObject(mItem);
    }

    mItem->setPos(mCurrentPosition);
    mItem->setTransform(mCurrentTransform);
    mItem->setZValue(mCurrentZValue);

    UBResizableGraphicsItem* resizableItem = dynamic_cast<UBResizableGraphicsItem*>(mItem);

    if (resizableItem)
        resizableItem->resize(mCurrentSize);
}
