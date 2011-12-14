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

#include "UBGraphicsPDFItem.h"

#include <QtGui>

#include "UBGraphicsScene.h"
#include "UBGraphicsPixmapItem.h"
#include "UBGraphicsItemDelegate.h"

#include "core/memcheck.h"

UBGraphicsPDFItem::UBGraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem* parent)
    : GraphicsPDFItem(renderer, pageNumber, parent)
{
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    mDelegate = new UBGraphicsItemDelegate(this,0);
    mDelegate->init();
}


UBGraphicsPDFItem::~UBGraphicsPDFItem()
{
    if (mDelegate)
        delete mDelegate;
}


QVariant UBGraphicsPDFItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = mDelegate->itemChange(change, value);
    return GraphicsPDFItem::itemChange(change, newValue);
}


void UBGraphicsPDFItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mousePressEvent(event))
    {
        // NOOP
    }
    else
    {
        GraphicsPDFItem::mousePressEvent(event);
    }
}


void UBGraphicsPDFItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mouseMoveEvent(event))
    {
        // NOOP
    }
    else
    {
        GraphicsPDFItem::mouseMoveEvent(event);
    }
}


void UBGraphicsPDFItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mDelegate->mouseReleaseEvent(event);
    GraphicsPDFItem::mouseReleaseEvent(event);
}


UBItem* UBGraphicsPDFItem::deepCopy() const
{
    UBGraphicsPDFItem *copy =  new UBGraphicsPDFItem(mRenderer, mPageNumber, parentItem());

    copy->setPos(this->pos());
//    copy->setZValue(this->zValue());
    UBGraphicsItem::assignZValue(copy, this->zValue());
    copy->setTransform(this->transform());
    copy->setFlag(QGraphicsItem::ItemIsMovable, true);
    copy->setFlag(QGraphicsItem::ItemIsSelectable, true);
    copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    copy->setUuid(this->uuid()); // this is OK for now as long as PDF are imutable
    copy->setSourceUrl(this->sourceUrl());

    return copy;
}


void UBGraphicsPDFItem::setRenderingQuality(RenderingQuality pRenderingQuality)
{
    UBItem::setRenderingQuality(pRenderingQuality);

    if (pRenderingQuality == RenderingQualityHigh)
    {
        setCacheMode(QGraphicsItem::NoCache);
    }
    else
    {
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
}


UBGraphicsScene* UBGraphicsPDFItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsPDFItem::remove()
{
    if (mDelegate)
        mDelegate->remove(true);
}


UBGraphicsPixmapItem* UBGraphicsPDFItem::toPixmapItem() const
{   
    QPixmap pixmap(mRenderer->pageSizeF(mPageNumber).toSize());
    QPainter painter(&pixmap);
    mRenderer->render(&painter, mPageNumber);

    UBGraphicsPixmapItem *pixmapItem =  new UBGraphicsPixmapItem();
    pixmapItem->setPixmap(pixmap);

    pixmapItem->setPos(this->pos());
//    pixmapItem->setZValue(this->zValue());
    UBGraphicsItem::assignZValue(pixmapItem, this->zValue());
    pixmapItem->setTransform(this->transform());
    pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    pixmapItem->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));

    return pixmapItem;
}
