/*
 * UBWebController.cpp
 *
 *  Created on: Nov 24, 2008
 *      Author: luc
 */

#include <QtGui>

#include "frameworks/UBPlatformUtils.h"

#include "UBWebController.h"
#include "UBTrapFlashController.h"

#include "web/browser/WBBrowserWindow.h"
#include "web/browser/WBWebView.h"
#include "web/browser/WBDownloadManager.h"
#include "web/browser/WBTabWidget.h"

#include "network/UBServerXMLHttpRequest.h"
#include "network/UBNetworkAccessManager.h"

#include "gui/UBWidgetMirror.h"
#include "gui/UBScreenMirror.h"
#include "gui/UBMainWindow.h"
#include "gui/UBWebToolsPalette.h"

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"

#include "domain/UBGraphicsScene.h"

#include "desktop/UBCustomCaptureWindow.h"


UBWebController::UBWebController(UBMainWindow* mainWindow)
    : QObject(mainWindow->centralWidget())
    , mMainWindow(mainWindow)
    , mCurrentWebBrowser(0)
    , mBrowserWidget(0)
    , mTrapFlashController(0)
    , mToolsCurrentPalette(0)
    , mToolsPalettePositionned(false)
    , mDownloadViewIsVisible(false)

{
    connect(mMainWindow->actionWebTools, SIGNAL(toggled(bool)), this, SLOT(toggleWebToolsPalette(bool)));

    mMainWindow->addWebWidget(mStackedWidget);

    for (int i = 0; i < TotalNumberOfWebInstances; i += 1){
        mWebBrowserList[i] = 0;
        mToolsPaletteList[i] = 0;
        mToolsPalettePositionnedList[i] = false;
    }

}


UBWebController::~UBWebController()
{
    // NOOP
}

void UBWebController::webBrowserInstance()
{
    QString webHomePage = UBSettings::settings()->webHomePage->get().toString();
    QUrl currentUrl = WBBrowserWindow::guessUrlFromString(webHomePage);

    if (UBSettings::settings()->webUseExternalBrowser->get().toBool())
    {
        QDesktopServices::openUrl(currentUrl);
    }
    else
    {
        mCurrentWebBrowser = &mWebBrowserList[WebBrowser];
        mToolsCurrentPalette = &mToolsPaletteList[WebBrowser];
        mToolsPalettePositionned = mToolsPalettePositionnedList[WebBrowser];
        if (!(*mCurrentWebBrowser))
        {
            (*mCurrentWebBrowser) = new WBBrowserWindow(mMainWindow->centralWidget(), mMainWindow);

            connect((*mCurrentWebBrowser), SIGNAL(activeViewChange(QWidget*)), this, SLOT(setSourceWidget(QWidget*)));

            WBBrowserWindow::downloadManager()->setParent((*mCurrentWebBrowser), Qt::Tool);

            UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->webToolBar,
                    mMainWindow->actionBoard, 32);

            UBApplication::app()->decorateActionMenu(mMainWindow->actionMenu);

            bool showAddBookmarkButtons = UBSettings::settings()->webShowAddBookmarkButton->get().toBool();
            mMainWindow->actionBookmarks->setVisible(showAddBookmarkButtons);
            mMainWindow->actionAddBookmark->setVisible(showAddBookmarkButtons);

            mStackedWidget->insertWidget(WebBrowser, (*mCurrentWebBrowser));

            showTabAtTop(UBSettings::settings()->appToolBarPositionedAtTop->get().toBool());

            adaptToolBar();

            mTrapFlashController = new UBTrapFlashController((*mCurrentWebBrowser));

            connect((*mCurrentWebBrowser), SIGNAL(activeViewPageChanged()), this, SLOT(activePageChanged()));

            (*mCurrentWebBrowser)->loadUrl(currentUrl);

            (*mCurrentWebBrowser)->tabWidget()->tabBar()->show();
            (*mCurrentWebBrowser)->tabWidget()->lineEdits()->show();
		}

        UBApplication::applicationController->setMirrorSourceWidget((*mCurrentWebBrowser)->paintWidget());

        mStackedWidget->setCurrentIndex(WebBrowser);
        mMainWindow->switchToWebWidget();

		setupPalettes();
		screenLayoutChanged();

        bool mirroring = UBSettings::settings()->webShowPageImmediatelyOnMirroredScreen->get().toBool();
        UBApplication::mainWindow->actionWebShowHideOnDisplay->setChecked(mirroring);
		(*mToolsCurrentPalette)->show();
	}

    if (mDownloadViewIsVisible)
       WBBrowserWindow::downloadManager()->show();
	
}

