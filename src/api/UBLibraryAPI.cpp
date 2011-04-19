/*
 * UBLibraryAPI.cpp
 *
 *  Created on: 9 juil. 2009
 *      Author: Luc
 */

#include "UBLibraryAPI.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"

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
        UBApplication::boardController->downloadURL(QUrl(pUrl), QPointF(x, y), QSize(width, height), background);

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

