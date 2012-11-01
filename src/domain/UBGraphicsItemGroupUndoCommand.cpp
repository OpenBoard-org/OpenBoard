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


#include "UBGraphicsItemGroupUndoCommand.h"

#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsScene.h"
#include "core/memcheck.h"


UBGraphicsItemGroupUndoCommand::UBGraphicsItemGroupUndoCommand(UBGraphicsScene *pScene, UBGraphicsGroupContainerItem *pGroupCreated) :
    mScene (pScene), mGroup(pGroupCreated), mFirstRedo(true)

{
    if (pGroupCreated->childItems().count()) {
        foreach (QGraphicsItem *item, pGroupCreated->childItems()) {
            mItems << item;
        }
    }
}

UBGraphicsItemGroupUndoCommand::~UBGraphicsItemGroupUndoCommand()
{
}

void UBGraphicsItemGroupUndoCommand::undo()
{
    mGroup->destroy(false);
    foreach(QGraphicsItem *item, mItems) {
        item->setSelected(true);
    }
}

void UBGraphicsItemGroupUndoCommand::redo()
{
    if (mFirstRedo) {
        //Work around. TODO determine why does Qt call the redo function on pushing to undo
        mFirstRedo = false;
        return;
    }

    foreach (QGraphicsItem *item, mItems) {
        if (item->type() == UBGraphicsGroupContainerItem::Type) {
            QList<QGraphicsItem*> childItems = item->childItems();
            UBGraphicsGroupContainerItem *currentGroup = dynamic_cast<UBGraphicsGroupContainerItem*>(item);
            if (currentGroup) {
                currentGroup->destroy(false);
            }
            foreach (QGraphicsItem *chItem, childItems) {
                mGroup->addToGroup(chItem);
            }
        } else {
            mGroup->addToGroup(item);
        }
    }

    mScene->addItem(mGroup);
    mGroup->setVisible(true);
    mGroup->setFocus();
    mGroup->setSelected(true);
}
