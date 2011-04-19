/*
 * UBWebController.h
 *
 *  Created on: Nov 24, 2008
 *      Author: luc
 */

#ifndef UBWEBCONTROLLER_H_
#define UBWEBCONTROLLER_H_

#include <QtGui>
#include <QtWebKit>


class WBBrowserWindow;
class UBApplication;
class UBScreenMirror;
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
        bool isEduMedia(const QUrl& pUrl);

        void copy();
        void paste();
        void cut();

    private:

        QStackedWidget mStackedWidget[TotalNumberOfWebInstances];

        UBMainWindow *mMainWindow;

        WBBrowserWindow* mWebBrowserList[TotalNumberOfWebInstances];
        WBBrowserWindow** mCurrentWebBrowser;

        QWidget* mBrowserWidget;
        UBTrapFlashController* mTrapFlashController;
        UBWebToolsPalette** mToolsCurrentPalette;
        UBWebToolsPalette* mToolsPaletteList[TotalNumberOfWebInstances];

        bool mToolsPalettePositionned;
        bool mToolsPalettePositionnedList[TotalNumberOfWebInstances];

        bool mDownloadViewIsVisible;

        QStringList mOEmbedProviders;

        UBServerXMLHttpRequest* mGetOEmbedProviderListRequest;

        void tutorialWebInstance();
        void webBrowserInstance();
        void paraschoolWebInstance();

    private slots:

        void getOEmbedProviderList();

        void activePageChanged();
        void trapFlash();

        void toggleWebTrap(bool checked);

        void getOEmbedProviderListResponse(bool success, const QByteArray& payload);

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
