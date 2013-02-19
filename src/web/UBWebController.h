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


#ifndef UBWEBCONTROLLER_H_
#define UBWEBCONTROLLER_H_

#include <QtGui>
#include <QtWebKit>

#include "UBOEmbedParser.h"

class WBBrowserWindow;
class UBApplication;
class UBTrapFlashController;
class UBMainWindow;
class UBWebToolsPalette;
class WBWebView;
class UBServerXMLHttpRequest;


class UBWebController : public QObject
{
    Q_OBJECT;

    public:
        UBWebController(UBMainWindow* mainWindow);
        virtual ~UBWebController();
        void closing();
        void adaptToolBar();

        QPixmap captureCurrentPage();

        void showTabAtTop(bool attop);

        void loadUrl(const QUrl& url);

        QWebView* createNewTab();

        QUrl currentPageUrl() const;

        enum WebInstance
        {
            WebBrowser = 0, Tutorial, Paraschool, TotalNumberOfWebInstances
        };

        void show(WebInstance type = UBWebController::WebBrowser);

        WBBrowserWindow* GetCurrentWebBrowser() 
        { 
            if( mCurrentWebBrowser != NULL ) return *mCurrentWebBrowser; 
            else return NULL;
        };


    protected:
        void setupPalettes();
        QPixmap getScreenPixmap();

    public slots:

        void screenLayoutChanged();

        void setSourceWidget(QWidget* pWidget);
        void toggleWebToolsPalette(bool checked);
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
        bool hasEmbeddedContent();
        void getEmbeddableContent();

        bool isEduMedia(const QUrl& pUrl);

        void copy();
        void paste();
        void cut();

    private:
        void initialiazemOEmbedProviders();
        void tutorialWebInstance();
        void webBrowserInstance();
        void paraschoolWebInstance();
        void lookForEmbedContent(QString* pHtml, QString tag, QString attribute, QList<QUrl>* pList);
        void checkForOEmbed(QString* pHtml);

        QStackedWidget *mStackedWidget;

        UBMainWindow *mMainWindow;

        WBBrowserWindow* mWebBrowserList[TotalNumberOfWebInstances];
        WBBrowserWindow** mCurrentWebBrowser;

        QWidget* mBrowserWidget;
        UBTrapFlashController* mTrapFlashController;
        UBWebToolsPalette** mToolsCurrentPalette;
        UBWebToolsPalette* mToolsPaletteList[TotalNumberOfWebInstances];
//         UBKeyboardPalette** mKeyboardCurrentPalette;
//         UBKeyboardPalette* mKeyboardPaletteList[TotalNumberOfWebInstances];

        bool mToolsPalettePositionned;
        bool mToolsPalettePositionnedList[TotalNumberOfWebInstances];

        bool mDownloadViewIsVisible;

        QStringList mOEmbedProviders;

        UBOEmbedParser mOEmbedParser;


    private slots:

        void activePageChanged();
        void trapFlash();

        void toggleWebTrap(bool checked);

        void onOEmbedParsed(QVector<sOEmbedContent> contents);
//        void showKeyboard(bool checked);

    signals:
        /**
         * This signal is emitted once the screenshot has been performed. This signal is also emitted when user
         * click on go to uniboard button. In this case pCapturedPixmap is an empty pixmap.
         * @param pCapturedPixmap QPixmap corresponding to the capture.
         */
        void imageCaptured(const QPixmap& pCapturedPixmap, bool pageMode, const QUrl& source);

        void activeWebPageChanged(WBWebView* pWebView);

};

#endif /* UBWEBCONTROLLER_H_ */
