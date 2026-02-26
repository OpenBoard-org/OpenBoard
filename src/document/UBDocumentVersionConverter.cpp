/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBDocumentVersionConverter.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentToc.h"

#include "gui/UBMainWindow.h"

#include <QDebug>
#include <QHash>
#include <QStack>
#include <QVersionNumber>


constexpr int TMP_ID{99999};

UBDocumentVersionConverter::UBDocumentVersionConverter(std::shared_ptr<UBDocumentProxy> proxy)
    : mProxy(proxy)
{

}

UBDocumentVersionConverter::ConversionResult UBDocumentVersionConverter::convert() const
{
    // check validity of proxy
    if (!mProxy)
    {
        return SKIPPED;
    }

    // check document version
    QVersionNumber documentVersion = QVersionNumber::fromString(mProxy->metaData(UBSettings::documentVersion).toString());

    if (documentVersion < QVersionNumber{4, 9, 0})
    {
        // no conversion necessary
        return SKIPPED;
    }

    // ask user
    const auto accept = UBApplication::mainWindow->yesNoQuestion(
                tr("This document was created using OpenBoard 1.8"),
                tr("This document was created using OpenBoard 1.8, and its content has been reorganized. "
                   "We strongly recommend updating OpenBoard to version 1.8.\n\n"
                   "If you wish to continue, the document will be converted "
                   "to be compatible with version 1.7.7. This may take a few minutes. "
                   "You will still be able to open it in OpenBoard 1.8, "
                   "but it will then be converted again to the 1.8 format.\n\n"
                   "Do you want to continue?"));

    if (!accept)
    {
        return DENIED;
    }

    // load the TOC
    UBDocumentToc toc{mProxy->persistencePath()};

    if (!toc.load())
    {
        qDebug() << "unable to load TOC from" << mProxy->persistencePath();
        return FAILED;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    reorderPages(toc);
    copyAssets(toc);

    // update document version in metadata
    mProxy->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);
    UBMetadataDcSubsetAdaptor::persist(mProxy);

    // adjust page count
    mProxy->setPageCount(toc.pageCount());

    // delete the TOC
    QFile::remove(mProxy->persistencePath() + "/toc.json");

    QApplication::restoreOverrideCursor();

    return CONVERTED;
}

void UBDocumentVersionConverter::reorderPages(UBDocumentToc& toc) const
{
    // create a map for fast reverse lookup
    QHash<int, int> indexById;

    for (int index = 0; index < toc.pageCount(); ++index)
    {
        indexById[toc.pageId(index)] = index;
    }

    auto persistenceManager = UBPersistenceManager::persistenceManager();

    for (int index = 0; index < toc.pageCount(); ++index)
    {
        if (toc.pageId(index) == index)
        {
            // page id is already matching the index
            continue;
        }

        // build a stack of rename operations until we find a free name or a loop
        QStack<int> indexStack;
        int currentIndex = index;

        do
        {
            indexStack.push(currentIndex);

            if (!indexById.contains(currentIndex))
            {
                break;
            }

            currentIndex = indexById[currentIndex];
        } while (currentIndex != index);

        // if we found a loop, move the first page out of the way
        if (currentIndex == index)
        {
            persistenceManager->renamePage(mProxy, toc.pageId(index), TMP_ID);
            toc.setPageId(index, TMP_ID);
        }

        // now process the stack
        while (!indexStack.empty())
        {
            const auto ix = indexStack.pop();

            // rename toc.pageId(ix) to ix
            persistenceManager->renamePage(mProxy, toc.pageId(ix), ix);
            toc.setPageId(ix, ix);
        }
    }
}

void UBDocumentVersionConverter::copyAssets(const UBDocumentToc& toc) const
{
    // find the scenes with images and copy the asset files
    const auto prefix = UBPersistenceManager::imageDirectory + "/";

    for (int index = 0; index < toc.pageCount(); ++index)
    {
        const auto images = toc.assets(index).filter(prefix);

        if (!images.empty())
        {
            // load scene and find image items
            const auto scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(mProxy, index, false);
            const auto items = scene->items();

            for (const auto item : items)
            {
                if (item->type() == UBGraphicsItemType::SvgItemType || item->type() == UBGraphicsItemType::PixmapItemType)
                {
                    const auto ubitem = dynamic_cast<UBItem*>(item);

                    if (ubitem)
                    {
                        const auto itemUuid = ubitem->uuid();

                        // now copy the asset file
                        const auto asset = ubitem->sourceUrl().toLocalFile();
                        const auto documentPath = mProxy->persistencePath() + "/";

                        auto copiedAsset = asset;
                        const auto itemUuidString = itemUuid.toString();
                        copiedAsset.replace(prefix.size(), itemUuidString.size(), itemUuidString);

                        QFile::copy(documentPath + asset, documentPath + copiedAsset);
                    }
                }
            }
        }
    }
}
