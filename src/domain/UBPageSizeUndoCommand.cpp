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




#include "UBPageSizeUndoCommand.h"

#include <QtGui>

#include "board/UBBoardController.h"
#include "document/UBDocument.h"

#include "core/memcheck.h"

UBPageSizeUndoCommand::UBPageSizeUndoCommand(std::shared_ptr<UBDocument> pDocument, int pIndex, const QSize& previousSize, const QSize& newSize)
    : mDocument{pDocument}
    , mPageIndex{pIndex}
    , mPreviousSize(previousSize)
    , mNewSize(newSize)
{
    mFirstRedo = true;
}

UBPageSizeUndoCommand::~UBPageSizeUndoCommand()
{
   //NOOP
}

void UBPageSizeUndoCommand::undo()
{
    if (!mDocument){
        return;
    }

    mDocument->setPageSize(mPageIndex, mPreviousSize);
}

void UBPageSizeUndoCommand::redo()
{
    // the Undo framework calls a redo while appending the undo command.
    // as we have already plotted the elements, we do not want to do it twice
    if (!mFirstRedo)
    {
        if (!mDocument){
            return;
        }

        mDocument->setPageSize(mPageIndex, mNewSize);
    }
    else
    {
        mFirstRedo = false;
    }
}
