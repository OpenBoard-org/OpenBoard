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

#include "WBTabWidget.h"

#include "WBBrowserWindow.h"
#include "WBHistory.h"
#include "WBUrlLineEdit.h"
#include "WBWebView.h"

#include "core/UBApplication.h" // TODO UB 4.x remove this nasty dependency
#include "core/UBApplicationController.h"// TODO UB 4.x remove this nasty dependency

#include <QtGui>
#include <QSvgWidget>
#include <QListView>
#include <QLabel>

#include "core/memcheck.h"

WBTabBar::WBTabBar(QWidget *parent)
    : QTabBar(parent)
{

    setObjectName("ubWebBrowserTabBar");

    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(contextMenuRequested(const QPoint &)));

    QString alt = QLatin1String("Alt+%1");
    for (int i = 1; i <= 10; ++i) {
        int key = i;
        if (key == 10)
            key = 0;
        QShortcut *shortCut = new QShortcut(alt.arg(key), this);
        mTabShortcuts.append(shortCut);
        connect(shortCut, SIGNAL(activated()), this, SLOT(selectTabAction()));
    }
    setTabsClosable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SIGNAL(closeTab(int)));
    setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    setMovable(true);
    setDocumentMode(false);

#ifdef Q_OS_OSX
    QFont baseFont = font();
    baseFont.setPointSize(baseFont.pointSize() - 2);
    setFont(baseFont);
#endif
}

void WBTabBar::selectTabAction()
{
    if (QShortcut *shortCut = qobject_cast<QShortcut*>(sender())) {
        int index = mTabShortcuts.indexOf(shortCut);
        if (index == 0)
            index = 10;
        setCurrentIndex(index);
    }
}

void WBTabBar::contextMenuRequested(const QPoint &position)
{
    QMenu menu;
    menu.addAction(tr("New &Tab"), this, SIGNAL(newTab()), QKeySequence::AddTab);
    int index = tabAt(position);
    if (-1 != index) {
        QAction *action = menu.addAction(tr("Clone Tab"), this, SLOT(cloneTab()));
        action->setData(index);

        menu.addSeparator();

        action = menu.addAction(tr("&Close Tab"), this, SLOT(closeTab()), QKeySequence::Close);
        action->setData(index);

        action = menu.addAction(tr("Close &Other Tabs"), this, SLOT(closeOtherTabs()));
        action->setData(index);

        menu.addSeparator();

        action = menu.addAction(tr("Reload Tab"), this, SLOT(reloadTab()), QKeySequence::Refresh);
        action->setData(index);
    }
    else {
        menu.addSeparator();
    }
    menu.addAction(tr("Reload All Tabs"), this, SIGNAL(reloadAllTabs()));
    menu.exec(QCursor::pos());
}

void WBTabBar::cloneTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit cloneTab(index);
    }
}

void WBTabBar::closeTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit closeTab(index);
    }
}

void WBTabBar::closeOtherTabs()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit closeOtherTabs(index);
    }
}

void WBTabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        mDragStartPos = event->pos();
    QTabBar::mousePressEvent(event);
}

void WBTabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        int diffX = event->pos().x() - mDragStartPos.x();
        int diffY = event->pos().y() - mDragStartPos.y();
        if ((event->pos() - mDragStartPos).manhattanLength() > QApplication::startDragDistance()
            && diffX < 3 && diffX > -3
            && diffY < -10) {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            QList<QUrl> urls;
            int index = tabAt(event->pos());
            QUrl url = tabData(index).toUrl();
            urls.append(url);
            mimeData->setUrls(urls);
            mimeData->setText(tabText(index));
            mimeData->setData(QLatin1String("action"), "tab-reordering");
            drag->setMimeData(mimeData);
            drag->exec();
        }
    }
    QTabBar::mouseMoveEvent(event);
}

// When index is -1 index chooses the current tab
void WBTabWidget::reloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    QWidget *widget = this->widget(index);
    if (WBWebView *tab = qobject_cast<WBWebView*>(widget))
        tab->reload();
}

