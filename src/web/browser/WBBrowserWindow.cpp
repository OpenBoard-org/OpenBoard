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

#include <QtGui>
#include <QWebHistory>
#include <QDesktopWidget>

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"

#include "network/UBNetworkAccessManager.h"
#include "network/UBAutoSaver.h"

#include "gui/UBMainWindow.h"

#include "pdf/UBWebPluginPDFWidget.h"


#include "WBChaseWidget.h"
#include "WBDownloadManager.h"
#include "WBHistory.h"
#include "WBTabWidget.h"
#include "WBToolBarSearch.h"
#include "ui_passworddialog.h"
#include "WBWebView.h"

#include "core/memcheck.h"

WBDownloadManager *WBBrowserWindow::sDownloadManager = 0;
WBHistoryManager *WBBrowserWindow::sHistoryManager = 0;


WBBrowserWindow::WBBrowserWindow(QWidget *parent, Ui::MainWindow* uniboardMainWindow)
        : QWidget(parent)
        , mWebToolBar(0)
        , mSearchToolBar(0)
        , mTabWidget(new WBTabWidget(this))
        , mSearchAction(0)
        , mUniboardMainWindow(uniboardMainWindow)
{
    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);

    setupMenu();
    setupToolBar();


    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(mTabWidget);

    this->setLayout(layout);

    connect(mTabWidget, SIGNAL(loadPage(const QString &)), this, SLOT(loadPage(const QString &)));


    connect(mTabWidget, SIGNAL(setCurrentTitle(const QString &)), this, SLOT(slotUpdateWindowTitle(const QString &)));

    connect(mTabWidget, SIGNAL(loadProgress(int)), this, SLOT(slotLoadProgress(int)));


    connect(mTabWidget, SIGNAL(loadFinished(bool)), this, SIGNAL(activeViewPageChanged()));

    connect(mTabWidget, SIGNAL(geometryChangeRequested(const QRect &)), this, SLOT(geometryChangeRequested(const QRect &)));

    slotUpdateWindowTitle();


    mTabWidget->newTab();

    connect(mTabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabCurrentChanged(int)));

}


WBBrowserWindow::~WBBrowserWindow()
{
    if(mTabWidget){
        delete mTabWidget;
        mTabWidget = NULL;
    }

    //Explanation mSearchToolBar has a parent so it's automatically freed
}


UBCookieJar *WBBrowserWindow::cookieJar()
{
    return (UBCookieJar*)UBNetworkAccessManager::defaultAccessManager()->cookieJar();
}


WBDownloadManager *WBBrowserWindow::downloadManager()
{
    if (!sDownloadManager) {
        sDownloadManager = new WBDownloadManager(UBApplication::mainWindow);
    }
    return sDownloadManager;
}


WBHistoryManager *WBBrowserWindow::historyManager()
{
    if (!sHistoryManager) {
        sHistoryManager = new WBHistoryManager();
        QWebHistoryInterface::setDefaultInterface(sHistoryManager);
    }
    return sHistoryManager;
}


QSize WBBrowserWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry(UBApplication::controlScreenIndex());
    QSize size = desktopRect.size() * qreal(0.9);
    return size;
}


void WBBrowserWindow::setupMenu()
{
    new QShortcut(QKeySequence(Qt::Key_F6), this, SLOT(slotSwapFocus()));
}


void WBBrowserWindow::setupToolBar()
{
    mWebToolBar = mUniboardMainWindow->webToolBar;

    mTabWidget->addWebAction(mUniboardMainWindow->actionWebBack, QWebPage::Back);
    mTabWidget->addWebAction(mUniboardMainWindow->actionWebForward, QWebPage::Forward);
    mTabWidget->addWebAction(mUniboardMainWindow->actionWebReload, QWebPage::Reload);
    mTabWidget->addWebAction(mUniboardMainWindow->actionStopLoading, QWebPage::Stop);

    mHistoryBackMenu = new QMenu(this);
    connect(mHistoryBackMenu, SIGNAL(aboutToShow()),this, SLOT(aboutToShowBackMenu()));
    connect(mHistoryBackMenu, SIGNAL(triggered(QAction *)), this, SLOT(openActionUrl(QAction *)));

    foreach (QWidget* menuWidget,  mUniboardMainWindow->actionWebBack->associatedWidgets())
    {
        QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

        if (tb && !tb->menu())
        {
            tb->setMenu(mHistoryBackMenu);
            tb->setStyleSheet("QToolButton::menu-indicator { subcontrol-position: bottom left; }");
        }
    }

    mHistoryForwardMenu = new QMenu(this);
    connect(mHistoryForwardMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowForwardMenu()));
    connect(mHistoryForwardMenu, SIGNAL(triggered(QAction *)), this, SLOT(openActionUrl(QAction *)));

    foreach (QWidget* menuWidget,  mUniboardMainWindow->actionWebForward->associatedWidgets())
    {
        QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

        if (tb && !tb->menu())
        {
            tb->setMenu(mHistoryForwardMenu);
            tb->setStyleSheet("QToolButton { padding-right: 8px; }");
        }
    }

    mWebToolBar->insertWidget(mUniboardMainWindow->actionWebBigger, mTabWidget->lineEditStack());

    mSearchToolBar = new WBToolbarSearch(mWebToolBar);

    mSearchAction = mWebToolBar->insertWidget(mUniboardMainWindow->actionWebBigger, mSearchToolBar);

    connect(mSearchToolBar, SIGNAL(search(const QUrl&)), SLOT(loadUrl(const QUrl&)));

    mChaseWidget = new WBChaseWidget(this);
    mWebToolBar->insertWidget(mUniboardMainWindow->actionWebBigger, mChaseWidget);
    mWebToolBar->insertSeparator(mUniboardMainWindow->actionWebBigger);

    connect(mUniboardMainWindow->actionHome, SIGNAL(triggered()), this , SLOT(slotHome()));

    connect(mUniboardMainWindow->actionBookmarks, SIGNAL(triggered()), this , SLOT(bookmarks()));
    connect(mUniboardMainWindow->actionAddBookmark, SIGNAL(triggered()), this , SLOT(addBookmark()));
    connect(mUniboardMainWindow->actionWebBigger, SIGNAL(triggered()), this , SLOT(slotViewZoomIn()));
    connect(mUniboardMainWindow->actionWebSmaller, SIGNAL(triggered()), this , SLOT(slotViewZoomOut()));

    mWebToolBar->show();
}


