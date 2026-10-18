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


#include "UBItemStyleUndoCommand.h"

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBStyledItem.h"
#include "gui/shapes/UBStylePalette.h"


UBItemStyleUndoCommand::UBItemStyleUndoCommand(std::shared_ptr<UBGraphicsScene> scene, QSet<UBStyledItem*> items, const UBItemStyle& prevStyle)
    : mScene{scene}
    , mItems{items}
    , mPrevStyle{prevStyle}
{
    for (const auto item : items)
    {
        mCurrStyle[item] = item->itemStyle();
    }
}

void UBItemStyleUndoCommand::undo()
{
    const auto isDark = mScene->isDarkBackground();

    for (const auto item : mItems)
    {
        item->applyItemStyle(mCurrStyle.value(item), isDark);
    }

    UBApplication::boardController->stylePalette()->updateSelection();
}

void UBItemStyleUndoCommand::redo()
{
    const auto isDark = mScene->isDarkBackground();

    for (const auto item : mItems)
    {
        item->applyItemStyle(mPrevStyle, isDark);
    }

    UBApplication::boardController->stylePalette()->updateSelection();
}
