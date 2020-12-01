/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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




/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "WBBrowserWindow.h"
#include "WBDownloadManager.h"
#include "WBTabWidget.h"
#include "WBWebView.h"
#include "web/UBWebPage.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "board/UBBoardController.h"
#include "core/UBSettings.h"

#include "network/UBNetworkAccessManager.h"
#include "network/UBCookieJar.h"

#include <QtGui>
#include <QtUiTools/QUiLoader>
#include <QMessageBox>
#include <QWebFrame>

#include "core/memcheck.h"

WBWebPage::WBWebPage(QObject *parent)
    : UBWebPage(parent)
    , mKeyboardModifiers(Qt::NoModifier)
    , mPressedButtons(Qt::NoButton)
    , mOpenInNewTab(false)
{
    setNetworkAccessManager(UBNetworkAccessManager::defaultAccessManager());

    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)),
            this, SLOT(handleUnsupportedContent(QNetworkReply *)));
}

WBBrowserWindow *WBWebPage::mainWindow()
{
    QObject *w = this->parent();
    while (w)
    {
        if (WBBrowserWindow *mw = qobject_cast<WBBrowserWindow*>(w))
            return mw;

        w = w->parent();
    }

        return 0;
}

bool WBWebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    // ctrl open in new tab
    // ctrl-shift open in new tab and select
    // ctrl-alt open in new window
    if (type == QWebPage::NavigationTypeLinkClicked
        && (mKeyboardModifiers & Qt::ControlModifier
            || mPressedButtons == Qt::MidButton))
    {
        WBWebView *webView;

        bool selectNewTab = (mKeyboardModifiers & Qt::ShiftModifier);
        webView = mainWindow()->tabWidget()->newTab(selectNewTab);

        webView->load(request);
        mKeyboardModifiers = Qt::NoModifier;
        mPressedButtons = Qt::NoButton;

        return false;
    }

    if (frame == mainFrame())
    {
        mLoadingUrl = request.url();
        emit loadingUrl(mLoadingUrl);
    }

    return QWebPage::acceptNavigationRequest(frame, request, type);
}


QWebPage *WBWebPage::createWindow(QWebPage::WebWindowType type)
{
    Q_UNUSED(type);

    return mainWindow()->tabWidget()->newTab()->page();
}


QObject *WBWebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames
        , const QStringList &paramValues)
{
    Q_UNUSED(url);
    Q_UNUSED(paramNames);
    Q_UNUSED(paramValues);

    QUiLoader loader;

    return loader.createWidget(classId, view());
}


void WBWebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    if(reply->url().scheme() == "mailto")
    {
        bool result = QDesktopServices::openUrl(reply->url());
        if (result)
            return;
    }

    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    bool isPDF = (contentType == "application/pdf");

    // Delete this big "if (isPDF)" block to get the pdf directly inside the browser
    if (isPDF)
    {
        QMessageBox messageBox(mainWindow());
        messageBox.setText(tr("Download PDF Document: would you prefer to download the PDF file or add it to the current OpenBoard document?"));

        messageBox.addButton(tr("Download"), QMessageBox::AcceptRole);
        QAbstractButton *addButton = messageBox.addButton(tr("Add to Current Document"), QMessageBox::AcceptRole);

        messageBox.exec();
        if (messageBox.clickedButton() == addButton)
        {
            UBApplication::applicationController->showBoard();
            UBApplication::boardController->downloadURL(reply->request().url());
            return;
        }
        else
        {
            isPDF = false;
        }
    }

    if (!isPDF && reply->error() == QNetworkReply::NoError)
    {
        if(contentType == "application/widget")
            WBBrowserWindow::downloadManager()->handleUnsupportedContent(reply,false, UBSettings::settings()->userGipLibraryDirectory());
        else
            WBBrowserWindow::downloadManager()->handleUnsupportedContent(reply);
        return;
    }

    QFile file;
    file.setFileName(isPDF ? QLatin1String(":/webbrowser/object-wrapper.html") : QLatin1String(":/webbrowser/notfound.html"));

    bool isOpened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(isOpened);
    QString html;
    if (isPDF)
    {
        html = QString(QLatin1String(file.readAll()))
                        .arg(tr("PDF"))
                        .arg("application/x-ub-pdf")
                        .arg(reply->url().toString());
    }
    else
    {
        QString title = tr("Error loading page: %1").arg(reply->url().toString());
        html = QString(QLatin1String(file.readAll()))
                        .arg(title)
                        .arg(reply->errorString())
                        .arg(reply->url().toString());
    }

    QList<QWebFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty())
    {
        QWebFrame *frame = frames.takeFirst();
        if (frame->url() == reply->url())
        {
            frame->setHtml(html, reply->url());
            return;
        }
        QList<QWebFrame *> children = frame->childFrames();
        foreach(QWebFrame *frame, children)
            frames.append(frame);
    }

    if (mLoadingUrl == reply->url())
    {
        mainFrame()->setHtml(html, reply->url());
    }
}


