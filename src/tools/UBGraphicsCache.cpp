/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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




#include <QDebug>

#include "UBGraphicsCache.h"

#include "core/UBApplication.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

QMap<UBGraphicsScene*, UBGraphicsCache*> UBGraphicsCache::sInstances;

UBGraphicsCache* UBGraphicsCache::instance(UBGraphicsScene *scene)
{
    if (!sInstances.contains(scene))
        sInstances.insert(scene, new UBGraphicsCache(scene));
    return sInstances[scene];
}

UBGraphicsCache::UBGraphicsCache(UBGraphicsScene *scene) : QGraphicsRectItem()
  , mMaskColor(Qt::black)
  , mMaskShape(eMaskShape_Circle)
  , mShapeWidth(100)
  , mDrawMask(false)
  , mScene(scene)
{
    // Get the board size and pass it to the shape
    QRect boardRect = UBApplication::boardController->displayView()->rect();
    setRect(-15*boardRect.width(), -15*boardRect.height(), 30*boardRect.width(), 30*boardRect.height());
    setData(Qt::UserRole, QVariant("Cache"));
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Cache)); //Necessary to set if we want z value to be assigned correctly
}

UBGraphicsCache::~UBGraphicsCache()
{
    sInstances.remove(mScene);
}

UBItem* UBGraphicsCache::deepCopy() const
{
    UBGraphicsCache* copy = new UBGraphicsCache(mScene);

    copyItemParameters(copy);

    // TODO UB 4.7 ... complete all members ?

    return copy;
}

void UBGraphicsCache::copyItemParameters(UBItem *copy) const
{
    UBGraphicsCache *cp = dynamic_cast<UBGraphicsCache*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setRect(this->rect());
        cp->setTransform(this->transform());
    }
}

QColor UBGraphicsCache::maskColor()
{
    return mMaskColor;
}

void UBGraphicsCache::setMaskColor(QColor color)
{
    mMaskColor = color;
    update();
}

eMaskShape UBGraphicsCache::maskshape()
{
    return mMaskShape;
}

void UBGraphicsCache::setMaskShape(eMaskShape shape)
{
    mMaskShape = shape;
    update();
}

void UBGraphicsCache::init()
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void UBGraphicsCache::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(mMaskColor);
    painter->setPen(mMaskColor);

    // Draw the hole
    QPainterPath path;
    path.addRect(rect());

    if(mDrawMask)
    {
        if(eMaskShape_Circle == mMaskShape)
        {
            path.addEllipse(mShapePos, mShapeWidth, mShapeWidth);
        }
        else if(eMaskShap_Rectangle == mMaskShape)
        {
            path.addRect(mShapePos.x() - mShapeWidth, mShapePos.y() - mShapeWidth, 2*mShapeWidth, 2*mShapeWidth);
        }
        path.setFillRule(Qt::OddEvenFill);
    }

    painter->drawPath(path);
}

void UBGraphicsCache::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    mShapePos = event->pos();
    mDrawMask = true;

    // Note: if refresh issues occure, replace the following 3 lines by: update();
    update(updateRect(event->pos()));
    mOldShapeWidth = mShapeWidth;
    mOldShapePos = event->pos();
}

void UBGraphicsCache::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mShapePos = event->pos();

    // Note: if refresh issues occure, replace the following 3 lines by: update();
    update(updateRect(event->pos()));
    mOldShapeWidth = mShapeWidth;
    mOldShapePos = event->pos();
}

void UBGraphicsCache::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    mDrawMask = false;

    // Note: if refresh issues occure, replace the following 3 lines by: update();
    update(updateRect(event->pos()));
    mOldShapeWidth = mShapeWidth;
    mOldShapePos = event->pos();
}

int UBGraphicsCache::shapeWidth()
{
    return mShapeWidth;
}

void UBGraphicsCache::setShapeWidth(int width)
{
    mShapeWidth = width;
    update();
}

QRectF UBGraphicsCache::updateRect(QPointF currentPoint)
{
    QRectF r;
    int x;
    int y;

    x = qMin(currentPoint.x() - mShapeWidth, mOldShapePos.x() - mOldShapeWidth);
    y = qMin(currentPoint.y() - mShapeWidth, mOldShapePos.y() - mOldShapeWidth);
    r = QRect(  x,
                y,
                qAbs(currentPoint.x() - mOldShapePos.x()) + 2*mShapeWidth,
                qAbs(currentPoint.y() - mOldShapePos.y()) + 2*mShapeWidth);
    return r;
}