void UBWebController::tutorialWebInstance()
{
    QLocale locale = QLocale();
    QString language = "_" + locale.name().left(2);
    qDebug() << language;
    QString tutorialHtmlIndexFile = 0;
    QString tutorialPath = "/etc/Paraschool/Tutorial/site" + language + "/index.html";
#if defined(Q_WS_MAC)
    tutorialHtmlIndexFile = QApplication::applicationDirPath()+ "/../Resources" + tutorialPath;
#elif defined(Q_WS_WIN)
    tutorialHtmlIndexFile = QApplication::applicationDirPath()+ tutorialPath;
#else
    tutorialHtmlIndexFile = QApplication::applicationDirPath()+ tutorialPath;
#endif

    QUrl currentUrl = QUrl::fromLocalFile(tutorialHtmlIndexFile);

    if (UBSettings::settings()->webUseExternalBrowser->get().toBool())
    {
        QDesktopServices::openUrl(currentUrl);
    }
    else
    {
        mCurrentWebBrowser = &mWebBrowserList[Tutorial];
        mToolsCurrentPalette = &mToolsPaletteList[Tutorial];
        mToolsPalettePositionned = &mToolsPalettePositionnedList[Tutorial];
        if (!(*mCurrentWebBrowser))
        {
            (*mCurrentWebBrowser) = new WBBrowserWindow(mMainWindow->centralWidget(), mMainWindow, true);
            connect((*mCurrentWebBrowser), SIGNAL(activeViewChange(QWidget*)), this, SLOT(setSourceWidget(QWidget*)));

            mStackedWidget->insertWidget(Tutorial, (*mCurrentWebBrowser));

            adaptToolBar();

            mTrapFlashController = new UBTrapFlashController((*mCurrentWebBrowser));

            connect((*mCurrentWebBrowser), SIGNAL(activeViewPageChanged()), this, SLOT(activePageChanged()));
            (*mCurrentWebBrowser)->loadUrl(currentUrl);

            (*mCurrentWebBrowser)->tabWidget()->tabBar()->hide();
            (*mCurrentWebBrowser)->tabWidget()->lineEdits()->hide();

        }

        UBApplication::applicationController->setMirrorSourceWidget((*mCurrentWebBrowser)->paintWidget());


        mStackedWidget->setCurrentIndex(Tutorial);
        mMainWindow->switchToWebWidget();

        screenLayoutChanged();

        bool mirroring = UBSettings::settings()->webShowPageImmediatelyOnMirroredScreen->get().toBool();
        UBApplication::mainWindow->actionWebShowHideOnDisplay->setChecked(mirroring);

    }

}

