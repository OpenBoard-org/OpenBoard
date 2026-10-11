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


#include "UBAbstractEditableGraphicsShapeItem.h"

#include "domain/UBGraphicsDelegateFrame.h"
#include "domain/UBGraphicsScene.h"

UBAbstractEditableGraphicsShapeItem::UBAbstractEditableGraphicsShapeItem(QGraphicsItem* parent)
    : UBAbstractGraphicsItem(parent)
{
    mHasMoved = false;
}

void UBAbstractEditableGraphicsShapeItem::onActivateEditionMode()
{
    // NOOP
}

void UBAbstractEditableGraphicsShapeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    mHasMoved = false;
}

void UBAbstractEditableGraphicsShapeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
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

void UBAbstractEditableGraphicsShapeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
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

void UBAbstractEditableGraphicsShapeItem::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event)

    if (isInEditMode())
    {
        prepareGeometryChange();
        showEditMode(false);
    }
}

void UBAbstractEditableGraphicsShapeItem::paintCenterMark(QPainter* painter)
{
    if (mHasMoved || (Delegate() && Delegate()->frame() && Delegate()->frame()->moving()))
    {
        painter->save();

        // determine contrast color to item
        auto fillColor = painter->brush().color();

        if (fillColor == Qt::transparent)
        {
            // get background color
            fillColor = scene()->isDarkBackground() ? Qt::black : Qt::white;
        }

        fillColor = fillColor.toHsv();

        // compute a color which has the opposite color and brightness
        auto hue = fillColor.hueF() + 0.5;
        hue -= (long)hue;

        if (hue < 0.)
        {
            hue = 0;
        }

        const auto saturation = fillColor.saturationF();

        // prefer brighter colors
        auto value = fillColor.valueF() * fillColor.valueF() + 0.5;
        value -= (long)value;

        const auto markColor = QColor::fromHsvF(hue, saturation, value);

        painter->setBrush(QBrush());
        QPen p;
        p.setColor(markColor);
        p.setWidth(3);
        painter->setPen(p);

        const auto center = boundingRect().center();
        painter->drawLine(center - QPointF{5, 0}, center + QPointF{5, 0});
        painter->drawLine(center - QPointF{0, 5}, center + QPointF{0, 5});

        painter->restore();
    }
}

void UBAbstractEditableGraphicsShapeItem::deactivateEditionMode()
{
    prepareGeometryChange();

    showEditMode(false);
}

void UBAbstractEditableGraphicsShapeItem::addExtraShapes(QPainterPath& path) const
{
    addHandleShapes(this, path);
}
