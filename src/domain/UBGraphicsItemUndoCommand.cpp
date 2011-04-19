/*
 * UBGraphicsItemUndoCommand.cpp
 *
 *  Created on: Sep 16, 2008
 *      Author: luc
 */

#include "UBGraphicsItemUndoCommand.h"

#include <QtGui>

#include "UBGraphicsScene.h"


UBGraphicsItemUndoCommand::UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, const QSet<QGraphicsItem*>& pRemovedItems,
        const QSet<QGraphicsItem*>& pAddedItems)
    : mScene(pScene)
        , mRemovedItems(pRemovedItems - pAddedItems)
        , mAddedItems(pAddedItems - pRemovedItems)
{
    mFirstRedo = true;
}

UBGraphicsItemUndoCommand::UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, QGraphicsItem* pRemovedItem,
               QGraphicsItem* pAddedItem) :
    mScene(pScene)
{

    if (pRemovedItem)
        mRemovedItems.insert(pRemovedItem);

    if (pAddedItem)
        mAddedItems.insert(pAddedItem);

    mFirstRedo = true;

}

UBGraphicsItemUndoCommand::~UBGraphicsItemUndoCommand()
{
   //NOOP
}

void UBGraphicsItemUndoCommand::undo()
{
    if (!mScene){
        return;
    }

    QSetIterator<QGraphicsItem*> itAdded(mAddedItems);
    while (itAdded.hasNext())
    {
        QGraphicsItem* item = itAdded.next();
        item->setSelected(false);
        mScene->removeItem(item);
    }

    QSetIterator<QGraphicsItem*> itRemoved(mRemovedItems);
    while (itRemoved.hasNext())
    {
        mScene->addItem(itRemoved.next());
    }

    // force refresh, QT is a bit lazy and take a lot of time (nb item ^2 ?) to trigger repaint
    mScene->update(mScene->sceneRect());

}

void UBGraphicsItemUndoCommand::redo()
{
    // the Undo framework calls a redo while appending the undo command.
    // as we have already plotted the elements, we do not want to do it twice
    if (!mFirstRedo)
    {
        if (!mScene){
            return;
        }

        QSetIterator<QGraphicsItem*> itRemoved(mRemovedItems);
        while (itRemoved.hasNext())
        {
            QGraphicsItem* item = itRemoved.next();
            item->setSelected(false);
            mScene->removeItem(item);
        }

        QSetIterator<QGraphicsItem*> itAdded(mAddedItems);
        while (itAdded.hasNext())
        {
            mScene->addItem(itAdded.next());
        }

        // force refresh, QT is a bit lazy and take a lot of time (nb item ^2) to trigger repaint
        mScene->update(mScene->sceneRect());
    }
    else
    {
        mFirstRedo = false;
    }
}