void UBWebController::paraschoolWebInstance()
{
    QUrl currentUrl("http://apps.dev-paraschool.com/editor");

    if (UBSettings::settings()->webUseExternalBrowser->get().toBool())
    {
        QDesktopServices::openUrl(currentUrl);
    }
    else
    {
        mCurrentWebBrowser = &mWebBrowserList[Paraschool];
        mToolsCurrentPalette = &mToolsPaletteList[Paraschool];
        mToolsPalettePositionned = &mToolsPalettePositionnedList[Paraschool];
        if (!(*mCurrentWebBrowser))
        {
            (*mCurrentWebBrowser) = new WBBrowserWindow(mMainWindow->centralWidget(), mMainWindow, true);
            connect((*mCurrentWebBrowser), SIGNAL(activeViewChange(QWidget*)), this, SLOT(setSourceWidget(QWidget*)));

            mStackedWidget->insertWidget(Paraschool, (*mCurrentWebBrowser));

            adaptToolBar();

            mTrapFlashController = new UBTrapFlashController((*mCurrentWebBrowser));

            connect((*mCurrentWebBrowser), SIGNAL(activeViewPageChanged()), this, SLOT(activePageChanged()));
            (*mCurrentWebBrowser)->loadUrl(currentUrl);
            (*mCurrentWebBrowser)->tabWidget()->tabBar()->hide();
            (*mCurrentWebBrowser)->tabWidget()->lineEdits()->hide();

        }

        UBApplication::applicationController->setMirrorSourceWidget((*mCurrentWebBrowser)->paintWidget());

        mStackedWidget->setCurrentIndex(Paraschool);
        mMainWindow->switchToWebWidget();

        screenLayoutChanged();

        bool mirroring = UBSettings::settings()->webShowPageImmediatelyOnMirroredScreen->get().toBool();
        UBApplication::mainWindow->actionWebShowHideOnDisplay->setChecked(mirroring);
    }
}


void UBWebController::show(WebInstance type)
{
    switch(type)
    {
        case WebBrowser:
            webBrowserInstance();
            break;
        case Tutorial:
            tutorialWebInstance();
            break;
        case Paraschool:
            paraschoolWebInstance();
        default:
            qCritical() << __FILE__ << " non supported web instance type " << QString::number(type) ;
            break;
    }
}

void UBWebController::setSourceWidget(QWidget* pWidget)
{
    mBrowserWidget = pWidget;
    UBApplication::applicationController->setMirrorSourceWidget(pWidget);
}


void UBWebController::trapFlash()
{
    mTrapFlashController->showTrapFlash();
    activePageChanged();
}


void UBWebController::activePageChanged()
{
    if (mCurrentWebBrowser && (*mCurrentWebBrowser)->currentTabWebView())
    {
        if (mTrapFlashController && (*mCurrentWebBrowser)->currentTabWebView()->page())
        {
            mTrapFlashController->updateTrapFlashFromPage((*mCurrentWebBrowser)->currentTabWebView()->page()->currentFrame());
        }

        mMainWindow->actionWebTrap->setChecked(false);

        QUrl latestUrl = (*mCurrentWebBrowser)->currentTabWebView()->url();
        UBApplication::mainWindow->actionWebOEmbed->setEnabled(isOEmbedable(latestUrl));
        UBApplication::mainWindow->actionEduMedia->setEnabled(isEduMedia(latestUrl));

        emit activeWebPageChanged((*mCurrentWebBrowser)->currentTabWebView());
    }
}


QPixmap UBWebController::captureCurrentPage()
{
    QPixmap pix;

    if (mCurrentWebBrowser
        && (*mCurrentWebBrowser)
        && (*mCurrentWebBrowser)->currentTabWebView()
        && (*mCurrentWebBrowser)->currentTabWebView()->page()
        && (*mCurrentWebBrowser)->currentTabWebView()->page()->mainFrame())
    {
        QWebFrame* frame = (*mCurrentWebBrowser)->currentTabWebView()->page()->mainFrame();
        QSize size = frame->contentsSize();

        qDebug() << size;

        QVariant top = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientTop");
        QVariant left = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientLeft");
        QVariant width = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientWidth");
        QVariant height = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientHeight");

        QSize vieportSize = (*mCurrentWebBrowser)->currentTabWebView()->page()->viewportSize();
        (*mCurrentWebBrowser)->currentTabWebView()->page()->setViewportSize(frame->contentsSize());
        pix = QPixmap(frame->geometry().width(), frame->geometry().height());

        {
            QPainter p(&pix);
            frame->render(&p);
        }

        if (left.isValid() && top.isValid() && width.isValid() && height.isValid())
        {
            bool okLeft, okTop, okWidth, okHeight;

            int iLeft = left.toInt(&okLeft) * frame->zoomFactor();
            int iTop = top.toInt(&okTop) * frame->zoomFactor();
            int iWidth = width.toInt(&okWidth) * frame->zoomFactor();
            int iHeight = height.toInt(&okHeight) * frame->zoomFactor();

            if(okLeft && okTop && okWidth && okHeight)
            {
                pix = pix.copy(iLeft, iTop, iWidth, iHeight);
            }
        }


        (*mCurrentWebBrowser)->currentTabWebView()->page()->setViewportSize(vieportSize);
    }

    return pix;
}


