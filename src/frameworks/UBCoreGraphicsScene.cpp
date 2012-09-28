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
        qDebug() << "force delete is " << forceDelete;
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
