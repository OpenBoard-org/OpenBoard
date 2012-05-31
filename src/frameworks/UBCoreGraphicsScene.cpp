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
    foreach (const QGraphicsItem* item, mItemsToDelete)
    {
        if (item->scene()==NULL || item->scene() == this)
        {
            delete item;
        }
    }
}

void UBCoreGraphicsScene::addItem(QGraphicsItem* item)
{
    if (item->type() == UBGraphicsGroupContainerItem::Type && item->childItems().count()) {
        foreach (QGraphicsItem *curItem, item->childItems()) {
            removeItemFromDeletion(curItem);
        }
    }

    mItemsToDelete << item;

    if (item->scene() != this)
        QGraphicsScene::addItem(item);
}


void UBCoreGraphicsScene::removeItem(QGraphicsItem* item, bool forceDelete)
{
    QGraphicsScene::removeItem(item);
    if (forceDelete)
    {
        mItemsToDelete.remove(item);
        delete item;
    }
}

bool UBCoreGraphicsScene::deleteItem(QGraphicsItem* item)
{
    if(mItemsToDelete.contains(item))
    {
        UBGraphicsItem* item_casted = 0;
        switch (item->type())
        {
        case UBGraphicsMediaItem::Type:
                item_casted = dynamic_cast<UBGraphicsMediaItem*>(item);
                break;
        case UBGraphicsW3CWidgetItem::Type:
                item_casted = dynamic_cast<UBGraphicsWidgetItem*>(item);
                break;
        }

        if (0 != item_casted)
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
