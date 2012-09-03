/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include "UBCoreGraphicsScene.h"

#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsGroupContainerItem.h"

#include "core/memcheck.h"

UBCoreGraphicsScene::UBCoreGraphicsScene(QObject * parent)
    : QGraphicsScene ( parent  )
{
    //NOOP
}

UBCoreGraphicsScene::~UBCoreGraphicsScene()
{
    //we must delete removed items that are no more in any scene
    //at groups deleting some items can be added to mItemsToDelete, so we need to use iterators.
    if (mItemsToDelete.count())
    {
        QSet<QGraphicsItem *>::iterator it = mItemsToDelete.begin();
        QGraphicsItem* item = *it;
        do 
        {
            item = *it;
            if (item && (item->scene() == NULL || item->scene() == this))
            {
                mItemsToDelete.remove(*it);
                delete item;
            }

            it = mItemsToDelete.begin();

        }while(mItemsToDelete.count());
    }
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
}


void UBCoreGraphicsScene::removeItem(QGraphicsItem* item, bool forceDelete)
{
    QGraphicsScene::removeItem(item);
    if (forceDelete)
    {
        deleteItem(item);
    }
}

bool UBCoreGraphicsScene::deleteItem(QGraphicsItem* item)
{
    if(mItemsToDelete.contains(item))
    {
        UBGraphicsItem *item_casted = dynamic_cast<UBGraphicsItem *>(item);
        if (0 != item_casted)
            item_casted->clearSource();

        mItemsToDelete.remove(item);
        delete item;
        item = 0;
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
