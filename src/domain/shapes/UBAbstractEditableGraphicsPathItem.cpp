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


#include "UBAbstractEditableGraphicsPathItem.h"

UBAbstractEditableGraphicsPathItem::UBAbstractEditableGraphicsPathItem(QGraphicsItem* parent)
    : UBAbstractGraphicsPathItem(parent)
{
    mHasMoved = false;
}

void UBAbstractEditableGraphicsPathItem::onActivateEditionMode()
{
    // NOOP
}

void UBAbstractEditableGraphicsPathItem::addExtraShapes(QPainterPath& path) const
{
    addHandleShapes(this, path);
}

void UBAbstractEditableGraphicsPathItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    mHasMoved = false;
}

void UBAbstractEditableGraphicsPathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    prepareGeometryChange();

    if (!mHasMoved)
    {
        if (!Delegate()->isLocked())
        {
            if (!isInEditMode() && isSelected())
            {
                onActivateEditionMode();

                Delegate()->showFrame(false);
                setFocus();
                showEditMode(true);
            }
            else
            {
                showEditMode(false);
                Delegate()->positionHandles();
                Delegate()->showFrame(true);
            }
        }
    }

    mHasMoved = false;
}

QRectF UBAbstractEditableGraphicsPathItem::boundingRect() const
{
    QRectF rect = path().boundingRect();

    rect = UBAbstractGraphicsPathItem::adjustBoundingRect(rect);

    if (isInEditMode())
    {
        qreal r = mHandles.first()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UBAbstractEditableGraphicsPathItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!Delegate()->isLocked())
    {
        mHasMoved = true;

        if (!isInEditMode())
        {
            Delegate()->mouseMoveEvent(event);
        }
    }
}

void UBAbstractEditableGraphicsPathItem::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event)

    if (isInEditMode())
    {
        prepareGeometryChange();
        showEditMode(false);
    }
}

void UBAbstractEditableGraphicsPathItem::focusHandle(UBAbstractHandle* handle)
{
    Q_UNUSED(handle)

    Delegate()->showFrame(false);
}

void UBAbstractEditableGraphicsPathItem::deactivateEditionMode()
{
    prepareGeometryChange();

    showEditMode(false);
}
