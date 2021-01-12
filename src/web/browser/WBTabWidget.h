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

#ifndef WBTABWIDGET_H
#define WBTABWIDGET_H

#include <QtGui>
#include <QShortcut>
#include <QStackedWidget>
#include <QCompleter>
#include <QTabBar>
#include <QWebPage>
#include <QTabWidget>
#include <QLineEdit>

/*
    Tab bar with a few more features such as a context menu and shortcuts
 */
class WBTabBar : public QTabBar
{
    Q_OBJECT;

    signals:
        void newTab();
        void cloneTab(int index);
        void closeTab(int index);
        void closeOtherTabs(int index);
        void reloadTab(int index);
        void reloadAllTabs();
        void tabMoveRequested(int fromIndex, int toIndex);

    public:
        WBTabBar(QWidget *parent = 0);

    protected:
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);

    private slots:
        void selectTabAction();
        void cloneTab();
        void closeTab();
        void closeOtherTabs();
        void reloadTab();
        void contextMenuRequested(const QPoint &position);

    private:
        QList<QShortcut*> mTabShortcuts;
        friend class WBTabWidget;

        QPoint mDragStartPos;
};


class WBWebView;
/*!
    A proxy object that connects a single browser action
    to one child webpage action at a time.

    Example usage: used to keep the main window stop action in sync with
    the current tabs webview's stop action.
 */
class WBWebActionMapper : public QObject
{
    Q_OBJECT;

    public:
        WBWebActionMapper(QAction *root, QWebPage::WebAction webAction, QObject *parent);
        QWebPage::WebAction webAction() const;
        void addChild(QAction *action);
        void updateCurrent(QWebPage *currentParent);

    private slots:
        void rootTriggered();
        void childChanged();
        void rootDestroyed();
        void currentDestroyed();

    private:
        QWebPage *mCurrentParent;
        QAction *mRootAction;
        QWebPage::WebAction mWebAction;
};



/*!
    TabWidget that contains WebViews and a stack widget of associated line edits.

    Connects up the current tab's signals to this class's signal and uses WebActionMapper
    to proxy the actions.
 */
class WBTabWidget : public QTabWidget
{
    Q_OBJECT

    signals:
        // tab widget signals
        void loadPage(const QString &url);

        // current tab signals
        void setCurrentTitle(const QString &url);
        void showStatusBarMessage(const QString &message);
        void linkHovered(const QString &link);
        void loadProgress(int progress);
        void loadFinished(bool pOk);
        void geometryChangeRequested(const QRect &geometry);
        void menuBarVisibilityChangeRequested(bool visible);
        void statusBarVisibilityChangeRequested(bool visible);
        void toolBarVisibilityChangeRequested(bool visible);
        void printRequested(QWebFrame *frame);

    public:
        WBTabWidget(QWidget *parent = 0);
        void clear();
        void addWebAction(QAction *action, QWebPage::WebAction webAction);

        QWidget *lineEditStack() const;
        QLineEdit *currentLineEdit() const;
        WBWebView *currentWebView() const;
        WBWebView *webView(int index) const;
        QLineEdit *lineEdit(int index) const;
        int webViewIndex(WBWebView *webView) const;

        QByteArray saveState() const;
        bool restoreState(const QByteArray &state);

        WBTabBar* tabBar() { return mTabBar; }
        QStackedWidget* lineEdits() { return mLineEdits; }

        void setLineEditStackVisible(bool visible) {mLineEdits->setVisible(visible);mLineEdits->hide();}
    protected:
        void mouseDoubleClickEvent(QMouseEvent *event);
        void contextMenuEvent(QContextMenuEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void paintEvent(QPaintEvent * event);
        QRect addTabButtonRect();

    public slots:
        void loadUrlInCurrentTab(const QUrl &url);
        WBWebView *newTab(bool makeCurrent = true);
        void cloneTab(int index = -1);
        void closeTab(int index = -1);
        void closeOtherTabs(int index);
        void reloadTab(int index = -1);
        void reloadAllTabs();
        void nextTab();
        void previousTab();

    private slots:
        void currentChanged(int index);
        void aboutToShowRecentTabsMenu();
        void aboutToShowRecentTriggeredAction(QAction *action);
        void webViewLoadStarted();
        void webViewIconChanged();
        void webViewTitleChanged(const QString &title);
        void webViewUrlChanged(const QUrl &url);
        void lineEditReturnPressed();
        void windowCloseRequested();
        void moveTab(int fromIndex, int toIndex);

    private:
        QAction *mRecentlyClosedTabsAction;

        QMenu *mRecentlyClosedTabsMenu;
        static const int sRecentlyClosedTabsSize = 10;
        QList<QUrl> mRecentlyClosedTabs;
        QList<WBWebActionMapper*> mWebActions;

        QCompleter *mLineEditCompleter;
        QStackedWidget *mLineEdits;
        WBTabBar *mTabBar;
        QPixmap mAddTabIcon;
};

#endif // WBTABWIDGET_H