void WBTabBar::reloadTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit reloadTab(index);
    }
}

WBTabWidget::WBTabWidget(QWidget *parent)
    : QTabWidget(parent)
    , mRecentlyClosedTabsMenu(0)
    , mLineEditCompleter(0)
    , mLineEdits(0)
    , mTabBar(new WBTabBar(this))
{
    setObjectName("ubWebBrowserTabWidget");
    mAddTabIcon = QPixmap(":/images/toolbar/plusBlack.png");

    setElideMode(Qt::ElideRight);

    connect(mTabBar, SIGNAL(newTab()), this, SLOT(newTab()));
    connect(mTabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(mTabBar, SIGNAL(cloneTab(int)), this, SLOT(cloneTab(int)));
    connect(mTabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(mTabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(mTabBar, SIGNAL(reloadAllTabs()), this, SLOT(reloadAllTabs()));
    connect(mTabBar, SIGNAL(tabMoved(int, int)), this, SLOT(moveTab(int, int)));
    setTabBar(mTabBar);
    setDocumentMode(false);

    mRecentlyClosedTabsMenu = new QMenu(this);
    connect(mRecentlyClosedTabsMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowRecentTabsMenu()));
    connect(mRecentlyClosedTabsMenu, SIGNAL(triggered(QAction *)), this, SLOT(aboutToShowRecentTriggeredAction(QAction *)));
    mRecentlyClosedTabsAction = new QAction(tr("Recently Closed Tabs"), this);
    mRecentlyClosedTabsAction->setMenu(mRecentlyClosedTabsMenu);
    mRecentlyClosedTabsAction->setEnabled(false);

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    mLineEdits = new QStackedWidget(this);
    mLineEdits->setMinimumWidth(200);
    QSizePolicy spolicy = mLineEdits->sizePolicy();
    mLineEdits->setSizePolicy(QSizePolicy::Maximum, spolicy.verticalPolicy());
}

void WBTabWidget::clear()
{
    // clear the recently closed tabs
    mRecentlyClosedTabs.clear();
    // clear the line edit history
    for (int i = 0; i < mLineEdits->count(); ++i) {
        QLineEdit *qLineEdit = lineEdit(i);
        qLineEdit->setText(qLineEdit->text());
    }
}

void WBTabWidget::moveTab(int fromIndex, int toIndex)
{
    QWidget *lineEdit = mLineEdits->widget(fromIndex);
    mLineEdits->removeWidget(lineEdit);
    mLineEdits->insertWidget(toIndex, lineEdit);
}

void WBTabWidget::addWebAction(QAction *action, QWebPage::WebAction webAction)
{
    if (!action)
        return;
    mWebActions.append(new WBWebActionMapper(action, webAction, this));
}

void WBTabWidget::currentChanged(int index)
{
    WBWebView *webView = this->webView(index);
    if (!webView)
        return;

    Q_ASSERT(mLineEdits->count() == count());

    WBWebView *oldWebView = this->webView(mLineEdits->currentIndex());
    if (oldWebView) {
        disconnect(oldWebView, SIGNAL(statusBarMessage(const QString&)),
                this, SIGNAL(showStatusBarMessage(const QString&)));
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
                this, SIGNAL(linkHovered(const QString&)));
        disconnect(oldWebView, SIGNAL(loadProgress(int)),
                this, SIGNAL(loadProgress(int)));
        disconnect(oldWebView, SIGNAL(loadFinished(bool)),
                this, SIGNAL(loadFinished(bool)));
    }

    connect(webView, SIGNAL(statusBarMessage(const QString&)),
            this, SIGNAL(showStatusBarMessage(const QString&)));
    connect(webView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
            this, SIGNAL(linkHovered(const QString&)));
    connect(webView, SIGNAL(loadProgress(int)),
            this, SIGNAL(loadProgress(int)));
    connect(webView, SIGNAL(loadFinished (bool)),
            this, SIGNAL(loadFinished(bool)));

    for (int i = 0; i < mWebActions.count(); ++i) {
        WBWebActionMapper *mapper = mWebActions[i];
        mapper->updateCurrent(webView->page());
    }
    emit setCurrentTitle(webView->title());
    mLineEdits->setCurrentIndex(index);
    emit loadProgress(webView->progress());
    emit showStatusBarMessage(webView->lastStatusBarText());
    if (webView->url().isEmpty())
        mLineEdits->currentWidget()->setFocus();
    else
        webView->setFocus();
}

QWidget *WBTabWidget::lineEditStack() const
{
    return mLineEdits;
}

QLineEdit *WBTabWidget::currentLineEdit() const
{
    return lineEdit(mLineEdits->currentIndex());
}

WBWebView *WBTabWidget::currentWebView() const
{
    return webView(currentIndex());
}

QLineEdit *WBTabWidget::lineEdit(int index) const
{
    WBUrlLineEdit *urlLineEdit = qobject_cast<WBUrlLineEdit*>(mLineEdits->widget(index));
    if (urlLineEdit)
        return urlLineEdit->lineEdit();
    return 0;
}

WBWebView *WBTabWidget::webView(int index) const
{
    QWidget *widget = this->widget(index);
    if (WBWebView *webView = qobject_cast<WBWebView*>(widget)) {
        return webView;
    } else {
        // optimization to delay creating the first webview
        if (count() == 1) {
            WBTabWidget *that = const_cast<WBTabWidget*>(this);
            that->setUpdatesEnabled(false);
            that->newTab();
            that->closeTab(0);
            that->setUpdatesEnabled(true);
            return currentWebView();
        }
    }
    return 0;
}

int WBTabWidget::webViewIndex(WBWebView *webView) const
{
    int index = indexOf(webView);
    return index;
}

WBWebView *WBTabWidget::newTab(bool makeCurrent)
{
    // line edit
    WBUrlLineEdit *urlLineEdit = new WBUrlLineEdit;

    QSizePolicy urlPolicy = urlLineEdit->sizePolicy();
    urlLineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, urlPolicy.verticalPolicy());

    QLineEdit *lineEdit = urlLineEdit->lineEdit();
    QSizePolicy policy = lineEdit->sizePolicy();
    lineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, policy.verticalPolicy());

    if (!mLineEditCompleter && count() > 0)
    {
        WBHistoryCompletionModel *completionModel = new WBHistoryCompletionModel(this);
        completionModel->setSourceModel(WBBrowserWindow::historyManager()->historyFilterModel());
        mLineEditCompleter = new QCompleter(completionModel, this);
        // Should this be in Qt by default?
        QAbstractItemView *popup = mLineEditCompleter->popup();
        QListView *listView = qobject_cast<QListView*>(popup);
        if (listView)
            listView->setUniformItemSizes(true);
    }

    lineEdit->setCompleter(mLineEditCompleter);
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(lineEditReturnPressed()));
    mLineEdits->addWidget(urlLineEdit);
    mLineEdits->setSizePolicy(lineEdit->sizePolicy());

    // optimization to delay creating the more expensive WebView, history, etc
    if (count() == 0)
    {
        QWidget *emptyWidget = new QWidget;
        QPalette p = emptyWidget->palette();
        p.setColor(QPalette::Window, palette().color(QPalette::Base));
        emptyWidget->setPalette(p);
        emptyWidget->setAutoFillBackground(true);
        disconnect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
        addTab(emptyWidget, tr("(Untitled)"));
        connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
        return 0;
    }

    // webview
    WBWebView *webView = new WBWebView;
    urlLineEdit->setWebView(webView);
    connect(webView, SIGNAL(loadStarted()), this, SLOT(webViewLoadStarted()));
    connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(iconChanged()), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(titleChanged(const QString &)), this, SLOT(webViewTitleChanged(const QString &)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(webViewUrlChanged(const QUrl &)));
    connect(webView->page(), SIGNAL(windowCloseRequested()), this, SLOT(windowCloseRequested()));
    connect(webView->page(), SIGNAL(geometryChangeRequested(const QRect &)), this, SIGNAL(geometryChangeRequested(const QRect &)));
    connect(webView->page(), SIGNAL(printRequested(QWebFrame *)), this, SIGNAL(printRequested(QWebFrame *)));
    connect(webView->page(), SIGNAL(menuBarVisibilityChangeRequested(bool)), this, SIGNAL(menuBarVisibilityChangeRequested(bool)));
    connect(webView->page(), SIGNAL(statusBarVisibilityChangeRequested(bool)), this, SIGNAL(statusBarVisibilityChangeRequested(bool)));
    connect(webView->page(), SIGNAL(toolBarVisibilityChangeRequested(bool)), this, SIGNAL(toolBarVisibilityChangeRequested(bool)));

    connect(webView, SIGNAL(pixmapCaptured(const QPixmap&, bool)), UBApplication::applicationController, SLOT(addCapturedPixmap(const QPixmap &, bool)));

    connect(webView, SIGNAL(embedCodeCaptured(const QString&)), UBApplication::applicationController, SLOT(addCapturedEmbedCode(const QString&)));

    addTab(webView, tr("(Untitled)"));
    if (makeCurrent)
        setCurrentWidget(webView);

    // webview actions
    for (int i = 0; i < mWebActions.count(); ++i)
    {
        WBWebActionMapper *mapper = mWebActions[i];
        mapper->addChild(webView->page()->action(mapper->webAction()));
    }

    if (count() == 1)
        currentChanged(currentIndex());

    return webView;
}

