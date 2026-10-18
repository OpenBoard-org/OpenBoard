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


#include "UBAbstractEditable.h"

#include "UBHorizontalHandle.h"

#include "domain/UBGraphicsScene.h"

UBHorizontalHandle::UBHorizontalHandle()
{
    mId = Horizontal;
}

UBHorizontalHandle::UBHorizontalHandle(UBHorizontalHandle* const src):
    UBAbstractHandle(src)
{
}

void UBHorizontalHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    auto scenePos = event->scenePos();
    std::shared_ptr<UBGraphicsScene> scenePtr = scene();

    if (scenePtr->isSnapping())
    {
        scenePos += scenePtr->snap(scenePos);
    }

    QPointF p = parentItem()->mapFromScene(scenePos);

    this->setPos(p.x(), pos().y());

    mEditableObject->updateHandle(this);
    mEditableObject->setModified();
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
