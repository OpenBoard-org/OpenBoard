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


#include "UBDiagonalHandle.h"

#include "UBAbstractEditable.h"
#include "domain/UBGraphicsScene.h"

UBDiagonalHandle::UBDiagonalHandle()
{
    mId = Diagonal;
}

UBDiagonalHandle::UBDiagonalHandle(UBDiagonalHandle* const src)
    : UBAbstractHandle(src)
{
}

void UBDiagonalHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    auto scenePos = event->scenePos();
    const std::shared_ptr<UBGraphicsScene> scenePtr = scene();

    if (scenePtr->isSnapping())
    {
        scenePos += scenePtr->snap(scenePos);
    }

    QPointF p = parentItem()->mapFromScene(scenePos);

    QPointF diff(p - pos());
    moveBy(diff.x(), diff.y());
    mEditableObject->updateHandle(this);
    mEditableObject->setModified();
}

void UBDiagonalHandle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    UBAbstractHandle::mousePressEvent(event);
}

void UBDiagonalHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    UBAbstractHandle::mouseReleaseEvent(event);
}

UBItem* UBDiagonalHandle::deepCopy() const
{
    UBDiagonalHandle* copy = new UBDiagonalHandle();

    copyItemParameters(copy);

    return copy;
}

void UBDiagonalHandle::copyItemParameters(UBItem* copy) const
{
    UBDiagonalHandle* cp = dynamic_cast<UBDiagonalHandle*>(copy);
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
