/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBGraphicsItemUndoCommand.h"

#include <QtGui>

#include "UBGraphicsScene.h"

#include "core/UBApplication.h"

#include "board/UBBoardController.h"

#include "core/memcheck.h"
#include "domain/UBGraphicsGroupContainerItem.h"
#include "domain/UBGraphicsPolygonItem.h"

UBGraphicsItemUndoCommand::UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, const QSet<QGraphicsItem*>& pRemovedItems, const QSet<QGraphicsItem*>& pAddedItems, const GroupDataTable &groupsMap): UBUndoCommand()
    , mScene(pScene)
    , mRemovedItems(pRemovedItems - pAddedItems)
    , mAddedItems(pAddedItems - pRemovedItems)
    , mExcludedFromGroup(groupsMap)
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

UBGraphicsItemUndoCommand::UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, QGraphicsItem* pRemovedItem, QGraphicsItem* pAddedItem) : UBUndoCommand()
    , mScene(pScene)
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

        UBApplication::boardController->freezeW3CWidget(item, true);
        item->setSelected(false);

        QTransform t;
        bool bApplyTransform = false;
        UBGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<UBGraphicsPolygonItem*>(item);
        if (polygonItem){
            if (polygonItem->strokesGroup()
                    && polygonItem->strokesGroup()->parentItem()
                    && UBGraphicsGroupContainerItem::Type == polygonItem->strokesGroup()->parentItem()->type())
            {
                bApplyTransform = true;
                t = polygonItem->sceneTransform();
            }
            else if (polygonItem->strokesGroup())
                polygonItem->resetTransform();

            polygonItem->strokesGroup()->removeFromGroup(polygonItem);
        }
        mScene->removeItem(item);

        if (bApplyTransform)
            polygonItem->setTransform(t);

    }

    QSetIterator<QGraphicsItem*> itRemoved(mRemovedItems);
    while (itRemoved.hasNext())
    {
        QGraphicsItem* item = itRemoved.next();
        if (item)
        {
            if (itemLayerType::BackgroundItem == item->data(UBGraphicsItemData::itemLayerType))
                mScene->setAsBackgroundObject(item);
            else
                mScene->addItem(item);

            if (UBGraphicsPolygonItem::Type == item->type())
            {
                UBGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<UBGraphicsPolygonItem*>(item);
                if (polygonItem)
                {
                    mScene->removeItem(polygonItem);
                    mScene->removeItemFromDeletion(polygonItem);
                    polygonItem->strokesGroup()->addToGroup(polygonItem);
                }
            }

            UBApplication::boardController->freezeW3CWidget(item, false);
        }
    }

    GroupDataTableIterator curMapElement(mExcludedFromGroup);
    UBGraphicsGroupContainerItem *nextGroup = NULL;
    UBGraphicsGroupContainerItem *previousGroupItem = NULL;
    bool groupChanged = false;

    while (curMapElement.hasNext()) {
        curMapElement.next();

        groupChanged = previousGroupItem != curMapElement.key();
        //trying to find the group on the scene;
        if (!nextGroup || groupChanged) {
            UBGraphicsGroupContainerItem *groupCandidate = curMapElement.key();
            if (groupCandidate) {
                nextGroup = groupCandidate;
                if(!mScene->items().contains(nextGroup)) {
                    mScene->addItem(nextGroup);
                }
                nextGroup->setVisible(true);
            }
        }

        QGraphicsItem *groupedItem = mScene->itemForUuid(curMapElement.value());
        if (groupedItem) {
            nextGroup->addToGroup(groupedItem);
        }

        previousGroupItem = curMapElement.key();
        UBGraphicsItem::Delegate(nextGroup)->update();
    }

    // force refresh, QT is a bit lazy and take a lot of time (nb item ^2 ?) to trigger repaint
    mScene->update(mScene->sceneRect());
    mScene->updateSelectionFrame();

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

        GroupDataTableIterator curMapElement(mExcludedFromGroup);
        UBGraphicsGroupContainerItem *nextGroup = NULL;
        UBGraphicsGroupContainerItem *previousGroupItem = NULL;
        bool groupChanged = false;

        while (curMapElement.hasNext()) {
            curMapElement.next();

            groupChanged = previousGroupItem != curMapElement.key();
            //trying to find the group on the scene;
            if (!nextGroup || groupChanged) {
                UBGraphicsGroupContainerItem *groupCandidate = curMapElement.key();
                if (groupCandidate) {
                    nextGroup = groupCandidate;
                }
            }
            QGraphicsItem *groupedItem = mScene->itemForUuid(curMapElement.value());
            if (groupedItem) {
                if (nextGroup->childItems().count() == 1) {
                    nextGroup->destroy(false);
                    break;
                }
                nextGroup->removeFromGroup(groupedItem);
            }

            previousGroupItem = curMapElement.key();
            UBGraphicsItem::Delegate(nextGroup)->update();
        }

        QSetIterator<QGraphicsItem*> itRemoved(mRemovedItems);
        while (itRemoved.hasNext())
        {
            QGraphicsItem* item = itRemoved.next();
            item->setSelected(false);

            QTransform t;
            bool bApplyTransform = false;
            UBGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<UBGraphicsPolygonItem*>(item);

            if (polygonItem){
                if(polygonItem->strokesGroup()
                        && polygonItem->strokesGroup()->parentItem()
                        && UBGraphicsGroupContainerItem::Type == polygonItem->strokesGroup()->parentItem()->type())
                {
                    bApplyTransform = true;
                    t = polygonItem->sceneTransform();
                }
                else if (polygonItem->strokesGroup())
                    polygonItem->resetTransform();

                polygonItem->strokesGroup()->removeFromGroup(polygonItem);
            }

            if (itemLayerType::BackgroundItem == item->data(UBGraphicsItemData::itemLayerType))
                mScene->setAsBackgroundObject(nullptr);
            else
                mScene->removeItem(item);

            if (bApplyTransform)
                item->setTransform(t);

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

                UBGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<UBGraphicsPolygonItem*>(item);
                if (polygonItem)
                {
                    mScene->removeItem(polygonItem);
                    mScene->removeItemFromDeletion(polygonItem);
                    polygonItem->strokesGroup()->addToGroup(polygonItem);
                }
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
