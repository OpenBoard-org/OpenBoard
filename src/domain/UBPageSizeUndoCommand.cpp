/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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


#include "UBPageSizeUndoCommand.h"

#include <QProgressDialog>

#include "core/UBApplication.h"
#include "document/UBDocument.h"
#include "document/UBDocumentToc.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBMainWindow.h"
#include "gui/UBThumbnailScene.h"

#include "core/memcheck.h"


UBPageSizeUndoCommand::UBPageSizeUndoCommand(std::shared_ptr<UBDocument> document, QList<QUuid> pages, QSize newSize)
    : mDocument{document}
    , mPages{pages}
    , mNewSize(newSize)
{
}

UBPageSizeUndoCommand::~UBPageSizeUndoCommand()
{
   //NOOP
}

std::shared_ptr<UBDocument> UBPageSizeUndoCommand::document() const
{
    return mDocument;
}

void UBPageSizeUndoCommand::undo()
{
    if (!mDocument)
    {
        return;
    }

    QProgressDialog progress(QObject::tr("Changing page format..."), "", 0, mPages.size(), UBApplication::mainWindow);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setWindowFlag(Qt::FramelessWindowHint);
    progress.setCancelButton(nullptr);
    progress.setMinimumDuration(1000);

    for (int i = 0; i < mPages.size(); ++i)
    {
        const auto pageIndex = mDocument->toc()->findUuid(mPages[i]);

        if (pageIndex >= 0 && mPreviousSizes[i] != mDocument->pageSize(pageIndex))
        {
            mDocument->setPageSize(pageIndex, mPreviousSizes[i]);
        }

        progress.setValue(i);
    }

    progress.close();

    mDocument->thumbnailScene()->update();
}

void UBPageSizeUndoCommand::redo()
{
    if (!mDocument)
    {
        return;
    }

    QProgressDialog progress(QObject::tr("Changing page format..."), QObject::tr("Cancel"), 0, mPages.size(), UBApplication::mainWindow);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setWindowFlag(Qt::FramelessWindowHint);
    progress.setMinimumDuration(1000);

    for (int i = 0; i < mPages.size(); ++i)
    {
        const auto pageIndex = mDocument->toc()->findUuid(mPages[i]);

        if (pageIndex >= 0)
        {
            const auto currentSize = mDocument->pageSize(pageIndex);
            mPreviousSizes << currentSize;

            if (progress.wasCanceled())
            {
                continue;
            }
            else if (mNewSize.isNull())
            {
                const auto scene = mDocument->loadScene(pageIndex, false, false);
                const auto backgroundItem = scene->backgroundObject();

                if (backgroundItem)
                {
                    const auto itemSize = backgroundItem->sceneBoundingRect().size();
                    const QSize desiredPageSize{qCeil(itemSize.width()), qCeil(itemSize.height())};

                    if (desiredPageSize != currentSize)
                    {
                        mDocument->setPageSize(pageIndex, desiredPageSize, scene);
                    }
                }
            }
            else if (mNewSize != currentSize)
            {
                mDocument->setPageSize(pageIndex, mNewSize);
            }
        }

        progress.setValue(i);
    }

    progress.close();

    mDocument->thumbnailScene()->update();
}