void WBTabWidget::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i)
    {
        QWidget *tabWidget = widget(i);
        if (WBWebView *tab = qobject_cast<WBWebView*>(tabWidget))
        {
            tab->reload();
        }
    }
}

void WBTabWidget::lineEditReturnPressed()
{
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender()))
    {
        emit loadPage(lineEdit->text());
        if (mLineEdits->currentWidget() == lineEdit)
            currentWebView()->setFocus();
    }
}

void WBTabWidget::windowCloseRequested()
{
    WBWebPage *webPage = qobject_cast<WBWebPage*>(sender());
    WBWebView *webView = qobject_cast<WBWebView*>(webPage->view());
    int index = webViewIndex(webView);
    if (index >= 0)
    {
        if (count() == 1)
            webView->webPage()->mainWindow()->close();
        else
            closeTab(index);
    }
}

void WBTabWidget::closeOtherTabs(int index)
{
    if (-1 == index)
        return;
    for (int i = count() - 1; i > index; --i)
        closeTab(i);
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}

// When index is -1 index chooses the current tab
void WBTabWidget::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;
    WBWebView *tab = newTab(false);
    tab->setUrl(webView(index)->url());
}

// When index is -1 index chooses the current tab
void WBTabWidget::closeTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    bool hasFocus = false;
    if (WBWebView *tab = webView(index))
    {
        hasFocus = tab->hasFocus();

        mRecentlyClosedTabsAction->setEnabled(true);
        mRecentlyClosedTabs.prepend(tab->url());
        if (mRecentlyClosedTabs.size() >= WBTabWidget::sRecentlyClosedTabsSize)
            mRecentlyClosedTabs.removeLast();
    }

    QWidget *lineEdit = mLineEdits->widget(index);
    mLineEdits->removeWidget(lineEdit);
    lineEdit->deleteLater();
    QWidget *webView = widget(index);
    removeTab(index);
    webView->deleteLater();

    if (hasFocus && count() > 0)
        currentWebView()->setFocus();

    if (count() == 0){
        newTab();
        emit currentChanged(0);
    }
}

