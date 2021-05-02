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




#ifndef UBWEBCONTROLLER_H_
#define UBWEBCONTROLLER_H_

#include <QtGui>

#include "UBOEmbedParser.h"
#include "simplebrowser/downloadmanagerwidget.h"

class BrowserWindow;
class WebView;
class UBApplication;
class UBTrapFlashController;
class UBMainWindow;
class UBWebToolsPalette;
class UBServerXMLHttpRequest;


class UBWebController : public QObject
{
    Q_OBJECT

    public:
        UBWebController(UBMainWindow* mainWindow);
        virtual ~UBWebController();
        void closing();
        void adaptToolBar();

        QPixmap captureCurrentPage();

        void showTabAtTop(bool attop);

        void loadUrl(const QUrl& url);

        WebView* createNewTab();

        QUrl currentPageUrl() const;

        void show();

        BrowserWindow* GetCurrentWebBrowser(){return mCurrentWebBrowser;}


    protected:
        void setupPalettes();
        QPixmap getScreenPixmap();

    public slots:

        void screenLayoutChanged();

        void setSourceWidget(QWidget* pWidget);
        void captureWindow();
        void customCapture();
        void toogleMirroring(bool checked);

        QWidget* controlView()
        {
            return mBrowserWidget;
        }

        void captureoEmbed();
        void captureEduMedia();

        bool isOEmbedable(const QUrl& pUrl);
        // FIXME bool hasEmbeddedContent();
        // FIXME void getEmbeddableContent();

        bool isEduMedia(const QUrl& pUrl);

        void copy();
        void paste();
        void cut();

    private:
        void initialiazemOEmbedProviders();
        void webBrowserInstance();
        void lookForEmbedContent(QString* pHtml, QString tag, QString attribute, QList<QUrl>* pList);
        void checkForOEmbed(QString* pHtml);
        static QUrl guessUrlFromString(const QString &string);

        UBMainWindow *mMainWindow;

        BrowserWindow* mCurrentWebBrowser;
        DownloadManagerWidget m_downloadManagerWidget;

        QWidget* mBrowserWidget;
        UBTrapFlashController* mTrapFlashController;
        UBWebToolsPalette* mToolsCurrentPalette;

        bool mToolsPalettePositionned;

        bool mDownloadViewIsVisible;

        QStringList mOEmbedProviders;

        UBOEmbedParser mOEmbedParser;


    private slots:

        void activePageChanged();
        void trapFlash();

        void toggleWebTrap(bool checked);

        void onOEmbedParsed(QVector<sOEmbedContent> contents);
        void onOpenTutorial();


    signals:
        /**
         * This signal is emitted once the screenshot has been performed. This signal is also emitted when user
         * click on go to uniboard button. In this case pCapturedPixmap is an empty pixmap.
         * @param pCapturedPixmap QPixmap corresponding to the capture.
         */
        void imageCaptured(const QPixmap& pCapturedPixmap, bool pageMode, const QUrl& source);

        void activeWebPageChanged(WebView* pWebView);

};

#endif /* UBWEBCONTROLLER_H_ */
