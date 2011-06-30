/*
 * UBCoreGraphicsScene.cpp
 *
 *  Created on: 28 mai 2009
 *      Author: Luc
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