void UBWebController::setupPalettes()
{
	if(!(*mToolsCurrentPalette)){
		(*mToolsCurrentPalette) = new UBWebToolsPalette((*mCurrentWebBrowser),false);

		connect(mMainWindow->actionWebTrapFlash, SIGNAL(triggered()), this, SLOT(trapFlash()));
	    connect(mMainWindow->actionWebCustomCapture, SIGNAL(triggered()), this, SLOT(customCapture()));
		connect(mMainWindow->actionWebWindowCapture, SIGNAL(triggered()), this, SLOT(captureWindow()));
		connect(mMainWindow->actionWebOEmbed, SIGNAL(triggered()), this, SLOT(captureoEmbed()));
		connect(mMainWindow->actionEduMedia, SIGNAL(triggered()), this, SLOT(captureEduMedia()));

		connect(mMainWindow->actionWebShowHideOnDisplay, SIGNAL(toggled(bool)), this, SLOT(toogleMirroring(bool)));
		connect(mMainWindow->actionWebTrap, SIGNAL(toggled(bool)), this, SLOT(toggleWebTrap(bool)));

		(*mToolsCurrentPalette)->hide();
		(*mToolsCurrentPalette)->adjustSizeAndPosition();


		if (controlView()){
			int left = controlView()->width() - 20 - (*mToolsCurrentPalette)->width();
			int top = (controlView()->height() - (*mToolsCurrentPalette)->height()) / 2;
			mToolsPalettePositionnedList[mStackedWidget->currentIndex()] = true;
			(*mToolsCurrentPalette)->setCustomPosition(true);
			(*mToolsCurrentPalette)->move(left, top);
		}
		mMainWindow->actionWebTools->trigger();
	}
}


void UBWebController::toggleWebTrap(bool checked)
{
    if (mCurrentWebBrowser
        && (*mCurrentWebBrowser)
        && (*mCurrentWebBrowser)->currentTabWebView())
    {
        (*mCurrentWebBrowser)->currentTabWebView()->setIsTrapping(checked);
    }
}


void UBWebController::toggleWebToolsPalette(bool checked)
{
    (*mToolsCurrentPalette)->setVisible(checked);
}


void UBWebController::captureWindow()
{
    QPixmap webPagePixmap = captureCurrentPage();

    if (!webPagePixmap.isNull())
    {
        emit imageCaptured(webPagePixmap, true, (*mCurrentWebBrowser)->currentTabWebView()->url());
    }
}


void UBWebController::customCapture()
{
    (*mToolsCurrentPalette)->setVisible(false);
    qApp->processEvents();

    UBCustomCaptureWindow customCaptureWindow((*mCurrentWebBrowser));

    customCaptureWindow.show();

    if (customCaptureWindow.execute(getScreenPixmap()) == QDialog::Accepted)
    {
        QPixmap selectedPixmap = customCaptureWindow.getSelectedPixmap();
        emit imageCaptured(selectedPixmap, false, (*mCurrentWebBrowser)->currentTabWebView()->url());
    }

    (*mToolsCurrentPalette)->setVisible(true);
}


