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


#include "UBThumbnailScene.h"

#include "adaptors/UBThumbnailAdaptor.h"
#include "document/UBDocument.h"
#include "frameworks/UBBackgroundLoader.h"
#include "gui/UBThumbnail.h"
#include "gui/UBThumbnailArranger.h"
#include "gui/UBThumbnailsView.h"

UBThumbnailScene::UBThumbnailScene(UBDocument* document)
    : mDocument{document}
    , mThumbnailItems{}
{
}

UBDocument* UBThumbnailScene::document()
{
    return mDocument;
}

void UBThumbnailScene::arrangeThumbnails(int fromIndex, int toIndex)
{
    auto thumbnailArranger = currentThumbnailArranger();

    if (!thumbnailArranger)
    {
        return;
    }

    if (toIndex < 0 || toIndex > mThumbnailItems.size())
    {
        toIndex = mThumbnailItems.size();
    }

    if (toIndex <= fromIndex || fromIndex >= mThumbnailItems.size())
    {
        // nothing to do
        return;
    }

    // arrange thumbnails in rows and columns
    const auto thumbnailWidth = thumbnailArranger->thumbnailWidth();
    const auto availableViewWidth = thumbnailArranger->availableViewWidth();

    const int nbColumns = thumbnailArranger->columnCount();
    const int nbRows = ((mThumbnailItems.size() - 1) / nbColumns) + 1;

    const auto thumbnailHeight = UBThumbnail::heightForWidth(thumbnailWidth);
    const QSizeF thumbnailSize{thumbnailWidth, thumbnailHeight};
    auto columnIndex = fromIndex % nbColumns;
    auto rowIndex = fromIndex / nbColumns;

    const auto margins = thumbnailArranger->margins();
    const auto spacing = thumbnailArranger->spacing();
    QSizeF gridSize{thumbnailWidth + spacing.width(), thumbnailHeight + spacing.height()};
    const auto horizontalCenterOffset =
        nbColumns == 1 ? (availableViewWidth - gridSize.width() + spacing.width()) / 2 : 0.;

    for (int i = fromIndex; i < toIndex; ++i)
    {
        auto thumbnail = mThumbnailItems.at(i);

        if (thumbnail)
        {
            thumbnail->setThumbnailSize(thumbnailSize);
            thumbnail->setColumn(columnIndex);
            thumbnail->setRow(rowIndex);

            // start with a margin left
            // step by width + spacing
            // start with margin top
            // step by height including label + spacing
            const auto posX = margins.left() + columnIndex * gridSize.width() + horizontalCenterOffset;
            const auto posY = margins.top() + rowIndex * gridSize.height();

            thumbnail->setPos(posX, posY);
        }

        if (++columnIndex >= nbColumns)
        {
            ++rowIndex;
            columnIndex = 0;
        }
    }

    auto view = thumbnailArranger->thumbnailView();

    // adjust the viewport of the view
    const QSizeF sceneSize{availableViewWidth, nbRows * thumbnailHeight + (nbRows - 1) * spacing.height()};
    const QRectF sceneRect{{}, sceneSize.grownBy(margins)};

    view->setSceneRect(sceneRect);
}

void UBThumbnailScene::hightlightItem(int index, bool only, bool selected)
{
    if (only)
    {
        for (auto item : selectedItems())
        {
            auto thumbnail = dynamic_cast<UBThumbnail*>(item);

            if (thumbnail && thumbnail->sceneIndex() != index)
            {
                thumbnail->setSelected(false);
            }
        }

        mLastSelectedThumbnail = nullptr;
    }

    if (index >= 0 && index < mThumbnailItems.size())
    {
        auto thumbnail = thumbnailAt(index);
        thumbnail->setSelected(selected);

        mLastSelectedThumbnail = selected ? thumbnail : nullptr;
    }
}

int UBThumbnailScene::thumbnailCount() const
{
    return mThumbnailItems.size();
}

UBThumbnail* UBThumbnailScene::thumbnailAt(int index)
{
    if (index >= 0 && index < mThumbnailItems.size())
    {
        if (!mThumbnailItems.at(index))
        {
            // create the missing thumbnail
            auto thumbnailItem = new UBThumbnail;

            thumbnailItem->setPixmap(UBThumbnailAdaptor::get(mDocument, index));
            thumbnailItem->setSceneIndex(index);

            mThumbnailItems[index] = thumbnailItem;
            addItem(thumbnailItem);

            arrangeThumbnails(index, index + 1);
        }

        return mThumbnailItems.at(index);
    }

    return nullptr;
}

