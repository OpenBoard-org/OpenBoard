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

#ifndef WBWEBVIEW_H
#define WBWEBVIEW_H

#include <QtGui>
#include <QtWebKit>

#include "WBWebTrapWebView.h"
#include "web/UBWebPage.h"

class WBBrowserWindow;

class WBWebPage : public UBWebPage
{
    Q_OBJECT;

    signals:
        void loadingUrl(const QUrl &url);

    public:
        WBWebPage(QObject *parent = 0);
        WBBrowserWindow *mainWindow();

    protected:
        bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);
        QWebPage *createWindow(QWebPage::WebWindowType type);
        QObject *createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);

    private slots:
        void handleUnsupportedContent(QNetworkReply *reply);

    private:
        friend class WBWebView;

        // set the webview mousepressedevent
        Qt::KeyboardModifiers mKeyboardModifiers;
        Qt::MouseButtons mPressedButtons;
        bool mOpenInNewTab;
        QUrl mLoadingUrl;
};

class WBWebView : public WBWebTrapWebView
{
    Q_OBJECT

    public:
        WBWebView(QWidget *parent = 0);
        WBWebPage *webPage() const { return mPage; }

        void load(const QUrl &url);
        void load ( const QNetworkRequest & request, QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation
                , const QByteArray & body = QByteArray());
        QUrl url() const;

        QString lastStatusBarText() const;
        inline int progress() const { return mProgress; }

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void contextMenuEvent(QContextMenuEvent *event);
        void wheelEvent(QWheelEvent *event);

    private slots:
        void setProgress(int progress);
        void loadFinished(bool ok);
        void loadStarted();

        void setStatusBarText(const QString &string);

        void downloadRequested(const QNetworkRequest &request);
        void openLinkInNewTab();

    private:
        QString mLastStatusBarText;
        QUrl mInitialUrl;
        int mProgress;
        WBWebPage *mPage;
        QTime mLoadStartTime;
};

#endif //WBWEBVIEW_H
