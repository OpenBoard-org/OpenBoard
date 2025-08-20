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


#include <QGraphicsScene>

#include "core/UBSettings.h"

// forward
class UBBackgroundLoader;
class UBDocument;
class UBGraphicsScene;
class UBThumbnail;
class UBThumbnailArranger;

/**
 * @brief The UBThumbnailScene class is the graphics scene where the thumbnail items
 * for a specific document are placed.
 *
 * The scene is created, owned, and destroyed, and it can be accessed by the UBDocument.
 * The scene also arranges the thumbnails in rows and columns. Depending on the usage for
 * Board or Document mode, a related UBThumbnailArranger is used to provide layout
 * parameters to the scene.
 */
class UBThumbnailScene : public QGraphicsScene
{
    Q_OBJECT

public:
    UBThumbnailScene(UBDocument* document);

    UBDocument* document();

    // thumbnail management
    void createThumbnails(int startIndex = 0);
    void arrangeThumbnails(int fromIndex = 0, int toIndex = -1);
    void hightlightItem(int index, bool only = false, bool selected = true);
    int thumbnailCount() const;
    UBThumbnail* thumbnailAt(int index);
    UBThumbnail* lastSelectedThumbnail() const;

protected:
    // only to be called from UBDocument
    friend class UBDocument;
    void insertThumbnail(int pageIndex, bool loadThumbnail = true);
    void deleteThumbnail(int pageIndex, bool rearrange = true);
    void moveThumbnail(int fromIndex, int toIndex);
    void reloadThumbnail(int pageIndex);

private:
    friend class UBThumbnail;
    UBThumbnailArranger* currentThumbnailArranger();
    void loadThumbnail(int index, const QByteArray& data);
    void renumberThumbnails(int fromIndex = 0, int toIndex = -1) const;

private:
    UBDocument* mDocument{nullptr};
    QVector<UBThumbnail*> mThumbnailItems{};
    int mThumbnailWidth{UBSettings::defaultThumbnailWidth};
    UBBackgroundLoader* mLoader{nullptr};
    UBThumbnail* mLastSelectedThumbnail{nullptr};
};