UBThumbnail* UBThumbnailScene::lastSelectedThumbnail() const
{
    return mLastSelectedThumbnail;
}

/**
 * @brief Create thumbnails for this scene.
 *
 * Thumbnails for the document pages above startIndex are asynchronously
 * loaded and positioned on the scene. The application remains responsive
 * even while loading thumbnails.
 *
 * It is even possible to interact with the already loaded thumbnails during
 * the loading process. So the already loaded pages can be moved, copied,
 * deleted or opened in Board mode while loading of the remaining thumbnails
 * continues in background.
 */
void UBThumbnailScene::createThumbnails(int startIndex)
{
    // adjust number of thumbnail items and create empty thumbnails in new items
    for (int index =  mThumbnailItems.count(); index < mDocument->pageCount(); ++index)
    {
        insertThumbnail(index, false);
    }

    // skip already loaded thumbnails
    while (startIndex < mThumbnailItems.count() && mThumbnailItems.at(startIndex)
           && mThumbnailItems.at(startIndex)->isLoaded())
    {
        ++startIndex;
    }

    // create the list of all thumbnail paths
    QList<std::pair<int,QString>> paths;

    for (int index = startIndex; index < mDocument->pageCount(); ++index)
    {
        const auto thumbnailPath = UBThumbnailAdaptor::thumbnailUrl(mDocument, index).toLocalFile();
        paths << std::pair<int,QString>{index, thumbnailPath};
    }

    // start background loading of files
    if (mLoader)
    {
        // abort a running loader
        mLoader->abort();
        delete mLoader;
        mLoader = nullptr;
    }

    if (paths.empty())
    {
        return;
    }

    mLoader = new UBBackgroundLoader{UBBackgroundLoader::Pixmap, this};

    connect(mLoader, &UBBackgroundLoader::resultAvailable, this, [this](int index, const QVariant& data){
        // now generate all missing thumbnails for document as they arrive from the loader
        loadThumbnail(index, data);

        if (mLoader)
        {
            // delay next thumbnail processing  after a specified number of thumbnails
            // to improve user experience when simultaneously interacting with the board
            if (index % 10 == 0)
            {
                QTimer::singleShot(5, this, [this](){
                    mLoader->resultProcessed();
                });
            }
            else
            {
                mLoader->resultProcessed();
            }
        }
    });

    connect(mLoader, &UBBackgroundLoader::finished, this, [this](){
        // delete loader
        mLoader->deleteLater();
        mLoader = nullptr;
    });

    mLoader->load(paths);
}

/**
 * @brief Insert a thumbnail.
 *
 * Insert a thumbnail at the given index.
 *
 * @param pageIndex Page index of the scene.
 * @param loadThumbnail If true, load the thumbnail from the file (default).
 * If false, only an empty thumbnail is created which will be filled later.
 */
void UBThumbnailScene::insertThumbnail(int pageIndex, bool loadThumbnail)
{
    if (pageIndex <= mThumbnailItems.size())
    {
        if (mThumbnailItems.size() == 1)
        {
            thumbnailAt(0)->setDeletable(true);
        }

        auto thumbnailItem = new UBThumbnail;

        if (loadThumbnail)
        {
            const auto pixmap = UBThumbnailAdaptor::get(mDocument, pageIndex);
            thumbnailItem->setPixmap(pixmap);
        }

        thumbnailItem->setSceneIndex(pageIndex);

        mThumbnailItems.insert(pageIndex, thumbnailItem);
        addItem(thumbnailItem);

        renumberThumbnails(pageIndex);
        arrangeThumbnails(pageIndex);
    }

    if (mLoader && loadThumbnail)
    {
        // restart loading remaining thumbnails
        createThumbnails();
    }
}

