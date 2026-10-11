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


#include "UB3HEditableGraphicsRectItem.h"

UB3HEditableGraphicsRectItem::UB3HEditableGraphicsRectItem(QGraphicsItem* parent)
    : UB3HEditablesGraphicsBasicShapeItem(parent)
{
    // Rect has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    mWidth = 0;
    mHeight = 0;
}

UB3HEditableGraphicsRectItem::~UB3HEditableGraphicsRectItem()
{
}

ShapeType UB3HEditableGraphicsRectItem::shapeType()
{
    return ShapeType::Rectangle;
}

UBItem* UB3HEditableGraphicsRectItem::deepCopy() const
{
    UB3HEditableGraphicsRectItem* copy = new UB3HEditableGraphicsRectItem();

    copyItemParameters(copy);

    return copy;
}

void UB3HEditableGraphicsRectItem::copyItemParameters(UBItem* copy) const
{
    UB3HEditablesGraphicsBasicShapeItem::copyItemParameters(copy);

    UB3HEditableGraphicsRectItem* cp = dynamic_cast<UB3HEditableGraphicsRectItem*>(copy);
    if (cp)
    {
        cp->setRect(this->rect());
    }
}

void UB3HEditableGraphicsRectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    painter->drawRect(0, 0, mWidth, mHeight);

    paintCenterMark(painter);
}

QPainterPath UB3HEditableGraphicsRectItem::painterPath() const
{
    QPainterPath path;
    path.addRect(0, 0, mWidth, mHeight);
    return path;
}

void UB3HEditableGraphicsRectItem::onActivateEditionMode()
{
    getHandle(HandleId::Horizontal)->setPos(mWidth, mHeight / 2);
    getHandle(HandleId::Vertical)->setPos(mWidth / 2, mHeight);
    getHandle(HandleId::Diagonal)->setPos(mWidth, mHeight);
    getHandle(HandleId::Stretch)->setPos(mWidth, 0);
}

void UB3HEditableGraphicsRectItem::updateHandle(UBAbstractHandle* handle)
{
    prepareGeometryChange();

    qreal maxSize = handle->radius() * 4;

    if (handle->getId() == HandleId::Vertical)
    {
        // it's the vertical handle
        if (handle->pos().y() >= maxSize)
        {
            mHeight = handle->pos().y();
        }
    }
    else if (handle->getId() == HandleId::Horizontal)
    {
        // it's the horizontal handle
        if (handle->pos().x() > maxSize)
        {
            mWidth = handle->pos().x();
        }
    }
    else if (handle->getId() == HandleId::Diagonal)
    {
        // it's the diagonal handle
        if (handle->pos().x() >= maxSize && handle->pos().y() >= maxSize)
        {
            float ratio = mHeight / mWidth;

            if (mWidth > mHeight)
            {
                mWidth = handle->pos().x();
                mHeight = ratio * mWidth;
            }
            else
            {
                mHeight = handle->pos().y();
                mWidth = 1 / ratio * mHeight;
            }
        }
    }
    else if (handle->getId() == HandleId::Stretch)
    {
        // it's the stretch handle
        if (handle->pos().x() >= maxSize)
        {
            double ratio = mHeight / mWidth;
            double dx;
            double dy;

            if (mWidth > mHeight)
            {
                dx = handle->pos().x() - mWidth;
                dy = ratio * dx;
            }
            else
            {
                dy = handle->pos().y() - mHeight;
                dx = dy / ratio;
            }

            mWidth += 2 * dx;
            mHeight += 2 * dy;

            setTransform(transform().translate(-dx, -dy));
        }
    }

    getHandle(HandleId::Horizontal)->setPos(mWidth, mHeight / 2);
    getHandle(HandleId::Vertical)->setPos(mWidth / 2, mHeight);
    getHandle(HandleId::Diagonal)->setPos(mWidth, mHeight);
    getHandle(HandleId::Stretch)->setPos(mWidth, 0);
}

QRectF UB3HEditableGraphicsRectItem::boundingRect() const
{
    int x = (mWidth < 0 ? mWidth : 0);
    int y = (mHeight < 0 ? mHeight : 0);

    int w = (mWidth < 0 ? -mWidth : mWidth);
    int h = (mHeight < 0 ? -mHeight : mHeight);

    QRectF rect(x, y, w, h);

    rect = adjustBoundingRect(rect);

    if (isInEditMode())
    {
        qreal r = mHandles.at(0)->radius();
        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UB3HEditableGraphicsRectItem::setRect(QRectF rect)
{
    prepareGeometryChange();

    setPos(rect.topLeft());

    mWidth = rect.width();
    mHeight = rect.height();
}

QRectF UB3HEditableGraphicsRectItem::rect() const
{
    QRectF r;
    r.setTopLeft(pos());
    r.setWidth(mWidth);
    r.setHeight(mHeight);

    return r;
}
