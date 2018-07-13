/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBGraphicsRectItem.h"
#include "customWidgets/UBGraphicsItemAction.h"
#include "UBAbstractGraphicsItem.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"

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

UBItem *UB3HEditableGraphicsRectItem::deepCopy() const
{
    UB3HEditableGraphicsRectItem* copy = new UB3HEditableGraphicsRectItem();

    copyItemParameters(copy);

    return copy;
}

void UB3HEditableGraphicsRectItem::copyItemParameters(UBItem *copy) const
{
    UB3HEditablesGraphicsBasicShapeItem::copyItemParameters(copy);

    UB3HEditableGraphicsRectItem *cp = dynamic_cast<UB3HEditableGraphicsRectItem*>(copy);
    if (cp)
    {
        cp->setRect(this->rect());
    }
}

void UB3HEditableGraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    painter->drawRect(0, 0, mWidth, mHeight);
}

QPainterPath UB3HEditableGraphicsRectItem::shape() const
{
    QPainterPath path;

    if(isInEditMode()){
        path.addRect(boundingRect());
    }else{
        path.addRect(0, 0, mWidth, mHeight);
    }

    return path;
}

void UB3HEditableGraphicsRectItem::onActivateEditionMode()
{
    horizontalHandle()->setPos(mWidth, mHeight/2);
    verticalHandle()->setPos(mWidth/2, mHeight);
    diagonalHandle()->setPos(mWidth, mHeight);
}

void UB3HEditableGraphicsRectItem::updateHandle(UBAbstractHandle *handle)
{
    prepareGeometryChange();

    qreal maxSize = handle->radius() * 4;

    if(handle->getId() == 1){
        //it's the vertical handle
        if(handle->pos().y() >= maxSize){
            mHeight = handle->pos().y();
        }
    }else if(handle->getId() == 0){
        //it's the horizontal handle
        if(handle->pos().x() > maxSize){
            mWidth = handle->pos().x();
        }
    }else{
        //it's the diagonal handle
        if(handle->pos().x() >= maxSize && handle->pos().y() >= maxSize){
            float ratio = mHeight / mWidth;

            if(mWidth > mHeight){
                mWidth = handle->pos().x();
                mHeight = ratio * mWidth;
            }else{
                mHeight = handle->pos().y();
                mWidth = 1/ratio * mHeight;
            }
        }
    }

    horizontalHandle()->setPos(mWidth, mHeight/2);
    verticalHandle()->setPos(mWidth/2, mHeight);
    diagonalHandle()->setPos(mWidth, mHeight);

    if(hasGradient()){
        QLinearGradient g(QPointF(), QPointF(mWidth, 0));

        g.setColorAt(0, brush().gradient()->stops().at(0).second);
        g.setColorAt(1, brush().gradient()->stops().at(1).second);

        setBrush(g);
    }
}

QRectF UB3HEditableGraphicsRectItem::boundingRect() const
{
    int x = (mWidth < 0 ? mWidth : 0);
    int y = (mHeight < 0 ? mHeight : 0);

    int w = (mWidth < 0 ? -mWidth : mWidth);
    int h = (mHeight < 0 ? -mHeight : mHeight);

    QRectF rect(x, y, w, h);

    rect = adjustBoundingRect(rect);

    if(isInEditMode()){
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

    if(hasGradient()){
        QLinearGradient g(QPointF(), QPointF(mWidth, 0));

        g.setColorAt(0, brush().gradient()->stops().at(0).second);
        g.setColorAt(1, brush().gradient()->stops().at(1).second);

        setBrush(g);
    }
}

QRectF UB3HEditableGraphicsRectItem::rect() const
{
    QRectF r;
    r.setTopLeft(pos());
    r.setWidth(mWidth);
    r.setHeight(mHeight);

    return r;
}