void WBTabWidget::webViewLoadStarted()
{
    WBWebView *webView = qobject_cast<WBWebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        QIcon icon(QLatin1String(":loading.gif"));
        setTabIcon(index, icon);
    }
}

void WBTabWidget::webViewIconChanged()
{
    WBWebView *webView = qobject_cast<WBWebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        QIcon icon = webView->icon();

        WBUrlLineEdit *urlLineEdit = qobject_cast<WBUrlLineEdit*>(mLineEdits->widget(index));

        if (urlLineEdit)
        {
            QLabel *label = new QLabel(urlLineEdit);
            label->setGeometry(0, 0, 32, 32);
            label->setPixmap(icon.pixmap(16, 16));
            label->setAlignment(Qt::AlignCenter);
            label->show();
            urlLineEdit->setLeftWidget(label);
        }
    }
}

void WBTabWidget::webViewTitleChanged(const QString &title)
{
    WBWebView *webView = qobject_cast<WBWebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        setTabText(index, title);
    }
    if (currentIndex() == index)
        emit setCurrentTitle(title);
    WBBrowserWindow::historyManager()->updateHistoryItem(webView->url(), title);
}

void WBTabWidget::webViewUrlChanged(const QUrl &url)
{
    WBWebView *webView = qobject_cast<WBWebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        mTabBar->setTabData(index, url);
    }

    //  web view does not reload sometime on OSX ... force it
    webView->show();

}

