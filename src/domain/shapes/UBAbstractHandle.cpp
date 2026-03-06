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


#include "UBAbstractHandle.h"

#include "UBAbstractEditable.h"
#include "domain/UBGraphicsScene.h"

UBAbstractHandle::UBAbstractHandle()
{
    mId = 0;
    mClick = false;
    mRadius = 7;
    mEditableObject = 0;

    setData(UBGraphicsItemData::itemLayerType,
            QVariant(itemLayerType::ObjectItem)); // Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

UBAbstractHandle::UBAbstractHandle(UBAbstractHandle* const src)
{
    mId = src->mId;
    mClick = src->mClick;
    mRadius = src->mRadius;
    mEditableObject = src->mEditableObject;

    setPos(src->pos());

    setFlags(src->flags());
    setData(UBGraphicsItemData::itemLayerType, src->data(UBGraphicsItemData::itemLayerType));
}

void UBAbstractHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);

    if (mEditableObject)
    {
        mEditableObject->updateHandle(this);
        mEditableObject->setModified();
    }
}

void UBAbstractHandle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    mClick = true;
    QGraphicsItem::mousePressEvent(event);

    if (mEditableObject)
    {
        mEditableObject->focusHandle(this);
    }
}

void UBAbstractHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    mClick = false;
    QGraphicsItem::mouseReleaseEvent(event);
}


void UBAbstractHandle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QBrush brush(Qt::white);

    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);

    auto pen = painter->pen();
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2.);
    painter->setPen(pen);

    painter->drawEllipse(-mRadius, -mRadius, mRadius * 2, mRadius * 2);
}

QRectF UBAbstractHandle::boundingRect() const
{
    int d = mRadius * 2;
    int x = -mRadius;
    int y = x;

    return QRectF(x, y, d, d);
}

std::shared_ptr<UBGraphicsScene> UBAbstractHandle::scene()
{
    auto scenePtr = dynamic_cast<UBGraphicsScene*>(QGraphicsItem::scene());
    return scenePtr ? scenePtr->shared_from_this() : nullptr;
}
