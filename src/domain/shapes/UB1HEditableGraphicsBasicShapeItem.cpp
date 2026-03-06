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


#include "UB1HEditableGraphicsBasicShapeItem.h"

#include "UBAbstractHandlesBuilder.h"

UB1HEditableGraphicsBasicShapeItem::UB1HEditableGraphicsBasicShapeItem(QGraphicsItem* parent)
    : UBAbstractEditableGraphicsShapeItem(parent)
{
    UB1HandleBuilder::buildHandles(mHandles);

    for (auto handle : mHandles)
    {
        handle->setEditableObject(this);
        handle->setParentItem(this);
    }
}

QRectF UB1HEditableGraphicsBasicShapeItem::adjustBoundingRect(QRectF rect) const
{
    rect = UBAbstractEditableGraphicsShapeItem::adjustBoundingRect(rect);

    if (isInEditMode())
    {
        qreal r = getHandle(HandleId::Diagonal)->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}
