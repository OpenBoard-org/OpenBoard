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

#include "UBVerticalHandle.h"

#include "domain/UBGraphicsScene.h"

UBVerticalHandle::UBVerticalHandle(bool paintIndicators)
    : mPaintIndicators{paintIndicators}
{
    mId = paintIndicators ? Other : Vertical;
}

UBVerticalHandle::UBVerticalHandle(UBVerticalHandle* const src)
    : UBAbstractHandle(src)
    , mPaintIndicators{src->mPaintIndicators}
{
}

void UBVerticalHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    auto scenePos = event->scenePos();
    std::shared_ptr<UBGraphicsScene> scenePtr = scene();

    if (scenePtr->isSnapping())
    {
        scenePos += scenePtr->snap(scenePos);
    }

    QPointF p = parentItem()->mapFromScene(scenePos);

    this->setPos(pos().x(), p.y());

    mEditableObject->updateHandle(this);
    mEditableObject->setModified();
}

void UBVerticalHandle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    UBAbstractHandle::mousePressEvent(event);
}

void UBVerticalHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    UBAbstractHandle::mouseReleaseEvent(event);
}

void UBVerticalHandle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    UBAbstractHandle::paint(painter, option, widget);

    if (mPaintIndicators)
    {
        const auto height = mRadius + 2;
        const auto width = height;
        const auto gap = 5;

        QPainterPath path;

        path.moveTo(-width, -mRadius - gap);
        path.lineTo(0, -mRadius - height - gap);
        path.lineTo(width, -mRadius - gap);
        path.closeSubpath();

        path.moveTo(-width, mRadius + gap);
        path.lineTo(0, mRadius + height + gap);
        path.lineTo(width, mRadius + gap);
        path.closeSubpath();

        painter->drawPath(path);
    }
}

UBItem* UBVerticalHandle::deepCopy() const
{
    UBVerticalHandle* copy = new UBVerticalHandle();

    copyItemParameters(copy);

    return copy;
}

void UBVerticalHandle::copyItemParameters(UBItem* copy) const
{
    UBVerticalHandle* cp = dynamic_cast<UBVerticalHandle*>(copy);

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
