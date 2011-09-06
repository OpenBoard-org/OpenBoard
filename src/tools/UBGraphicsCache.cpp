/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QDebug>

#include "UBGraphicsCache.h"

#include "core/UBApplication.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"


UBGraphicsCache::UBGraphicsCache():QGraphicsRectItem()
  , mMaskColor(Qt::black)
  , mMaskShape(eMaskShape_Circle)
  , mShapeWidth(100)
  , mDrawMask(false)
{
    // Get the board size and pass it to the shape
    QRect boardRect = UBApplication::boardController->displayView()->rect();
    setRect(-15*boardRect.width(), -15*boardRect.height(), 30*boardRect.width(), 30*boardRect.height());
    setZValue(CACHE_ZVALUE);
    setData(Qt::UserRole, QVariant("Cache"));
}

UBGraphicsCache::~UBGraphicsCache()
{

}

UBItem* UBGraphicsCache::deepCopy() const
{
    UBGraphicsCache* copy = new UBGraphicsCache();

    copy->setPos(this->pos());
    copy->setRect(this->rect());
    copy->setZValue(this->zValue());
    copy->setTransform(this->transform());

    // TODO UB 4.7 ... complete all members ?

    return copy;
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

    setZValue(CACHE_ZVALUE);

    painter->setBrush(mMaskColor);
    painter->setPen(mMaskColor);

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
            path.addRect(mShapePos.x() - mShapeWidth / 2, mShapePos.y() - mShapeWidth / 2, mShapeWidth, mShapeWidth);
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
    update();
}

void UBGraphicsCache::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mShapePos = event->pos();
    update();
}

void UBGraphicsCache::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    mDrawMask = false;
    update();
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
