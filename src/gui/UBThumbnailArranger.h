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


#pragma once

#include <QMargins>
#include <QObject>

// forward
class UBThumbnailsView;
class UBBoardThumbnailsView;
class UBDocumentThumbnailsView;

/**
 * @brief The UBThumbnailArranger class contains the specific properties of thumbnails
 * for different views (Board mode and Document mode).
 */
class UBThumbnailArranger
{
public:
    UBThumbnailArranger(UBThumbnailsView* thumbnailView);
    virtual ~UBThumbnailArranger() = default;

    UBThumbnailsView* thumbnailView() const;

    virtual int columnCount() const = 0;
    virtual double thumbnailWidth() const = 0;
    virtual double availableViewWidth() const;
    virtual QMarginsF margins() const;
    virtual QSizeF spacing() const;
    virtual bool isUIEnabled() const;

private:
    UBThumbnailsView* mThumbnailView{nullptr};
};