void WBTabWidget::aboutToShowRecentTabsMenu()
{
    mRecentlyClosedTabsMenu->clear();
    for (int i = 0; i < mRecentlyClosedTabs.count(); ++i)
    {
        QAction *action = new QAction(mRecentlyClosedTabsMenu);
        action->setData(mRecentlyClosedTabs.at(i));

        action->setText(mRecentlyClosedTabs.at(i).toString());
        mRecentlyClosedTabsMenu->addAction(action);
    }
}

void WBTabWidget::aboutToShowRecentTriggeredAction(QAction *action)
{
    QUrl url = action->data().toUrl();
    loadUrlInCurrentTab(url);
}

void WBTabWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height()))
    {
        newTab();
        return;
    }
    QTabWidget::mouseDoubleClickEvent(event);
}

void WBTabWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!childAt(event->pos()))
    {
        mTabBar->contextMenuRequested(event->pos());
        return;
    }
    QTabWidget::contextMenuEvent(event);
}

void WBTabWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QRect addTabRect = addTabButtonRect();

    if (addTabRect.contains(event->pos()))
    {
        newTab();
    }
    else if (event->button() == Qt::MidButton && !childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height()))
    {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty())
        {
            WBWebView *webView = newTab();
            webView->setUrl(url);
        }
    }
}

void WBTabWidget::loadUrlInCurrentTab(const QUrl &url)
{
    WBWebView *webView = currentWebView();
    if (webView)
    {
        webView->load(url);
        webView->setFocus();
    }
}

void WBTabWidget::nextTab()
{
    int next = currentIndex() + 1;
    if (next == count())
        next = 0;
    setCurrentIndex(next);
}

void WBTabWidget::previousTab()
{
    int next = currentIndex() - 1;
    if (next < 0)
        next = count() - 1;
    setCurrentIndex(next);
}

static const qint32 TabWidgetMagic = 0xaa;

QByteArray WBTabWidget::saveState() const
{
    int version = 1;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(TabWidgetMagic);
    stream << qint32(version);

    QStringList tabs;
    for (int i = 0; i < count(); ++i)
    {
        if (WBWebView *tab = qobject_cast<WBWebView*>(widget(i)))
        {
            tabs.append(tab->url().toString());
        } else {
            tabs.append(QString::null);
        }
    }
    stream << tabs;
    stream << currentIndex();
    return data;
}

bool WBTabWidget::restoreState(const QByteArray &state)
{
    int version = 1;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != TabWidgetMagic || v != version)
        return false;

    QStringList openTabs;
    stream >> openTabs;

    for (int i = 0; i < openTabs.count(); ++i)
    {
        if (i != 0)
            newTab();
        loadPage(openTabs.at(i));
    }

    int currentTab;
    stream >> currentTab;
    setCurrentIndex(currentTab);

    return true;
}

WBWebActionMapper::WBWebActionMapper(QAction *root, QWebPage::WebAction webAction, QObject *parent)
    : QObject(parent)
    , mCurrentParent(0)
    , mRootAction(root)
    , mWebAction(webAction)
{
    if (!mRootAction)
        return;
    connect(mRootAction, SIGNAL(triggered()), this, SLOT(rootTriggered()));
    connect(root, SIGNAL(destroyed(QObject *)), this, SLOT(rootDestroyed()));
    root->setEnabled(false);
}