void UBWebController::toogleMirroring(bool checked)
{
    UBApplication::applicationController->mirroringEnabled(checked);
}


QPixmap UBWebController::getScreenPixmap()
{
    QDesktopWidget *desktop = QApplication::desktop();
    // we capture the screen in which the mouse is.
    const QRect primaryScreenRect = desktop->screenGeometry(QCursor::pos());
    QCoreApplication::flush ();

    return QPixmap::grabWindow(desktop->winId(),
            primaryScreenRect.x(), primaryScreenRect.y(),
            primaryScreenRect.width(), primaryScreenRect.height());
}


void UBWebController::screenLayoutChanged()
{
    bool hasDisplay = (UBApplication::applicationController &&
            UBApplication::applicationController->displayManager() &&
            UBApplication::applicationController->displayManager()->hasDisplay());

    UBApplication::mainWindow->actionWebShowHideOnDisplay->setVisible(hasDisplay);
}


void UBWebController::closing()
{
    //NOOP
}


void UBWebController::adaptToolBar()
{
    bool highResolution = mMainWindow->width() > 1024;

    mMainWindow->actionWebReload->setVisible(highResolution);
    mMainWindow->actionStopLoading->setVisible(highResolution);

    if(mCurrentWebBrowser && (*mCurrentWebBrowser) )
        (*mCurrentWebBrowser)->adaptToolBar(highResolution);

}


void UBWebController::showTabAtTop(bool attop)
{
    if (mCurrentWebBrowser && (*mCurrentWebBrowser))
        (*mCurrentWebBrowser)->showTabAtTop(attop);
}

void UBWebController::captureoEmbed()
{
    if ( mCurrentWebBrowser  && (*mCurrentWebBrowser)
        && (*mCurrentWebBrowser)->currentTabWebView())
    {
        QWebView* webView = (*mCurrentWebBrowser)->currentTabWebView();
        QUrl currentUrl = webView->url();

        if (isOEmbedable(currentUrl))
        {
            UBGraphicsW3CWidgetItem * widget = UBApplication::boardController->activeScene()->addOEmbed(currentUrl);

            if(widget)
            {
                UBApplication::applicationController->showBoard();
                UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
            }
        }
    }
}


void UBWebController::captureEduMedia()
{
    if (mCurrentWebBrowser && (*mCurrentWebBrowser)
        && (*mCurrentWebBrowser)->currentTabWebView())
    {
        QWebView* webView = (*mCurrentWebBrowser)->currentTabWebView();
        QUrl currentUrl = webView->url();

        if (isEduMedia(currentUrl))
        {
            QWebElementCollection objects = webView->page()->currentFrame()->findAllElements("object");

            bool found = false;

            foreach(QWebElement object, objects)
            {
                foreach(QWebElement param, object.findAll("param"))
                {
                    if(param.attribute("name") == "flashvars")
                    {
                        QString value = param.attribute("value");
                        QString midValue;
                        QString langValue;
                        QString hostValue;

                        found = true;

                        QStringList flashVars = value.split("&");

                        foreach(QString flashVar, flashVars)
                        {
                            QStringList var = flashVar.split("=");

                            if (var.length() < 2)
                                break;

                            if (var.at(0) == "mid")
                                midValue = var.at(1);
                            else if (var.at(0) == "lang")
                                langValue = var.at(1);
                            else if (var.at(0) == "host")
                                hostValue = var.at(1);

                        }

                        if (midValue.length() > 0 && langValue.length() > 0 && hostValue.length() > 0)
                        {
                            QString swfUrl = "http://" + hostValue + "/" + langValue + "/fl/" + midValue;

                            UBApplication::boardController->downloadURL(QUrl(swfUrl));

                            UBApplication::applicationController->showBoard();
                            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);

                            return;
                        }
                    }
                }
            }
        }
    }
    else
    {
        UBApplication::showMessage("Cannot find any reference to eduMedia content");
    }
}


