/*
 * UBGraphicsTextItemUndoCommand.cpp
 *
 *  Created on: July 7, 2009
 *      Author: Jerome
 */

#include "UBGraphicsTextItemUndoCommand.h"
#include "UBGraphicsTextItem.h"


UBGraphicsTextItemUndoCommand::UBGraphicsTextItemUndoCommand(UBGraphicsTextItem *textItem)
    : mTextItem(textItem)
{
    // NOOP
}

UBGraphicsTextItemUndoCommand::~UBGraphicsTextItemUndoCommand()
{
    // NOOP
}

void UBGraphicsTextItemUndoCommand::undo()
{
    if(mTextItem && mTextItem->document())
        mTextItem->document()->undo();
}

void UBGraphicsTextItemUndoCommand::redo()
{
    if(mTextItem && mTextItem->document())
        mTextItem->document()->redo();
}
