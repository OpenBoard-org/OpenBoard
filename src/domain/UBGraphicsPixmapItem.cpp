/*
 * UBGraphicsPixmapItem.cpp
 *
 *  Created on: Sep 18, 2008
 *      Author: luc
 */

#include "UBGraphicsPixmapItem.h"

#include <QtGui>
#include <QMimeData>
#include <QDrag>

#include "UBGraphicsScene.h"

#include "UBGraphicsItemDelegate.h"

#include "core/memcheck.h"

UBGraphicsPixmapItem::UBGraphicsPixmapItem(QGraphicsItem* parent/*, QString pSource*/)
    : QGraphicsPixmapItem(parent)
{
    mDelegate = new UBGraphicsItemDelegate(this, 0, true, true);
    mDelegate->init();
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    setTransformationMode(Qt::SmoothTransformation);

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

UBGraphicsPixmapItem::~UBGraphicsPixmapItem()
{
    if (mDelegate)
        delete mDelegate;
}

QVariant UBGraphicsPixmapItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = mDelegate->itemChange(change, value);
    return QGraphicsPixmapItem::itemChange(change, newValue);
}

void UBGraphicsPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QMimeData* pMime = new QMimeData();
    pMime->setImageData(pixmap().toImage());
    mDelegate->setMimeData(pMime);
    if (mDelegate->mousePressEvent(event))
    {
        //NOOP
    }
    else
    {
        QGraphicsPixmapItem::mousePressEvent(event);
    }
}

void UBGraphicsPixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mouseMoveEvent(event))
    {
        // NOOP;
    }
    else
    {
        QGraphicsPixmapItem::mouseMoveEvent(event);
    }
}

void UBGraphicsPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mDelegate->mouseReleaseEvent(event);
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}


void UBGraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;

    QGraphicsPixmapItem::paint(painter, &styleOption, widget);
}


UBItem* UBGraphicsPixmapItem::deepCopy() const
{
   UBGraphicsPixmapItem* copy = new UBGraphicsPixmapItem();

   copy->setPixmap(this->pixmap());
   copy->setPos(this->pos());
   copy->setZValue(this->zValue());
   copy->setTransform(this->transform());
   copy->setFlag(QGraphicsItem::ItemIsMovable, true);
   copy->setFlag(QGraphicsItem::ItemIsSelectable, true);
   copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
   copy->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));

   copy->setUuid(this->uuid()); // This is OK for now, as long as pixmaps are immutable -
   copy->setSourceUrl(this->sourceUrl());

   // TODO UB 4.7 ... complete all members ?

   return copy;
}


UBGraphicsScene* UBGraphicsPixmapItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsPixmapItem::remove()
{
    if (mDelegate)
        mDelegate->remove(true);
}


void UBGraphicsPixmapItem::setOpacity(qreal op)
{
    QGraphicsPixmapItem::setOpacity(op);
}


qreal UBGraphicsPixmapItem::opacity() const
{
    return QGraphicsPixmapItem::opacity();
}
