/*
 * UBDocumentUndoCommand.cpp
 *
 *  Created on: Sep 19, 2008
 *      Author: luc
 */

#include "UBDocumentUndoCommand.h"

#include "document/UBDocumentProxy.h"
#include "UBGraphicsScene.h"

#include "core/memcheck.h"

UBDocumentUndoCommand::UBDocumentUndoCommand(UBDocumentProxy* pDocument, const QList<UBGraphicsScene*>& pOldScenes,
        const QList<UBGraphicsScene*>& pNewScenes, const int& pActiveSceneIndex)
    : mDocument(pDocument)
    , mUndoScenes(pOldScenes)
    , mRedoScenes(pNewScenes)
    , mActiveSceneIndex(pActiveSceneIndex)
{
    // NOOP
}

UBDocumentUndoCommand::~UBDocumentUndoCommand()
{
    // NOOP
}

void UBDocumentUndoCommand::undo()
{
    // NOOP
}

void UBDocumentUndoCommand::redo()
{
    // NOOP
}

