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


#include "UBAbstractEditable.h"

#include "core/UBApplication.h"

#include "domain/shapes/UBAbstractGraphicsItem.h"
#include "domain/shapes/UBEditShapeUndoCommand.h"


UBAbstractEditable::UBAbstractEditable()
{
}

UBAbstractEditable::~UBAbstractEditable()
{
}

void UBAbstractEditable::addHandle(UBAbstractHandle* handle)
{
    mHandles.push_back(handle);
}

UBAbstractHandle* UBAbstractEditable::getHandle(HandleId id) const
{
    for (auto handle : mHandles)
    {
        if (handle->getId() == id)
        {
            return handle;
        }
    }

    return nullptr;
}

void UBAbstractEditable::showEditMode(bool show)
{
    if (!show)
    {
        for (int i = 0; i < mHandles.size(); i++)
        {
            mHandles.at(i)->hide();
        }
    }
    else
    {
        for (int i = 0; i < mHandles.size(); i++)
        {
            mHandles.at(i)->show();
        }
    }

    if (show && !mEditMode)
    {
        // reset and create undo command on transition to edit mode
        mModified = false;
        mUndoCommand = new UBEditShapeUndoCommand{dynamic_cast<UBAbstractGraphicsItem*>(this)};
    }
    else if (!show && mEditMode)
    {
        if (mModified)
        {
            // commit undo command when shape was modified during editing
            mUndoCommand->recordEditedItem();
            UBApplication::undoStack->push(mUndoCommand);
        }
        else
        {
            // delete undo command
            delete mUndoCommand;
        }

        mUndoCommand = nullptr;
    }

    mEditMode = show;
}

void UBAbstractEditable::deactivateEditionMode()
{
    // nop
}

bool UBAbstractEditable::isInEditMode() const
{
    return mEditMode;
}

void UBAbstractEditable::setModified()
{
    mModified = true;
}

void UBAbstractEditable::addHandleShapes(const QGraphicsItem* item, QPainterPath& path) const
{
    for (const auto handle: mHandles)
    {
        path = path.united(item->mapFromItem(handle, handle->shape()));
    }
}
