/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include "UBGraphicsItemUndoCommand.h"

#include <QtGui>

#include "UBGraphicsScene.h"

#include "core/UBApplication.h"

#include "board/UBBoardController.h"

#include "core/memcheck.h"
#include "domain/UBGraphicsGroupContainerItem.h"

UBGraphicsItemUndoCommand::UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, const QSet<QGraphicsItem*>& pRemovedItems,
        const QSet<QGraphicsItem*>& pAddedItems)
    : mScene(pScene)
        , mRemovedItems(pRemovedItems - pAddedItems)
        , mAddedItems(pAddedItems - pRemovedItems)
{
    mFirstRedo = true;

    QSetIterator<QGraphicsItem*> itAdded(mAddedItems);
    while (itAdded.hasNext())
    {
        UBApplication::boardController->freezeW3CWidget(itAdded.next(), true);
    }

    QSetIterator<QGraphicsItem*> itRemoved(mRemovedItems);
    while (itRemoved.hasNext())
    {
        UBApplication::boardController->freezeW3CWidget(itRemoved.next(), false);
    }
}

UBGraphicsItemUndoCommand::UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, QGraphicsItem* pRemovedItem,
               QGraphicsItem* pAddedItem) :
    mScene(pScene)
{

    if (pRemovedItem)
    {
        mRemovedItems.insert(pRemovedItem);
    }

    if (pAddedItem)
    {
        mAddedItems.insert(pAddedItem);
    }

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

        //if removing group
        if (item->type() == UBGraphicsGroupContainerItem::Type) {
            UBGraphicsGroupContainerItem *curGroup = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(item);
            if (curGroup) {
                curGroup->destroy();
            }
        }

        UBApplication::boardController->freezeW3CWidget(item, true);
        item->setSelected(false);
        mScene->removeItem(item);
    }

    QSetIterator<QGraphicsItem*> itRemoved(mRemovedItems);
    while (itRemoved.hasNext())
    {
        QGraphicsItem* item = itRemoved.next();
        if (item)
        {
            if (UBItemLayerType::FixedBackground == item->data(UBGraphicsItemData::ItemLayerType))
                mScene->setAsBackgroundObject(item);
            else
                mScene->addItem(item);

            UBApplication::boardController->freezeW3CWidget(item, false);
        }
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
            UBApplication::boardController->freezeW3CWidget(item, true);
        }

        QSetIterator<QGraphicsItem*> itAdded(mAddedItems);
        while (itAdded.hasNext())
        {
            QGraphicsItem* item = itAdded.next();
            if (item)
            {
                UBApplication::boardController->freezeW3CWidget(item, false);

                if (UBItemLayerType::FixedBackground == item->data(UBGraphicsItemData::ItemLayerType))
                    mScene->setAsBackgroundObject(item);
                else
                    mScene->addItem(item);
            }
        }

        // force refresh, QT is a bit lazy and take a lot of time (nb item ^2) to trigger repaint
        mScene->update(mScene->sceneRect());
    }
    else
    {
        mFirstRedo = false;
    }
}
