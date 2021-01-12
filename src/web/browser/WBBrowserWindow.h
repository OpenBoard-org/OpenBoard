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

#ifndef WBBROWSERMAINWINDOW_H
#define WBBROWSERMAINWINDOW_H

#include <QtGui>

class WBChaseWidget;
class WBTabWidget;
class WBToolbarSearch;
class WBWebView;
class WBHistoryManager;
class WBDownloadManager;
class UBCookieJar;

#include "ui_mainWindow.h"

/*!
    The MainWindow of the Browser view.

    Handles the tab widget and all the actions
 */
class WBBrowserWindow : public QWidget
{
    Q_OBJECT;

    public:
        WBBrowserWindow(QWidget *parent = 0, Ui::MainWindow* uniboardMainWindow = 0);
        ~WBBrowserWindow();
        QSize sizeHint() const;

    public:
        static QUrl guessUrlFromString(const QString &url);
        WBTabWidget *tabWidget() const;
        WBWebView *currentTabWebView() const;

        void adaptToolBar(bool wideRes);

        static WBHistoryManager *historyManager();
        static UBCookieJar *cookieJar();
        static WBDownloadManager *downloadManager();

    public slots:
        void loadPage(const QString &url);
        void slotHome();

        void loadUrl(const QUrl &url);
        void loadUrlInNewTab(const QUrl &url);

        WBWebView *createNewTab();

        WBWebView* paintWidget();

        void tabCurrentChanged(int);

        void bookmarks();
        void addBookmark();

        void showTabAtTop(bool attop);

        void aboutToShowBackMenu();
        void aboutToShowForwardMenu();
        void openActionUrl(QAction *action);

    signals:
        void activeViewPageChanged();
        void activeViewChange(QWidget*);
        void mirroringEnabled(bool checked);

    protected:
        void closeEvent(QCloseEvent *event);

    private slots:

        void slotLoadProgress(int);
        void slotUpdateStatusbar(const QString &string);
        void slotUpdateWindowTitle(const QString &title = QString());

        void slotFileSaveAs();

        void slotViewZoomIn();
        void slotViewZoomOut();
        void slotViewResetZoom();
        void slotViewZoomTextOnly(bool enable);

        void slotWebSearch();
        void slotToggleInspector(bool enable);
        void slotSelectLineEdit();
        void slotSwapFocus();

        void geometryChangeRequested(const QRect &geometry);

    private:

        static WBHistoryManager *sHistoryManager;
        static WBDownloadManager *sDownloadManager;

        void setupMenu();
        void setupToolBar();
        void updateStatusbarActionText(bool visible);

        QToolBar *mWebToolBar;
        WBToolbarSearch *mSearchToolBar;
        WBChaseWidget *mChaseWidget;
        WBTabWidget *mTabWidget;

        QAction *mSearchAction;

        QString mLastSearch;

        Ui::MainWindow* mUniboardMainWindow;

        QMenu    *mHistoryBackMenu;
        QMenu    *mHistoryForwardMenu;
};

#endif // WBBROWSERMAINWINDOW_H

