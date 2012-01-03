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
        mItemsToDelete.remove(item);
        delete item;
        return true;
    }
    else
        return false;
}

