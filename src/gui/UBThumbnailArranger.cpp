/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBThumbnailArranger.h"

#include <QScrollBar>

#include "core/UBSettings.h"
#include "gui/UBThumbnailsView.h"


UBThumbnailArranger::UBThumbnailArranger(UBThumbnailsView* thumbnailView)
    : mThumbnailView(thumbnailView)
{
}

UBThumbnailsView* UBThumbnailArranger::thumbnailView() const
{
    return mThumbnailView;
}

double UBThumbnailArranger::availableViewWidth() const
{
    const QScrollBar* vertScrollBar = thumbnailView()->verticalScrollBar();
    int scrollBarThickness{0};

    if (vertScrollBar && vertScrollBar->isVisible())
    {
        scrollBarThickness = vertScrollBar->width();
    }

    return thumbnailView()->width() - margins().left() - margins().right() - scrollBarThickness;
}

QMarginsF UBThumbnailArranger::margins() const
{
    const double margin = UBSettings::thumbnailSpacing;
    return {margin, margin, margin, margin};
}

QSizeF UBThumbnailArranger::spacing() const
{
    const double spacing = UBSettings::thumbnailSpacing;
    return {spacing, spacing};
}

bool UBThumbnailArranger::isUIEnabled() const
{
    return true;
}
