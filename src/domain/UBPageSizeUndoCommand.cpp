/*
 * UBPageSizeUndoCommand.cpp
 *
 *  Created on: Jul 17, 2009
 *      Author: Patrick
 */

#include "UBPageSizeUndoCommand.h"

#include <QtGui>

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "UBGraphicsScene.h"


UBPageSizeUndoCommand::UBPageSizeUndoCommand(UBGraphicsScene* pScene, const QSize& previousSize, const QSize& newSize)
    : mScene(pScene)
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
    if (!mScene){
        return;
    }

    UBApplication::boardController->setPageSize(mPreviousSize);
    // force refresh, QT is a bit lazy and take a lot of time (nb item ^2 ?) to trigger repaint
    mScene->update(mScene->sceneRect());

}

void UBPageSizeUndoCommand::redo()
{
    // the Undo framework calls a redo while appending the undo command.
    // as we have already plotted the elements, we do not want to do it twice
    if (!mFirstRedo)
    {
        if (!mScene){
            return;
        }

        UBApplication::boardController->setPageSize(mNewSize);
        // force refresh, QT is a bit lazy and take a lot of time (nb item ^2) to trigger repaint
        mScene->update(mScene->sceneRect());
    }
    else
    {
        mFirstRedo = false;
    }
}
