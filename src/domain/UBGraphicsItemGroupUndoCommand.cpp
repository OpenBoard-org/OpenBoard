/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBGraphicsItemGroupUndoCommand.h"

#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsScene.h"
#include "core/memcheck.h"


UBGraphicsItemGroupUndoCommand::UBGraphicsItemGroupUndoCommand(UBGraphicsScene *pScene, UBGraphicsGroupContainerItem *pGroupCreated) : UBUndoCommand()
  , mScene (pScene)
  , mGroup(pGroupCreated)
  , mFirstRedo(true)
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
