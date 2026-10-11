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


#include "UB1HEditableGraphicsSquareItem.h"


UB1HEditableGraphicsSquareItem::UB1HEditableGraphicsSquareItem(QGraphicsItem* parent)
    : UB1HEditableGraphicsBasicShapeItem(parent)
{
    // Rect has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    hIsNeg = false;
    wIsNeg = false;
}

UB1HEditableGraphicsSquareItem::~UB1HEditableGraphicsSquareItem()
{
}

ShapeType UB1HEditableGraphicsSquareItem::shapeType()
{
    return ShapeType::Square;
}

UBItem* UB1HEditableGraphicsSquareItem::deepCopy() const
{
    UB1HEditableGraphicsSquareItem* copy = new UB1HEditableGraphicsSquareItem();

    copyItemParameters(copy);

    return copy;
}

void UB1HEditableGraphicsSquareItem::copyItemParameters(UBItem* copy) const
{
    UB1HEditableGraphicsBasicShapeItem::copyItemParameters(copy);

    UB1HEditableGraphicsSquareItem* cp = dynamic_cast<UB1HEditableGraphicsSquareItem*>(copy);

    if (!cp)
        return;

    cp->mSide = mSide;
    cp->hIsNeg = hIsNeg;
    cp->wIsNeg = wIsNeg;
}

void UB1HEditableGraphicsSquareItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    int h = hIsNeg ? -mSide : mSide;
    int w = wIsNeg ? -mSide : mSide;

    painter->drawRect(0, 0, w, h);

    paintCenterMark(painter);
}

QPainterPath UB1HEditableGraphicsSquareItem::painterPath() const
{
    QPainterPath path;
    path.addRect(QRectF{0, 0, mSide, mSide}.normalized());
    return path;
}

void UB1HEditableGraphicsSquareItem::onActivateEditionMode()
{
    getHandle(HandleId::Diagonal)->setPos(mSide, mSide);
    getHandle(HandleId::Stretch)->setPos(mSide, 0);
}

void UB1HEditableGraphicsSquareItem::updateHandle(UBAbstractHandle* handle)
{
    prepareGeometryChange();

    qreal maxSize = handle->radius() * 4;

    if (handle->getId() == HandleId::Diagonal)
    {
        qreal side = qMin(handle->pos().x(), handle->pos().y());

        if (side >= maxSize)
        {
            mSide = side;
        }
    }
    else if (handle->getId() == HandleId::Stretch)
    {
        // it's the stretch handle
        if (handle->pos().x() >= maxSize)
        {
            double delta = handle->pos().x() - mSide;

            mSide += delta;

            setTransform(transform().translate(-delta / 2., -delta / 2.));
        }
    }

    getHandle(HandleId::Diagonal)->setPos(mSide, mSide);
    getHandle(HandleId::Stretch)->setPos(mSide, 0);
}

QRectF UB1HEditableGraphicsSquareItem::boundingRect() const
{
    int x = wIsNeg ? -mSide : 0;
    int y = hIsNeg ? -mSide : 0;

    QRectF rect(x, y, mSide, mSide);

    rect = adjustBoundingRect(rect);

    if (isInEditMode())
    {
        qreal r = mHandles.at(0)->radius();
        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UB1HEditableGraphicsSquareItem::setRect(QRectF rect)
{
    prepareGeometryChange();

    setPos(rect.topLeft());

    qreal w = rect.width();
    qreal h = rect.height();

    wIsNeg = w < 0;
    hIsNeg = h < 0;

    if (wIsNeg)
        w = -w;
    if (hIsNeg)
        h = -h;

    mSide = qMin(w, h);
}

QRectF UB1HEditableGraphicsSquareItem::rect() const
{
    QRectF r;
    r.setTopLeft(pos());

    r.setWidth(wIsNeg ? -mSide : mSide);
    r.setHeight(hIsNeg ? -mSide : mSide);

    return r;
}
