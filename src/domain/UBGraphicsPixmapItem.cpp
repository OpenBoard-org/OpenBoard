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




#include "UBGraphicsPixmapItem.h"

#include <QtGui>
#include <QMimeData>
#include <QDrag>

#include "UBGraphicsScene.h"

#include "UBGraphicsItemDelegate.h"

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "board/UBBoardController.h"

#include "core/memcheck.h"

UBGraphicsPixmapItem::UBGraphicsPixmapItem(QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
{
    setDelegate(new UBGraphicsItemDelegate(this, 0, GF_COMMON
                                           | GF_FLIPPABLE_ALL_AXIS
                                           | GF_REVOLVABLE
                                           | GF_RESPECT_RATIO
                                           | GF_TOOLBAR_USED));

    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    setTransformationMode(Qt::SmoothTransformation);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setData(UBGraphicsItemData::ItemCanBeSetAsBackground, true);

    setUuid(QUuid::createUuid()); //more logical solution is in creating uuid for element in element's constructor
}

UBGraphicsPixmapItem::~UBGraphicsPixmapItem()
{
}

QVariant UBGraphicsPixmapItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = Delegate()->itemChange(change, value);
    return QGraphicsPixmapItem::itemChange(change, newValue);
}

void UBGraphicsPixmapItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid));
}

void UBGraphicsPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QMimeData* pMime = new QMimeData();
    pMime->setImageData(pixmap().toImage());
    Delegate()->setMimeData(pMime);
    qreal k = (qreal)pixmap().width() / 100.0;

    QSize newSize((int)(pixmap().width() / k), (int)(pixmap().height() / k));

    Delegate()->setDragPixmap(pixmap().scaled(newSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    if (Delegate()->mousePressEvent(event))
    {
        //NOOP
    }
    else
    {
//        QGraphicsPixmapItem::mousePressEvent(event);
    }
}

void UBGraphicsPixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mouseMoveEvent(event))
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
    Delegate()->mouseReleaseEvent(event);
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}


void UBGraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing, false);

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);

    styleOption.state &= ~QStyle::State_Selected;
    QGraphicsPixmapItem::paint(painter, &styleOption, widget);
    Delegate()->postpaint(painter, option, widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
}


UBItem* UBGraphicsPixmapItem::deepCopy() const
{
   UBGraphicsPixmapItem* copy = new UBGraphicsPixmapItem();

   copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable

   copyItemParameters(copy);

   // TODO UB 4.7 ... complete all members ?

   return copy;
}

void UBGraphicsPixmapItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsPixmapItem *cp = dynamic_cast<UBGraphicsPixmapItem*>(copy);
    if (cp)
    {
        cp->setPixmap(this->pixmap());
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setSourceUrl(this->sourceUrl());

        cp->setZValue(this->zValue());
    }
}

UBGraphicsScene* UBGraphicsPixmapItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsPixmapItem::setOpacity(qreal op)
{
    QGraphicsPixmapItem::setOpacity(op);
}


qreal UBGraphicsPixmapItem::opacity() const
{
    return QGraphicsPixmapItem::opacity();
}


void UBGraphicsPixmapItem::clearSource()
{
    QString fileName = UBPersistenceManager::imageDirectory + "/" + uuid().toString() + ".png";
    QString diskPath =  UBApplication::boardController->selectedDocument()->persistencePath() + "/" + fileName;
    UBFileSystemUtils::deleteFile(diskPath);
}
