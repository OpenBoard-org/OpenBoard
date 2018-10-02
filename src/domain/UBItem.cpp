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




#include "UBItem.h"

#include "core/memcheck.h"

#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsStrokesGroup.h"
#include "domain/UBGraphicsGroupContainerItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsScene.h"
#include "tools/UBGraphicsCurtainItem.h"
#include "domain/UBGraphicsItemDelegate.h"

UBItem::UBItem()
    : mUuid(QUuid())
    , mRenderingQuality(UBItem::RenderingQualityNormal)
{
    // NOOP
}

UBItem::~UBItem()
{
    // NOOP
}

UBGraphicsItem::~UBGraphicsItem()
{
    if (mDelegate!=NULL)
        delete mDelegate;
}

void UBGraphicsItem::setDelegate(UBGraphicsItemDelegate* delegate)
{
    Q_ASSERT(mDelegate==NULL);
    mDelegate = delegate;
}

UBGraphicsItemDelegate *UBGraphicsItem::Delegate() const
{
    return mDelegate;
}

void UBGraphicsItem::assignZValue(QGraphicsItem *item, qreal value)
{
    item->setZValue(value);
    item->setData(UBGraphicsItemData::ItemOwnZValue, value);
}

bool UBGraphicsItem::isFlippable(QGraphicsItem *item)
{
    return item->data(UBGraphicsItemData::ItemFlippable).toBool();
}

bool UBGraphicsItem::isRotatable(QGraphicsItem *item)
{
    return item->data(UBGraphicsItemData::ItemRotatable).toBool();
}

bool UBGraphicsItem::isLocked(QGraphicsItem *item)
{
    return item->data(UBGraphicsItemData::ItemLocked).toBool();
}

QUuid UBGraphicsItem::getOwnUuid(QGraphicsItem *item)
{
    QString idCandidate = item->data(UBGraphicsItemData::ItemUuid).toString();
    return idCandidate == QUuid().toString() ? QUuid() : QUuid(idCandidate);
}

void UBGraphicsItem::remove(bool canUndo)
{
    if (Delegate())
        Delegate()->remove(canUndo);
}

UBGraphicsItemDelegate *UBGraphicsItem::Delegate(QGraphicsItem *pItem)
{
    UBGraphicsItemDelegate *result = 0;

    switch (static_cast<int>(pItem->type())) {
    case UBGraphicsPixmapItem::Type :
        result = (static_cast<UBGraphicsPixmapItem*>(pItem))->Delegate();
        break;
    case UBGraphicsTextItem::Type :
        result = (static_cast<UBGraphicsTextItem*>(pItem))->Delegate();
        break;
    case UBGraphicsSvgItem::Type :
        result = (static_cast<UBGraphicsSvgItem*>(pItem))->Delegate();
        break;
    case UBGraphicsMediaItem::Type:
    case UBGraphicsVideoItem::Type:
    case UBGraphicsAudioItem::Type:
        result = (static_cast<UBGraphicsMediaItem*>(pItem))->Delegate();
        break;
    case UBGraphicsStrokesGroup::Type :
        result = (static_cast<UBGraphicsStrokesGroup*>(pItem))->Delegate();
        break;
    case UBGraphicsGroupContainerItem::Type :
        result = (static_cast<UBGraphicsGroupContainerItem*>(pItem))->Delegate();
        break;
    case UBGraphicsWidgetItem::Type :
        result = (static_cast<UBGraphicsWidgetItem*>(pItem))->Delegate();
        break;
    case UBGraphicsCurtainItem::Type :
        result = (static_cast<UBGraphicsCurtainItem*>(pItem))->Delegate();
        break;
    }

    return result;
}
