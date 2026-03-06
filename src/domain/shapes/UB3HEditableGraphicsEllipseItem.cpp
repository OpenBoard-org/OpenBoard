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


#include "UB3HEditableGraphicsEllipseItem.h"

UB3HEditableGraphicsEllipseItem::UB3HEditableGraphicsEllipseItem(QGraphicsItem* parent)
    : UB3HEditablesGraphicsBasicShapeItem(parent)
{
    // Ellipse has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    mRadiusX = 0;
    mRadiusY = 0;
}

UB3HEditableGraphicsEllipseItem::~UB3HEditableGraphicsEllipseItem()
{
}

ShapeType UB3HEditableGraphicsEllipseItem::shapeType()
{
    return ShapeType::Ellipse;
}

UBItem* UB3HEditableGraphicsEllipseItem::deepCopy() const
{
    UB3HEditableGraphicsEllipseItem* copy = new UB3HEditableGraphicsEllipseItem();

    copyItemParameters(copy);

    return copy;
}

void UB3HEditableGraphicsEllipseItem::copyItemParameters(UBItem* copy) const
{
    UB3HEditablesGraphicsBasicShapeItem::copyItemParameters(copy);

    UB3HEditableGraphicsEllipseItem* cp = dynamic_cast<UB3HEditableGraphicsEllipseItem*>(copy);

    if (cp)
    {
        cp->mRadiusX = mRadiusX;
        cp->mRadiusY = mRadiusY;
    }
}

QPointF UB3HEditableGraphicsEllipseItem::center() const
{
    QPointF centre;

    centre.setX(pos().x() + mRadiusX);
    centre.setY(pos().y() + mRadiusY);

    return centre;
}

void UB3HEditableGraphicsEllipseItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    setStyle(painter);

    auto rx = (mRadiusX < 0 ? -mRadiusX : mRadiusX);
    auto ry = (mRadiusY < 0 ? -mRadiusY : mRadiusY);

    auto x = (mRadiusX < 0 ? mRadiusX : 0);
    auto y = (mRadiusY < 0 ? mRadiusY : 0);

    // N/C - NNE - 20140312 : Litle work around for avoid crash under MacOs 10.9
    QPainterPath path;
    path.addEllipse(QRectF(x * 2, y * 2, rx * 2, ry * 2));

    painter->drawPath(path);

    if (isInEditMode())
    {
        QPen p;
        p.setColor(QColor(128, 128, 200));
        p.setStyle(Qt::DotLine);
        p.setWidth(3);

        painter->setPen(p);
        painter->setBrush(QBrush());

        painter->drawRect(0, 0, mRadiusX * 2, mRadiusY * 2);
    }

    paintCenterMark(painter);
}

QRectF UB3HEditableGraphicsEllipseItem::boundingRect() const
{
    auto x = (mRadiusX < 0 ? mRadiusX : 0);
    auto y = (mRadiusY < 0 ? mRadiusY : 0);

    auto rx = (mRadiusX < 0 ? -mRadiusX : mRadiusX);
    auto ry = (mRadiusY < 0 ? -mRadiusY : mRadiusY);

    rx *= 2;
    ry *= 2;

    x *= 2;
    y *= 2;

    QRectF rect(x, y, rx, ry);

    rect = adjustBoundingRect(rect);

    return rect;
}

void UB3HEditableGraphicsEllipseItem::onActivateEditionMode()
{
    getHandle(HandleId::Vertical)->setPos(mRadiusX, mRadiusY * 2);
    getHandle(HandleId::Horizontal)->setPos(mRadiusX * 2, mRadiusY);
    getHandle(HandleId::Diagonal)->setPos(mRadiusX * 2, mRadiusY * 2);
    getHandle(HandleId::Stretch)->setPos(mRadiusX * 2, 0);
}

void UB3HEditableGraphicsEllipseItem::updateHandle(UBAbstractHandle* handle)
{
    prepareGeometryChange();

    qreal maxSize = handle->radius() * 4;

    if (handle->getId() == HandleId::Vertical)
    {
        // it's the vertical handle
        if (handle->pos().y() >= maxSize)
        {
            mRadiusY = handle->pos().y() / 2;
        }
    }
    else if (handle->getId() == HandleId::Horizontal)
    {
        // it's the horizontal handle
        if (handle->pos().x() >= maxSize)
        {
            mRadiusX = handle->pos().x() / 2;
        }
    }
    else if (handle->getId() == HandleId::Diagonal)
    {
        // it's the diagonal handle
        if (handle->pos().x() >= maxSize && handle->pos().y() >= maxSize)
        {
            float ratio = mRadiusY / mRadiusX;

            if (mRadiusX > mRadiusY)
            {
                mRadiusX = handle->pos().x() / 2;
                mRadiusY = ratio * mRadiusX;
            }
            else
            {
                mRadiusY = handle->pos().y() / 2;
                mRadiusX = 1 / ratio * mRadiusY;
            }
        }
    }
    else if (handle->getId() == HandleId::Stretch)
    {
        // it's the stretch handle
        if (handle->pos().x() >= maxSize)
        {
            double ratio = mRadiusY / mRadiusX;
            double dx;
            double dy;

            if (mRadiusX > mRadiusY)
            {
                dx = handle->pos().x() / 2. - mRadiusX;
                dy = ratio * dx;
            }
            else
            {
                dy = handle->pos().y() / 2. - mRadiusY;
                dx = dy / ratio;
            }

            mRadiusX += dx;
            mRadiusY += dy;

            setTransform(transform().translate(-dx, -dy));
        }
    }

    getHandle(HandleId::Vertical)->setPos(mRadiusX, mRadiusY * 2);
    getHandle(HandleId::Horizontal)->setPos(mRadiusX * 2, mRadiusY);
    getHandle(HandleId::Diagonal)->setPos(mRadiusX * 2, mRadiusY * 2);
    getHandle(HandleId::Stretch)->setPos(mRadiusX * 2, 0);
}

QPainterPath UB3HEditableGraphicsEllipseItem::painterPath() const
{
    QPainterPath path;
    path.addEllipse({mRadiusX, mRadiusY}, mRadiusX, mRadiusY);
    return path;
}

void UB3HEditableGraphicsEllipseItem::setRadiusX(qreal radius)
{
    prepareGeometryChange();
    mRadiusX = radius;
}

void UB3HEditableGraphicsEllipseItem::setRadiusY(qreal radius)
{
    prepareGeometryChange();
    mRadiusY = radius;
}

void UB3HEditableGraphicsEllipseItem::setRect(QRectF rect)
{
    prepareGeometryChange();

    setPos(rect.topLeft());
    mRadiusX = rect.width() / 2;
    mRadiusY = rect.height() / 2;
}

QRectF UB3HEditableGraphicsEllipseItem::rect() const
{
    QRectF r;
    r.setTopLeft(pos());
    r.setWidth(mRadiusX * 2);
    r.setHeight(mRadiusY * 2);

    return r;
}

qreal UB3HEditableGraphicsEllipseItem::radiusX() const
{
    return mRadiusX;
}

qreal UB3HEditableGraphicsEllipseItem::radiusY() const
{
    return mRadiusY;
}
