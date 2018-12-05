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




#include "UBCoreGraphicsScene.h"

#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsGroupContainerItem.h"

#include "core/memcheck.h"

UBCoreGraphicsScene::UBCoreGraphicsScene(QObject * parent)
    : QGraphicsScene ( parent  )
    , mIsModified(true)
{
    //NOOP
}

UBCoreGraphicsScene::~UBCoreGraphicsScene()
{
    //we must delete removed items that are no more in any scene
    //at groups deleting some items can be added to mItemsToDelete, so we need to use iterators.
    foreach(QGraphicsItem* item, mItemsToDelete)
    {
        if (item)
        {
            if (item->scene() == NULL || item->scene() == this)
            {
                delete item;
                item = NULL;
            }
        }
    }
    mItemsToDelete.clear();
}

void UBCoreGraphicsScene::addItem(QGraphicsItem* item)
{
    addItemToDeletion(item);

    if (item->type() == UBGraphicsGroupContainerItem::Type && item->childItems().count()) {
        foreach (QGraphicsItem *curItem, item->childItems()) {
            removeItemFromDeletion(curItem);
        }
    }

    if (item->scene() != this)
        QGraphicsScene::addItem(item);

    setModified(true);
}


void UBCoreGraphicsScene::removeItem(QGraphicsItem* item, bool forceDelete)
{
    QGraphicsScene::removeItem(item);
    if (forceDelete)
    {
        deleteItem(item);
    }
    setModified(true);
}

bool UBCoreGraphicsScene::deleteItem(QGraphicsItem* item)
{
    if(mItemsToDelete.contains(item))
    {
        UBGraphicsItem *item_casted = dynamic_cast<UBGraphicsItem *>(item);
        if (item_casted != NULL)
            item_casted->clearSource();

        mItemsToDelete.remove(item);
        delete item;
        item = NULL;
        return true;
    }
    else
        return false;
}

void UBCoreGraphicsScene::removeItemFromDeletion(QGraphicsItem *item)
{
    if(NULL != item){
        mItemsToDelete.remove(item);
    }
}

void UBCoreGraphicsScene::addItemToDeletion(QGraphicsItem *item)
{
    if (item) {
        mItemsToDelete.insert(item);
    }
}