void UBThumbnailScene::deleteThumbnail(int pageIndex, bool rearrange)
{
    if (pageIndex < mThumbnailItems.size())
    {
        auto thumbnail = mThumbnailItems.at(pageIndex);

        if (thumbnail)
        {
            if (thumbnail == mLastSelectedThumbnail)
            {
                mLastSelectedThumbnail = nullptr;
            }

            removeItem(thumbnail);
            delete thumbnail;
        }

        mThumbnailItems.removeAt(pageIndex);

        if (rearrange)
        {
            renumberThumbnails(pageIndex);
            arrangeThumbnails(pageIndex);
        }
    }

    if (mLoader)
    {
        // restart loading remaining thumbnails
        createThumbnails();
    }

    if (mThumbnailItems.size() == 1)
    {
        thumbnailAt(0)->setDeletable(false);
    }
}

void UBThumbnailScene::moveThumbnail(int fromIndex, int toIndex)
{
    if (fromIndex < mThumbnailItems.size() && toIndex < mThumbnailItems.size())
    {
        auto thumbnail = mThumbnailItems.at(fromIndex);
        mThumbnailItems.removeAt(fromIndex);
        mThumbnailItems.insert(toIndex, thumbnail);

        if (toIndex < fromIndex)
        {
            std::swap(fromIndex, toIndex);
        }

        renumberThumbnails(fromIndex, toIndex + 1);
        arrangeThumbnails(fromIndex, toIndex + 1);
    }

    if (mLoader)
    {
        // restart loading remaining thumbnails
        createThumbnails();
    }
}

void UBThumbnailScene::reloadThumbnail(int pageIndex)
{
    if (pageIndex < mThumbnailItems.size())
    {
        auto thumbnail = mThumbnailItems.at(pageIndex);

        if (thumbnail)
        {
            thumbnail->setPixmap(UBThumbnailAdaptor::get(mDocument, pageIndex));
            arrangeThumbnails(pageIndex, pageIndex + 1);
        }
    }
}

void UBThumbnailScene::ensureThumbnail(int pageIndex, UBGraphicsScene* scene)
{
    if (pageIndex < mThumbnailItems.size())
    {
        auto thumbnailItem = mThumbnailItems.at(pageIndex);

        if (!thumbnailItem || !thumbnailItem->isLoaded())
        {
            QPixmap pixmap = UBThumbnailAdaptor::generateMissingThumbnail(mDocument, pageIndex, scene);

            if (!thumbnailItem)
            {
                thumbnailItem = new UBThumbnail;
            }

            thumbnailItem->setPixmap(pixmap);
            thumbnailItem->setSceneIndex(pageIndex);

            if (!mThumbnailItems.at(pageIndex))
            {
                mThumbnailItems[pageIndex] = thumbnailItem;
                addItem(thumbnailItem);
            }

            arrangeThumbnails(pageIndex, pageIndex + 1);
        }
    }
}

UBThumbnailArranger* UBThumbnailScene::currentThumbnailArranger()
{
    UBThumbnailArranger* thumbnailArranger{nullptr};

    // get arranger from view
    for (const auto view : views())
    {
        const auto thumbnailsView = dynamic_cast<UBThumbnailsView*>(view);

        if (thumbnailsView)
        {
            auto arranger = thumbnailsView->thumbnailArranger();

            if (arranger && view->isVisible())
            {
                thumbnailArranger = arranger;
                break;
            }
        }
    }

    return thumbnailArranger;
}

void UBThumbnailScene::loadThumbnail(int index, const QVariant& data)
{
    auto thumbnailItem = mThumbnailItems.at(index);

    if (!thumbnailItem)
    {
        thumbnailItem = new UBThumbnail;
        mThumbnailItems[index] = thumbnailItem;
        addItem(thumbnailItem);
    }

    if (!data.isNull())
    {
        QPixmap pixmap = data.value<QPixmap>();
        thumbnailItem->setPixmap(pixmap);
    }

    thumbnailItem->setSceneIndex(index);

    arrangeThumbnails(index, index + 1);
}

void UBThumbnailScene::renumberThumbnails(int fromIndex, int toIndex) const
{
    if (toIndex < 0 || toIndex > mThumbnailItems.size())
    {
        toIndex = mThumbnailItems.size();
    }

    if (toIndex <= fromIndex || fromIndex >= mThumbnailItems.size())
    {
        // nothing to do
        return;
    }

    // fix index and page number of thumbnails
    for (int index = fromIndex; index < toIndex; ++index)
    {
        auto item = mThumbnailItems.at(index);

        if (item)
        {
            item->setSceneIndex(index);
        }
    }
}
