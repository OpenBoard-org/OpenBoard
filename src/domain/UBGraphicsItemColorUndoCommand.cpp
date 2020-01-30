/*
 * Copyright (C) 2015-2020 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBGraphicsItemColorUndoCommand.h"

UBGraphicsItemColorUndoCommand::UBGraphicsItemColorUndoCommand(UBGraphicsScene* _scene, const ColorCommandsVector& pCmds)
: mColorCommands(pCmds)
{
    Q_ASSERT(_scene != NULL);
    mpScene = _scene;
    mHack = false;

}

UBGraphicsItemColorUndoCommand::~UBGraphicsItemColorUndoCommand()
{

}

void UBGraphicsItemColorUndoCommand::undo()
{
    qDebug() << "mColorCommands.size: " << mColorCommands.size() << endl;

    if (!mpScene || mColorCommands.size() == 0)
        return;

    for (auto cc : mColorCommands)
    {
        cc->setPreviousColor();
    }
    updateLazyScene();
}

void UBGraphicsItemColorUndoCommand::redo()
{
    if (!mHack)
    {
        // Ugly! But pushing a new command to QUndoStack calls redo by itself.
        mHack = true;
        qDebug() << "hack gesetzt" << endl;
    }
    else
    {
        if (!mpScene)
            return;
        qDebug() << "mColorCommands.size: " << mColorCommands.size() << endl;

        for (auto cc : mColorCommands)
        {
            cc->setColor();
        }
        updateLazyScene();
    }
}

void UBGraphicsItemColorUndoCommand::updateLazyScene()
{
    mpScene->update(mpScene->sceneRect());
    mpScene->updateSelectionFrame();
}