WBWebView::WBWebView(QWidget* parent)
    : WBWebTrapWebView(parent)
    , mProgress(0)
    , mPage(new WBWebPage(this))
{
    setObjectName("ubBrowserWebView");

    setPage(mPage);

    QWebView::setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    connect(page(), SIGNAL(statusBarMessage(const QString&)),
            this, SLOT(setStatusBarText(const QString&)));

    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(setProgress(int)));

    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished(bool)));

    connect(this, SIGNAL(loadStarted()),
            this, SLOT(loadStarted()));

    connect(page(), SIGNAL(loadingUrl(const QUrl&)),
            this, SIGNAL(urlChanged(const QUrl &)));

    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)),
            this, SLOT(downloadRequested(const QNetworkRequest &)));

    page()->setForwardUnsupportedContent(true);

}


void WBWebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());

    if (!r.linkUrl().isEmpty())
    {
        QMenu menu(this);
        menu.addAction(pageAction(QWebPage::OpenLinkInNewWindow));
        menu.addAction(tr("Open in New Tab"), this, SLOT(openLinkInNewTab()));
        menu.addSeparator();
        menu.addAction(pageAction(QWebPage::DownloadLinkToDisk));
        // Add link to bookmarks...
        menu.addSeparator();
        menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));
        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
            menu.addAction(pageAction(QWebPage::InspectElement));
        menu.exec(mapToGlobal(event->pos()));
        return;
    }
    WBWebTrapWebView::contextMenuEvent(event);
}


void WBWebView::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
    {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        event->accept();
        return;
    }
    WBWebTrapWebView::wheelEvent(event);
}


void WBWebView::openLinkInNewTab()
{
    mPage->mOpenInNewTab = true;
    pageAction(QWebPage::OpenLinkInNewWindow)->trigger();
}


void WBWebView::setProgress(int progress)
{
    //qDebug() << "loading progress" << progress << "% in" << mLoadStartTime.elapsed() << "ms";

    mProgress = progress;
}


void WBWebView::loadStarted()
{
    mLoadStartTime.start();
}


void WBWebView::loadFinished(bool ok)
{
    if (100 != mProgress)
    {
        qWarning() << "Received finished signal while progress is still:" << progress()
                   << "Url:" << url();
    }

    QString error;

    if (!ok)
        error = "with error";

    qDebug() << "page loaded in" << mLoadStartTime.elapsed() << "ms" << url().toString() << error;

    mProgress = 0;
}


void WBWebView::load(const QUrl &url)
{
    qDebug() << "loading " << url.toString();

    mInitialUrl = url;

    WBWebTrapWebView::load(url);
}


QString WBWebView::lastStatusBarText() const
{
    return mLastStatusBarText;
}


QUrl WBWebView::url() const
{
    QUrl url;
    try{
        url = QWebView::url();
    } catch(...)
    {}

    if (!url.isEmpty())
       return url;

    return mInitialUrl;
}

void WBWebView::mousePressEvent(QMouseEvent *event)
{
    mPage->mPressedButtons = event->buttons();
    mPage->mKeyboardModifiers = event->modifiers();

    WBWebTrapWebView::mousePressEvent(event);
}

void WBWebView::mouseReleaseEvent(QMouseEvent *event)
{
    WBWebTrapWebView::mouseReleaseEvent(event);

    if (!event->isAccepted() && (mPage->mPressedButtons & Qt::MidButton))
    {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty())
        {
            setUrl(url);
        }
    }
}


void WBWebView::setStatusBarText(const QString &string)
{
    //qDebug() << "WebView::setStatusBarText" << string;

    mLastStatusBarText = string;
}


void WBWebView::downloadRequested(const QNetworkRequest &request)
{
    WBBrowserWindow::downloadManager()->download(request);
}

void WBWebView::load(const QNetworkRequest & request, QNetworkAccessManager::Operation operation, const QByteArray & body)
{
    WBWebTrapWebView::load(request, operation, body);
}

