/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#include "UB3HEditablesGraphicsBasicShapeItem.h"

#include "UBAbstractHandlesBuilder.h"

UB3HEditablesGraphicsBasicShapeItem::UB3HEditablesGraphicsBasicShapeItem(QGraphicsItem* parent)
    : UBAbstractEditableGraphicsShapeItem(parent)
{
    UB3HandlesBuilder::buildHandles(mHandles);

    for (int i = 0; i < mHandles.size(); i++)
    {
        mHandles.at(i)->setEditableObject(this);
        mHandles.at(i)->setParentItem(this);
    }
}

QRectF UB3HEditablesGraphicsBasicShapeItem::adjustBoundingRect(QRectF rect) const
{
    rect = UBAbstractEditableGraphicsShapeItem::adjustBoundingRect(rect);

    if (isInEditMode())
    {
        qreal r = getHandle(HandleId::Horizontal)->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}
