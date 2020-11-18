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




#include "UBGraphicsPDFItem.h"

#include <QtGui>

#include "UBGraphicsScene.h"
#include "UBGraphicsPixmapItem.h"
#include "UBGraphicsItemDelegate.h"

#include "core/memcheck.h"

UBGraphicsPDFItem::UBGraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem* parent)
    : GraphicsPDFItem(renderer, pageNumber, parent)
{
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object); //deprecated
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::BackgroundItem)); //Necessary to set if we want z value to be assigned correctly

    setDelegate(new UBGraphicsItemDelegate(this, 0, GF_COMMON));
}


UBGraphicsPDFItem::~UBGraphicsPDFItem()
{
}


QVariant UBGraphicsPDFItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = Delegate()->itemChange(change, value);
    return GraphicsPDFItem::itemChange(change, newValue);
}

void UBGraphicsPDFItem::updateChild()
{
    CacheMode prevCacheMode = cacheMode();

    GraphicsPDFItem::update();

    // Workaround: Necessary, otherwise only the control scene is updated, the display scene refresh is ignored for an unknown reason.
    setCacheMode(prevCacheMode);
}

void UBGraphicsPDFItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid));
}

void UBGraphicsPDFItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mousePressEvent(event))
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
    if (Delegate()->mouseMoveEvent(event))
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
    Delegate()->mouseReleaseEvent(event);
    GraphicsPDFItem::mouseReleaseEvent(event);
}

void UBGraphicsPDFItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicsPDFItem::paint(painter, option, widget);
    Delegate()->postpaint(painter, option, widget);
}

UBItem* UBGraphicsPDFItem::deepCopy() const
{
    UBGraphicsPDFItem *copy =  new UBGraphicsPDFItem(mRenderer, mPageNumber, parentItem());

    copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsPDFItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsPDFItem *cp = dynamic_cast<UBGraphicsPDFItem*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setSourceUrl(this->sourceUrl());
        cp->setZValue(this->zValue());
    }
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

void UBGraphicsPDFItem::setCacheBehavior(UBItem::CacheBehavior cacheBehavior)
{
    UBItem::setCacheBehavior(cacheBehavior);
    GraphicsPDFItem::setCacheAllowed(cacheBehavior == UBItem::CacheAllowed);
}

UBGraphicsScene* UBGraphicsPDFItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


UBGraphicsPixmapItem* UBGraphicsPDFItem::toPixmapItem() const
{   
    QPixmap pixmap(mRenderer->pageSizeF(mPageNumber).toSize());
    QPainter painter(&pixmap);
    mRenderer->render(&painter, mPageNumber, false /* Cache allowed */);

    UBGraphicsPixmapItem *pixmapItem =  new UBGraphicsPixmapItem();
    pixmapItem->setPixmap(pixmap);

    pixmapItem->setPos(this->pos());
    pixmapItem->setTransform(this->transform());
    pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    pixmapItem->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));

    return pixmapItem;
}


