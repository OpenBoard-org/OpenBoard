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

#include "UBGraphicsTextItemUndoCommand.h"
#include "UBGraphicsTextItem.h"

#include "core/memcheck.h"

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
