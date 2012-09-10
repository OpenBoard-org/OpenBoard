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

#include "UBGraphicsItemUndoCommand.h"

#include <QtGui>

#include "UBGraphicsScene.h"

#include "core/UBApplication.h"

#include "board/UBBoardController.h"

#include "core/memcheck.h"
#include "domain/UBGraphicsGroupContainerItem.h"

UBGraphicsItemUndoCommand::UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, const QSet<QGraphicsItem*>& pRemovedItems,
                                                     const QSet<QGraphicsItem*>& pAddedItems, const GroupDataTable &groupsMap)
    : mScene(pScene)
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

    QMapIterator<UBGraphicsGroupContainerItem*, QUuid> curMapElement(mExcludedFromGroup);
    UBGraphicsGroupContainerItem *nextGroup = 0;
    UBGraphicsGroupContainerItem *previousGroupItem;
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

        QMapIterator<UBGraphicsGroupContainerItem*, QUuid> curMapElement(mExcludedFromGroup);
        UBGraphicsGroupContainerItem *nextGroup = 0;
        UBGraphicsGroupContainerItem *previousGroupItem;
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
