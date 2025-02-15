/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBWEBCONTROLLER_H_
#define UBWEBCONTROLLER_H_

#include <QtGui>
#include <QRegularExpression>

#include <QWebEnginePage>
#include <QWebEngineUrlRequestInterceptor>

#include "UBEmbedContent.h"
#include "simplebrowser/downloadmanagerwidget.h"

class BrowserWindow;
class WebView;
class QMenu;
class QWebEngineProfile;
class QWebEngineView;
class UBApplication;
class UBEmbedController;
class UBEmbedParser;
class UBMainWindow;
class UBWebToolsPalette;
class UBServerXMLHttpRequest;


class UBUserAgentInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    UBUserAgentInterceptor(const QByteArray &alternativeUserAgent, QObject *parent = nullptr);

    virtual void interceptRequest(QWebEngineUrlRequestInfo &info);

private:
    QByteArray mAlternativeUserAgent;
    QRegularExpression mDomainMatcher;
};

class UBWebController : public QObject
{
    Q_OBJECT

    public:
        enum CookiePolicy {
            DenyAll,
            DenyThirdParty,
            AcceptAll
        };
        Q_ENUM(CookiePolicy)

        UBWebController(UBMainWindow* mainWindow);
        virtual ~UBWebController();

        void closing();
        void adaptToolBar();

        void captureCurrentPage();
        void showTabAtTop(bool attop);
        void loadUrl(const QUrl& url);
        WebView* createNewTab();
        QUrl currentPageUrl() const;
        void show();
        QWidget* controlView() const;
        QWebEngineProfile* webProfile() const;
        QList<UBEmbedContent> getEmbeddedContent(const QWebEngineView* view) const;
        BrowserWindow* browserWindow() const;
#if (QT_VERSION < QT_VERSION_CHECK(6, 8, 0))
        QWebEnginePage::PermissionPolicy hasFeaturePermission(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
        void setFeaturePermission(const QUrl &securityOrigin, QWebEnginePage::Feature feature, QWebEnginePage::PermissionPolicy policy);
#endif

        static void injectScripts(QWebEngineView* view);

    protected:
        void setupPalettes();
        QPixmap getScreenPixmap();

    public slots:
        void screenLayoutChanged();

        void setSourceWidget(QWidget* pWidget);
        void captureWindow();
        void customCapture();
        void toogleMirroring(bool checked);

        void copy();
        void paste();
        void cut();

        void aboutToShowBackMenu();
        void aboutToShowForwardMenu();
        void openActionUrl(QAction *action);

    private:
        void webBrowserInstance();
        UBEmbedParser* embedParser(const QWebEngineView* view) const;
        void updateEmbeddableContent(const QWebEngineView* view) const;
        static QUrl guessUrlFromString(const QString &string);

    private slots:
        void tabCreated(WebView* webView);
        void activePageChanged();
        void trap();

        void onEmbedParsed(QWebEngineView* view, bool hasEmbeddedContent);
        void onOpenTutorial();
        void onHintsAndTips();

        void captureStripe(QPointF pos, QSize size, QPixmap *pix, QPointF scrollPosition);

    signals:
        /**
         * This signal is emitted once the screenshot has been performed. This signal is also emitted when user
         * click on go to uniboard button. In this case pCapturedPixmap is an empty pixmap.
         * @param pCapturedPixmap QPixmap corresponding to the capture.
         */
        void imageCaptured(const QPixmap& pCapturedPixmap, bool pageMode, const QUrl& source);

        void activeWebPageChanged(WebView* pWebView);

private:
        UBMainWindow *mMainWindow;

        BrowserWindow* mCurrentWebBrowser;
        DownloadManagerWidget mDownloadManagerWidget;

        QWidget* mBrowserWidget;
        UBEmbedController* mEmbedController;
        UBWebToolsPalette* mToolsCurrentPalette;

        QWebEngineProfile* mWebProfile;
        UBUserAgentInterceptor* mInterceptor;

        bool mToolsPalettePositionned;
        bool mDownloadViewIsVisible;

        QMenu* mHistoryBackMenu;
        QMenu* mHistoryForwardMenu;

#if (QT_VERSION < QT_VERSION_CHECK(6, 8, 0))
        QMap<QPair<QUrl,QWebEnginePage::Feature>,QWebEnginePage::PermissionPolicy> mFeaturePermissions;
#endif

        bool cookieAutoDelete;
        QStringList cookieKeepDomains;
};

#endif /* UBWEBCONTROLLER_H_ */
