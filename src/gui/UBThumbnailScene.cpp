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
#include "core/UBApplication.h"
#include "document/UBDocument.h"
#include "document/UBDocumentProxy.h"
#include "frameworks/UBBackgroundLoader.h"
#include "gui/UBThumbnail.h"
#include "gui/UBThumbnailArranger.h"
#include "gui/UBThumbnailsView.h"

UBThumbnailScene::UBThumbnailScene(UBDocument* document)
    : mDocument{document}
    , mThumbnailItems{document->proxy()->pageCount()}
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

void UBThumbnailScene::hightlightItem(int index, bool only)
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
    }

    if (index >= 0 && index < mThumbnailItems.size())
    {
        thumbnailAt(index)->setSelected(true);
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

            thumbnailItem->setPixmap(UBThumbnailAdaptor::get(mDocument->proxy(), index));
            thumbnailItem->setSceneIndex(index);

            mThumbnailItems[index] = thumbnailItem;
            addItem(thumbnailItem);

            arrangeThumbnails(index, index + 1);
        }

        return mThumbnailItems.at(index);
    }

    return nullptr;
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
    // skip already loaded thumbnails
    while (startIndex < mThumbnailItems.count() && mThumbnailItems.at(startIndex))
    {
        ++startIndex;
    }

    // create the list of all thumbnail paths
    QList<std::pair<int,QString>> paths;

    for (int index = startIndex; index < mDocument->proxy()->pageCount(); ++index)
    {
        paths << std::pair<int,QString>{index, UBThumbnailAdaptor::thumbnailUrl(mDocument->proxy(), index).toLocalFile()};
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

    mLoader = new UBBackgroundLoader{paths, this};
    mLoader->start();

    // now create all missing thumbnails for document as they arrive from the loader
    loadNextThumbnail();
}

/**
 * @brief Insert a thumbnail.
 *
 * Insert a thumbnail at the given index. This function assumes that the thumbnail pixmap files
 * are already moved to provide the necessary space. The pixmap for the new thumbnail can either
 * be provided in the corresponding pixmap file or it will be created from the scene.
 *
 * @param pageIndex Page index of the scene.
 * @param pageScene Optional scene. If this parameter is provided, then the tumbnail pixmap is
 * created from the scene. Otherwise it is assumed that the pixmap file already exists.
 */
void UBThumbnailScene::insertThumbnail(int pageIndex, std::shared_ptr<UBGraphicsScene> pageScene)
{
    if (pageScene)
    {
        UBThumbnailAdaptor::persistScene(mDocument->proxy(), pageScene, pageIndex);
    }

    if (pageIndex <= mThumbnailItems.size())
    {
        if (mThumbnailItems.size() == 1)
        {
            mThumbnailItems.first()->setDeletable(true);
        }

        auto thumbnailItem = new UBThumbnail;

        thumbnailItem->setPixmap(UBThumbnailAdaptor::get(mDocument->proxy(), pageIndex));
        thumbnailItem->setSceneIndex(pageIndex);

        mThumbnailItems.insert(pageIndex, thumbnailItem);
        addItem(thumbnailItem);

        renumberThumbnails(pageIndex);
        arrangeThumbnails(pageIndex);
    }

    if (mLoader)
    {
        // restart loading remaining thumbnails
        createThumbnails(pageIndex);
    }
}

void UBThumbnailScene::deleteThumbnail(int pageIndex, bool rearrange)
{
    if (pageIndex < mThumbnailItems.size())
    {
        auto thumbnail = mThumbnailItems.at(pageIndex);
        removeItem(thumbnail);
        delete thumbnail;
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
        createThumbnails(pageIndex);
    }

    if (mThumbnailItems.size() == 1)
    {
        mThumbnailItems.first()->setDeletable(false);
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
        createThumbnails(fromIndex);
    }
}

void UBThumbnailScene::reloadThumbnail(int pageIndex)
{
    if (pageIndex < mThumbnailItems.size())
    {
        auto thumbnail = mThumbnailItems.at(pageIndex);

        if (thumbnail && !thumbnail->isExposed())
        {
            thumbnail->setPixmap(UBThumbnailAdaptor::get(mDocument->proxy(), pageIndex));
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

void UBThumbnailScene::loadNextThumbnail()
{
    // max number of thumbnails to load in one pass
    constexpr int bulkSize{10};

    if (!mLoader->isIdle())
    {
        if (UBApplication::isClosing)
        {
            return;
        }

        if (!mLoader->isResultAvailable())
        {
            // no data available, defer next execution
            QTimer::singleShot(50, mLoader, [this]() { loadNextThumbnail(); });
            return;
        }

        int firstIndex = -1;

        for (int i = 0; i < bulkSize; ++i)
        {
            if (!mLoader->isResultAvailable())
            {
                break;
            }

            // take and process next result
            const auto result = mLoader->takeResult();
            const auto index = result.first;
            auto thumbnailItem = mThumbnailItems.at(index);

            if (!thumbnailItem)
            {
                QPixmap pixmap;

                if (result.second.isEmpty())
                {
                    pixmap = UBThumbnailAdaptor::generateMissingThumbnail(mDocument->proxy(), index);
                }
                else
                {
                    pixmap.loadFromData(result.second);
                }

                thumbnailItem = new UBThumbnail;

                thumbnailItem->setPixmap(pixmap);
                thumbnailItem->setSceneIndex(index);

                mThumbnailItems[index] = thumbnailItem;
                addItem(thumbnailItem);

                if (firstIndex < 0)
                {
                    firstIndex = index;
                }
            }
        }

        if (firstIndex >= 0)
        {
            arrangeThumbnails(firstIndex);
        }

        // load next thumbnails in a deferred task executed on the main thread when it is idle.
        QTimer::singleShot(1, mLoader, [this]() { loadNextThumbnail(); });
    }
    else
    {
        // finished. set undeletable if only one page
        if (mThumbnailItems.size() == 1)
        {
            mThumbnailItems.first()->setDeletable(false);
        }

        // delete background loader
        delete mLoader;
        mLoader = nullptr;
    }
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
        item->setSceneIndex(index);
    }
}
