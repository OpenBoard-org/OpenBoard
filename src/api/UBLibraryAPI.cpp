/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include "UBLibraryAPI.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#include "core/memcheck.h"

UBLibraryAPI::UBLibraryAPI(QWebView *pWebView)
    : QObject(pWebView)
    , mWebView(pWebView)
{
    connect(this, SIGNAL(downloadTriggered(const QUrl&, const QPointF&, const QSize&, bool)),
            UBApplication::boardController, SLOT(downloadURL(const QUrl&, const QPointF&, const QSize&, bool)),
            Qt::QueuedConnection);

}


UBLibraryAPI::~UBLibraryAPI()
{
    // NOOP
}


void UBLibraryAPI::addObject(QString pUrl, int width, int height, int x, int y, bool background)
{
    if (UBApplication::boardController)
        UBApplication::boardController->downloadURL(QUrl(pUrl), QString(), QPointF(x, y), QSize(width, height), background);

}


void UBLibraryAPI::addTool(QString pUrl, int x, int y)
{
    if (UBApplication::boardController)
    {
        emit downloadTriggered(pUrl, QPointF(x , y), QSize(), false);
    }
}


void UBLibraryAPI::startDrag(QString pUrl)
{
    QDrag *drag = new QDrag(mWebView);
    QMimeData *mimeData = new QMimeData;

    QList<QUrl> urls;
    urls << QUrl(pUrl);

    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction);

}