void WBBrowserWindow::adaptToolBar(bool wideRes)
{
    if (mSearchAction)
    {
        mSearchAction->setVisible(wideRes);
    }
}


QUrl WBBrowserWindow::guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (hasSchema)
    {
        int dotCount = urlStr.count(".");

        if (dotCount == 0 && !urlStr.contains(".com"))
        {
            urlStr += ".com";
        }

        QUrl url = QUrl::fromEncoded(urlStr.toUtf8(), QUrl::TolerantMode);
        if (url.isValid())
        {
            return url;
        }
    }

    // Might be a file.
    if (QFile::exists(urlStr))
    {
        QFileInfo info(urlStr);
        return QUrl::fromLocalFile(info.absoluteFilePath());
    }

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema)
    {
        QString schema = "http";

        QString guessed = schema + "://" + urlStr;

        int dotCount = guessed.count(".");

        if (dotCount == 0 && !urlStr.contains(".com"))
        {
            guessed += ".com";
        }

        QUrl url = QUrl::fromEncoded(guessed.toUtf8(), QUrl::TolerantMode);

        if (url.isValid())
            return url;
    }

    // Fall back to QUrl's own tolerant parser.
    QUrl url = QUrl::fromEncoded(string.toUtf8(), QUrl::TolerantMode);

    // finally for cases where the user just types in a hostname add http
    if (url.scheme().isEmpty())
        url = QUrl::fromEncoded("http://" + string.toUtf8(), QUrl::TolerantMode);

    return url;
}


void WBBrowserWindow::loadUrl(const QUrl &url)
{
    if (!currentTabWebView() || !url.isValid())
        return;

    mTabWidget->currentLineEdit()->setText(url.toString());
    mTabWidget->loadUrlInCurrentTab(url);
}


void WBBrowserWindow::loadUrlInNewTab(const QUrl &url)
{
    if (!url.isValid())
        return;

    mTabWidget->newTab();
    loadUrl(url);
}


WBWebView* WBBrowserWindow::createNewTab()
{
    return mTabWidget->newTab();
}


void WBBrowserWindow::slotSelectLineEdit()
{
    mTabWidget->currentLineEdit()->selectAll();
    mTabWidget->currentLineEdit()->setFocus();
}


void WBBrowserWindow::slotFileSaveAs()
{
    downloadManager()->download(currentTabWebView()->url(), true);
}


void WBBrowserWindow::slotUpdateStatusbar(const QString &string)
{
    Q_UNUSED(string);
}


void WBBrowserWindow::slotUpdateWindowTitle(const QString &title)
{
    Q_UNUSED(title);
}


void WBBrowserWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    deleteLater();
}


void WBBrowserWindow::slotViewZoomIn()
{
    WBWebView *currentWebView = currentTabWebView();

    if (!currentWebView)
        return;

    QWidget *view = currentWebView->page()->view();

    UBWebPluginPDFWidget *pdfPluginWidget = view ? view->findChild<UBWebPluginPDFWidget *>("PDFWebPluginWidget") : 0;
    if (pdfPluginWidget)
    {
        pdfPluginWidget->zoomIn();
    }
    else
    {
        currentWebView->setZoomFactor(currentWebView->zoomFactor() + 0.1);
    }
}


void WBBrowserWindow::slotViewZoomOut()
{
    WBWebView *currentWebView = currentTabWebView();

    if (!currentWebView)
        return;

    QWidget *view = currentWebView->page()->view();

    UBWebPluginPDFWidget *pdfPluginWidget = view ? view->findChild<UBWebPluginPDFWidget *>("PDFWebPluginWidget") : 0;

    if (pdfPluginWidget)
    {
        pdfPluginWidget->zoomOut();
    }
    else
    {
        currentWebView->setZoomFactor(currentWebView->zoomFactor() - 0.1);
    }
}