void UBWebController::getOEmbedProviderList()
{
    QUrl url(UBSettings::settings()->oEmbedJsLibraryUrl);

    mGetOEmbedProviderListRequest = new UBServerXMLHttpRequest(UBNetworkAccessManager::defaultAccessManager()); // destroyed in getOEmbedProviderListResponse

    connect(mGetOEmbedProviderListRequest, SIGNAL(finished(bool, const QByteArray&)), this, SLOT(getOEmbedProviderListResponse(bool, const QByteArray&)));

    mGetOEmbedProviderListRequest->get(url);
}


void UBWebController::getOEmbedProviderListResponse(bool success, const QByteArray& payload)
{

    if (success)
    {
        QString js(payload);
        QStringList lines = js.split("\n");

        foreach(QString line, lines)
        {
            if (line.contains("new OEmbedProvider("))
            {
                int start = line.indexOf("(");
                int end = line.indexOf(")", start);

                QString provider = line.mid(start, end - start);
                provider = provider.replace(" ", "");
                provider = provider.replace("\"", "");

                QStringList providerInfo = provider.split(",");

                if (providerInfo.length() >= 2)
                {
                    mOEmbedProviders << providerInfo.at(1);
                }
            }
        }

        qDebug() << "oEmbed provider list" << mOEmbedProviders;
    }

    mGetOEmbedProviderListRequest->deleteLater();
}


bool UBWebController::isOEmbedable(const QUrl& pUrl)
{
    QString urlAsString = pUrl.toString();

    foreach(QString provider, mOEmbedProviders)
    {
        if(urlAsString.contains(provider))
            return true;
    }

    return false;
}


bool UBWebController::isEduMedia(const QUrl& pUrl)
{
    QString urlAsString = pUrl.toString();

    if (urlAsString.contains("edumedia-sciences.com"))
    {
        return true;
    }

    return false;
}


void UBWebController::loadUrl(const QUrl& url)
{
    if (UBSettings::settings()->webUseExternalBrowser->get().toBool())
    {
        QDesktopServices::openUrl(url);
    }
    else
    {
        UBApplication::applicationController->showInternet();

        if (mCurrentWebBrowser && !(*mCurrentWebBrowser)) {
            (*mCurrentWebBrowser)->loadUrl(url);
        }
        else {
            (*mCurrentWebBrowser)->loadUrlInNewTab(url);
        }
    }
}


QWebView* UBWebController::createNewTab()
{
    if (mCurrentWebBrowser && !(*mCurrentWebBrowser))
    {
        UBApplication::applicationController->showInternet();
    }

    return (*mCurrentWebBrowser)->createNewTab();
}


void UBWebController::copy()
{
    if (mCurrentWebBrowser && (*mCurrentWebBrowser) && (*mCurrentWebBrowser)->currentTabWebView())
    {
        QWebView* webView = (*mCurrentWebBrowser)->currentTabWebView();
        QAction *act = webView->pageAction(QWebPage::Copy);
        if(act)
            act->trigger();
    }
}


void UBWebController::paste()
{
    if (mCurrentWebBrowser && (*mCurrentWebBrowser) && (*mCurrentWebBrowser)->currentTabWebView())
    {
        QWebView* webView = (*mCurrentWebBrowser)->currentTabWebView();
        QAction *act = webView->pageAction(QWebPage::Paste);
        if(act)
            act->trigger();
    }
}


void UBWebController::cut()
{
    if (mCurrentWebBrowser && (*mCurrentWebBrowser) && (*mCurrentWebBrowser)->currentTabWebView())
    {
        QWebView* webView = (*mCurrentWebBrowser)->currentTabWebView();
        QAction *act = webView->pageAction(QWebPage::Cut);
        if(act)
            act->trigger();
    }
}
