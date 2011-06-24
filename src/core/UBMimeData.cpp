/*
 * UBMimeData.cpp
 *
 *  Created on: Dec 8, 2008
 *      Author: Luc
 */

#include "UBMimeData.h"

#include <QtGui>

#include "core/UBApplication.h"
#include "domain/UBItem.h"

#include "core/memcheck.h"

UBMimeDataItem::UBMimeDataItem(UBDocumentProxy* proxy, int sceneIndex)
    : mProxy(proxy)
    , mSceneIndex(sceneIndex)
{
    // NOOP
}

UBMimeDataItem::~UBMimeDataItem()
{
    // NOOP
}

UBMimeData::UBMimeData(const QList<UBMimeDataItem> &items)
    : QMimeData()
    , mItems(items)
{
    setData(UBApplication::mimeTypeUniboardPage, QByteArray());
}

UBMimeData::~UBMimeData()
{
    // NOOP
}

UBMimeDataGraphicsItem::UBMimeDataGraphicsItem(QList<UBItem*> pItems)
{
        mItems = pItems;
}

UBMimeDataGraphicsItem::~UBMimeDataGraphicsItem()
{
        foreach(UBItem* item, mItems)
            delete item;
}