void WBWebActionMapper::rootDestroyed()
{
    mRootAction = 0;
}

void WBWebActionMapper::currentDestroyed()
{
    updateCurrent(0);
}

void WBWebActionMapper::addChild(QAction *action)
{
    if (!action)
        return;
    connect(action, SIGNAL(changed()), this, SLOT(childChanged()));
}

QWebPage::WebAction WBWebActionMapper::webAction() const
{
    return mWebAction;
}

void WBWebActionMapper::rootTriggered()
{
    if (mCurrentParent)
    {
        QAction *gotoAction = mCurrentParent->action(mWebAction);
        gotoAction->trigger();
    }
}

void WBWebActionMapper::childChanged()
{
    if (QAction *source = qobject_cast<QAction*>(sender()))
    {
        if (mRootAction
            && mCurrentParent
            && source->parent() == mCurrentParent)
        {
            mRootAction->setChecked(source->isChecked());
            mRootAction->setEnabled(source->isEnabled());
        }
    }
}

void WBWebActionMapper::updateCurrent(QWebPage *currentParent)
{
    if (mCurrentParent)
        disconnect(mCurrentParent, SIGNAL(destroyed(QObject *)),
                   this, SLOT(currentDestroyed()));

    mCurrentParent = currentParent;
    if (!mRootAction)
        return;
    if (!mCurrentParent)
    {
        mRootAction->setEnabled(false);
        mRootAction->setChecked(false);
        return;
    }
    QAction *source = mCurrentParent->action(mWebAction);
    mRootAction->setChecked(source->isChecked());
    mRootAction->setEnabled(source->isEnabled());
    connect(mCurrentParent, SIGNAL(destroyed(QObject *)),
            this, SLOT(currentDestroyed()));
}


QRect WBTabWidget::addTabButtonRect()
{
    QRect lastTabRect = tabBar()->tabRect(tabBar()->count() -1);
    int x = lastTabRect.topRight().x();
    int y = 0;

    if (tabPosition() == QTabWidget::North)
        y = lastTabRect.topRight().y();
    else if (tabPosition() == QTabWidget::South)
        y = geometry().height() - lastTabRect.height();
    else
        qDebug() << "WBTabWidget::addTabButtonRect() - unsupported tab posion";

    // all this is in synch with CSS QTabBar ...
    QRect r(x + 3, y + 6 , 25, lastTabRect.height() - 8);

    return r;
}


void WBTabWidget::paintEvent ( QPaintEvent * event )
{
    QPainter painter(this);

    // all this is in synch with CSS QTabBar ...
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, 1));
    linearGrad.setColorAt(0, QColor("#d3d3d3"));
    linearGrad.setColorAt(1, QColor("#dddddd"));
    painter.setBrush(linearGrad);

    QRect r = addTabButtonRect();

    painter.setPen(QColor("#888888"));
    painter.drawRoundedRect(r, 3, 3);

    painter.setPen(Qt::NoPen);
    painter.drawRect(QRect(r.x(), r.y() + r.height() / 2, r.width() + 1, r.height() / 2 + 1));

    painter.setPen(QColor("#888888"));
    painter.drawLine(r.x(), r.y() + r.height() / 2, r.x(), r.bottom());
    painter.drawLine(r.right() + 1, r.y() + r.height() / 2, r.right() + 1, r.bottom());

    if (tabPosition() == QTabWidget::South)
    {
        QPen pen = painter.pen();
        pen.setColor(QColor("#b3b3b3"));
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawLine(0, r.bottom() + 2, width(), r.bottom() + 2);
    }

    QPointF topLeft = r.center() - QPointF(mAddTabIcon.width() / 2, mAddTabIcon.height() / 2);
    painter.drawPixmap(topLeft, mAddTabIcon);

    painter.end();

    QTabWidget::paintEvent(event);
}