void WBBrowserWindow::slotViewResetZoom()
{
    if (!currentTabWebView())
        return;
    currentTabWebView()->setZoomFactor(1.0);
}


void WBBrowserWindow::slotViewZoomTextOnly(bool enable)
{
    if (!currentTabWebView())
        return;
    currentTabWebView()->page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, enable);
}


void WBBrowserWindow::slotHome()
{
    loadPage(UBSettings::settings()->webHomePage->get().toString());
}


void WBBrowserWindow::slotWebSearch()
{
    mSearchToolBar->lineEdit()->selectAll();
    mSearchToolBar->lineEdit()->setFocus();
}


void WBBrowserWindow::slotToggleInspector(bool enable)
{
    Q_UNUSED(enable);
}


void WBBrowserWindow::slotSwapFocus()
{
    if (currentTabWebView()->hasFocus())
        mTabWidget->currentLineEdit()->setFocus();
    else
        currentTabWebView()->setFocus();
}


void WBBrowserWindow::loadPage(const QString &page)
{
    QUrl url = guessUrlFromString(page);
    loadUrl(url);
}


WBTabWidget *WBBrowserWindow::tabWidget() const
{
    return mTabWidget;
}


WBWebView *WBBrowserWindow::currentTabWebView() const
{
    return mTabWidget->currentWebView();
}


void WBBrowserWindow::slotLoadProgress(int progress)
{
    if (mChaseWidget)
        mChaseWidget->setAnimated(progress > 0 && progress < 100);
}


void WBBrowserWindow::geometryChangeRequested(const QRect &geometry)
{
    setGeometry(geometry);
}


void WBBrowserWindow::tabCurrentChanged(int index)
{
    QWidget* current = mTabWidget->widget(index);

    emit activeViewChange(current);
    emit activeViewPageChanged();
}


void WBBrowserWindow::bookmarks()
{
    loadPage(UBSettings::settings()->webBookmarksPage->get().toString());
}


void WBBrowserWindow::addBookmark()
{
    loadPage(UBSettings::settings()->webAddBookmarkUrl->get().toString() + currentTabWebView()->url().toString());
}


WBWebView* WBBrowserWindow::paintWidget()
{
    return mTabWidget->currentWebView();
}


void WBBrowserWindow::showTabAtTop(bool attop)
{
    if (attop)
        mTabWidget->setTabPosition(QTabWidget::North);
    else
        mTabWidget->setTabPosition(QTabWidget::South);
}

void WBBrowserWindow::aboutToShowBackMenu()
{
    mHistoryBackMenu->clear();
    if (!currentTabWebView())
        return;
    QWebHistory *history = currentTabWebView()->history();

    int historyCount = history->count();
    int historyLimit = history->backItems(historyCount).count() - UBSettings::settings()->historyLimit->get().toReal();
    if (historyLimit < 0)
        historyLimit = 0;

    for (int i = history->backItems(historyCount).count() - 1; i >= historyLimit; --i)
    {
        QWebHistoryItem item = history->backItems(historyCount).at(i);

        QAction *action = new QAction(this);
        action->setData(-1*(historyCount-i-1));

        if (!QWebSettings::iconForUrl(item.originalUrl()).isNull())
        action->setIcon(item.icon());
        action->setText(item.title().isEmpty() ? item.url().toString() : item.title());
        mHistoryBackMenu->addAction(action);
    }

    mHistoryBackMenu->addSeparator();

    QAction *action = new QAction(this);
    action->setData("clear");
    action->setText("Clear history");
    mHistoryBackMenu->addAction(action);

}

void WBBrowserWindow::aboutToShowForwardMenu()
{
    mHistoryForwardMenu->clear();
    if (!currentTabWebView())
        return;
    QWebHistory *history = currentTabWebView()->history();
    int historyCount = history->count();

    int historyLimit = history->forwardItems(historyCount).count();
    if (historyLimit > UBSettings::settings()->historyLimit->get().toReal())
        historyLimit = UBSettings::settings()->historyLimit->get().toReal();

    for (int i = 0; i < historyLimit; ++i)
    {
        QWebHistoryItem item = history->forwardItems(historyCount).at(i);

        QAction *action = new QAction(this);
        action->setData(historyCount-i);

        if (!QWebSettings::iconForUrl(item.originalUrl()).isNull())
        action->setIcon(item.icon());
        action->setText(item.title().isEmpty() ? item.url().toString() : item.title());
        mHistoryForwardMenu->addAction(action);
    }

    mHistoryForwardMenu->addSeparator();

    QAction *action = new QAction(this);
    action->setData("clear");
    action->setText("Clear history");
    mHistoryForwardMenu->addAction(action);
}

void WBBrowserWindow::openActionUrl(QAction *action)
{
    QWebHistory *history = currentTabWebView()->history();

    if (action->data() == "clear")
    {
        history->clear();
        return;
    }

    int offset = action->data().toInt();
    if (offset < 0)
        history->goToItem(history->backItems(-1*offset).first());
    else if (offset > 0)
        history->goToItem(history->forwardItems(history->count() - offset + 1).back());
 }
