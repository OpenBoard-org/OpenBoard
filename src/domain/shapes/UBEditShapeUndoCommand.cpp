/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#include "UBEditShapeUndoCommand.h"

#include "domain/shapes/UBAbstractGraphicsItem.h"

UBEditShapeUndoCommand::UBEditShapeUndoCommand(UBAbstractGraphicsItem* item)
    : mItem{item}
{
    // create a copy of the item
    mItemBefore = dynamic_cast<UBAbstractGraphicsItem*>(item->deepCopy());
}

UBEditShapeUndoCommand::~UBEditShapeUndoCommand()
{
    delete mItemBefore;
    delete mItemAfter;
}

void UBEditShapeUndoCommand::recordEditedItem()
{
    mItemAfter = dynamic_cast<UBAbstractGraphicsItem*>(mItem->deepCopy());
}

void UBEditShapeUndoCommand::undo()
{
    if (mItemBefore)
    {
        mItemBefore->copyItemParameters(mItem);
    }

    mItem->update();
}

void UBEditShapeUndoCommand::redo()
{
    if (mItemAfter)
    {
        mItemAfter->copyItemParameters(mItem);
    }

    mItem->update();
}
