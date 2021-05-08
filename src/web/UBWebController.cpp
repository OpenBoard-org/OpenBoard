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




#include <QtGui>
#include <QDomDocument>
#include <QXmlQuery>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

#include "frameworks/UBPlatformUtils.h"

#include "UBWebController.h"
#include "UBOEmbedParser.h"
#include "UBTrapFlashController.h"

#include "web/simplebrowser/browserwindow.h"
#include "web/simplebrowser/webview.h"
#include "web/simplebrowser/tabwidget.h"

#include "network/UBServerXMLHttpRequest.h"
#include "network/UBNetworkAccessManager.h"

#include "gui/UBWidgetMirror.h"
#include "gui/UBMainWindow.h"
#include "gui/UBWebToolsPalette.h"
#include "gui/UBKeyboardPalette.h"

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"

#include "domain/UBGraphicsScene.h"

#include "desktop/UBCustomCaptureWindow.h"
#include "board/UBBoardPaletteManager.h"

#include "core/memcheck.h"

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
    connect(&mOEmbedParser, SIGNAL(oembedParsed(QVector<sOEmbedContent>)), this, SLOT(onOEmbedParsed(QVector<sOEmbedContent>)));

    // TODO : Comment the next line to continue the Youtube button bugfix
    initialiazemOEmbedProviders();

    connect(mMainWindow->actionOpenTutorial,SIGNAL(triggered()),this, SLOT(onOpenTutorial()));
}


UBWebController::~UBWebController()
{
    // NOOP
}

void UBWebController::onOpenTutorial()
{
    loadUrl(QUrl(UBSettings::settings()->tutorialUrl->get().toString()));
}

void UBWebController::initialiazemOEmbedProviders()
{
    mOEmbedProviders << "5min.com";
    mOEmbedProviders << "amazon.com";
    mOEmbedProviders << "flickr";
    mOEmbedProviders << "video.google.";
    mOEmbedProviders << "hulu.com";
    mOEmbedProviders << "imdb.com";
    mOEmbedProviders << "metacafe.com";
    mOEmbedProviders << "qik.com";
    mOEmbedProviders << "slideshare";
    mOEmbedProviders << "twitpic.com";
    mOEmbedProviders << "viddler.com";
    mOEmbedProviders << "vimeo.com";
    mOEmbedProviders << "wikipedia.org";
    mOEmbedProviders << "wordpress.com";
    mOEmbedProviders << "youtube.com";
}

void UBWebController::webBrowserInstance()
{
    QString webHomePage = UBSettings::settings()->webHomePage->get().toString();
    QUrl currentUrl = guessUrlFromString(webHomePage);

    if (UBSettings::settings()->webUseExternalBrowser->get().toBool())
    {
        QDesktopServices::openUrl(currentUrl);
    }
    else
    {
        if (!mCurrentWebBrowser)
        {
            QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
            QWebEngineProfile *profile = QWebEngineProfile::defaultProfile(); // FIXME
            profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
            mCurrentWebBrowser = new BrowserWindow(nullptr, profile);

            mMainWindow->addWebWidget(mCurrentWebBrowser);

            connect(mCurrentWebBrowser, SIGNAL(activeViewChange(QWidget*)), this, SLOT(setSourceWidget(QWidget*)));

            m_downloadManagerWidget.setParent(mCurrentWebBrowser, Qt::Tool);

            UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->webToolBar, mMainWindow->actionBoard, 32);
            UBApplication::app()->decorateActionMenu(mMainWindow->actionMenu);

            showTabAtTop(UBSettings::settings()->appToolBarPositionedAtTop->get().toBool());

            adaptToolBar();

            mTrapFlashController = new UBTrapFlashController(mCurrentWebBrowser);

            connect(mCurrentWebBrowser, SIGNAL(activeViewPageChanged()), this, SLOT(activePageChanged()));

            // connect buttons
            TabWidget* tabWidget = mCurrentWebBrowser->tabWidget();

            connect(mMainWindow->actionWebBack, &QAction::triggered, [tabWidget]() {
                tabWidget->triggerWebPageAction(QWebEnginePage::Back);
            });

            connect(mMainWindow->actionWebForward, &QAction::triggered, [tabWidget]() {
                tabWidget->triggerWebPageAction(QWebEnginePage::Forward);
            });

            connect(mMainWindow->actionWebReload, &QAction::triggered, [tabWidget]() {
                tabWidget->triggerWebPageAction(QWebEnginePage::Reload);
            });

            connect(mMainWindow->actionStopLoading, &QAction::triggered, [tabWidget]() {
                tabWidget->triggerWebPageAction(QWebEnginePage::Stop);
            });

            connect(mMainWindow->actionHome, &QAction::triggered, [this, currentUrl](){
                mCurrentWebBrowser->currentTab()->load(currentUrl);
            });
//            connect(mMainWindow->actionBookmarks, SIGNAL(triggered()), this , SLOT(bookmarks()));
//            connect(mMainWindow->actionAddBookmark, SIGNAL(triggered()), this , SLOT(addBookmark()));
            connect(mMainWindow->actionWebBigger, SIGNAL(triggered()), mCurrentWebBrowser, SLOT(zoomIn()));
            connect(mMainWindow->actionWebSmaller, SIGNAL(triggered()), mCurrentWebBrowser, SLOT(zoomOut()));


            mCurrentWebBrowser->currentTab()->load(currentUrl);

            mCurrentWebBrowser->tabWidget()->tabBar()->show();
            // FIXME mCurrentWebBrowser->tabWidget()->lineEdits()->show();

            QObject::connect(
                QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested,
                &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);
        }

        UBApplication::applicationController->setMirrorSourceWidget(mCurrentWebBrowser->tabWidget()->currentWebView());
        mMainWindow->switchToWebWidget();

        setupPalettes();
        screenLayoutChanged();

        bool mirroring = UBSettings::settings()->webShowPageImmediatelyOnMirroredScreen->get().toBool();
        UBApplication::mainWindow->actionWebShowHideOnDisplay->setChecked(mirroring);
        mToolsCurrentPalette->show();
    }

    if (mDownloadViewIsVisible)
        m_downloadManagerWidget.show();
}

void UBWebController::show()
{
    webBrowserInstance();
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
    if (mCurrentWebBrowser && mCurrentWebBrowser->tabWidget()->currentWebView())
    {
        if (mTrapFlashController && mCurrentWebBrowser->tabWidget()->currentWebView()->page())
        {} // FIXME mTrapFlashController->updateTrapFlashFromPage(mCurrentWebBrowser->tabWidget()->currentWebView()->page()->currentFrame());

        mMainWindow->actionWebTrap->setChecked(false);

        QUrl latestUrl = mCurrentWebBrowser->tabWidget()->currentWebView()->url();

        // TODO : Uncomment the next line to continue the youtube button bugfix
        //UBApplication::mainWindow->actionWebOEmbed->setEnabled(hasEmbeddedContent());
        // And remove this line once the previous one is uncommented
        UBApplication::mainWindow->actionWebOEmbed->setEnabled(isOEmbedable(latestUrl));
        UBApplication::mainWindow->actionEduMedia->setEnabled(isEduMedia(latestUrl));

        emit activeWebPageChanged(mCurrentWebBrowser->tabWidget()->currentWebView());
    }
}

// FIXME can only give ansynchronous result, but line 208 is the only line calling this
/*
bool UBWebController::hasEmbeddedContent()
{
    bool bHasContent = false;
    if(mCurrentWebBrowser){
        QString html = mCurrentWebBrowser->tabWidget()->currentWebView()->page()->toHtml();

        // search the presence of "+oembed"
        QString query = "\\+oembed([^>]*)>";
        QRegExp exp(query);
        exp.indexIn(html);
        QStringList results = exp.capturedTexts();
        if(2 <= results.size() && "" != results.at(1)){
            // An embedded content has been found, no need to check the other ones
            bHasContent = true;
        }else{
            QList<QUrl> contentUrls;
            lookForEmbedContent(&html, "embed", "src", &contentUrls);
            lookForEmbedContent(&html, "video", "src", &contentUrls);
            lookForEmbedContent(&html, "object", "data", &contentUrls);

            // TODO: check the hidden iFrame

            if(!contentUrls.empty()){
                bHasContent = true;
            }
        }
    }

    return bHasContent;
}
*/
QPixmap UBWebController::captureCurrentPage()
{
    QPixmap pix;

    if (mCurrentWebBrowser
            && mCurrentWebBrowser->tabWidget()->currentWebView()
            && mCurrentWebBrowser->tabWidget()->currentWebView()->page())
    {
        WebView* view = mCurrentWebBrowser->tabWidget()->currentWebView();
        QWebEnginePage* frame = mCurrentWebBrowser->tabWidget()->currentWebView()->page();
        QSizeF size = frame->contentsSize();

        qDebug() << size;

        // FIXME JS results are only asynchronous
//        QVariant top = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientTop");
//        QVariant left = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientLeft");
//        QVariant width = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientWidth");
//        QVariant height = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientHeight");

//        QSize vieportSize = mCurrentWebBrowser->tabWidget()->currentWebView()->page()->viewportSize();
//        mCurrentWebBrowser->tabWidget()->currentWebView()->page()->setViewportSize(frame->contentsSize());
        pix = QPixmap(view->geometry().width(), view->geometry().height());

        {
            QPainter p(&pix);
            view->render(&p);
        }
/* FIXME this cutting just removed
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
*/

// FIXME        mCurrentWebBrowser->tabWidget()->currentWebView()->page()->setViewportSize(vieportSize);
    }

    return pix;
}


void UBWebController::setupPalettes()
{
    if(!mToolsCurrentPalette)
    {
        mToolsCurrentPalette = new UBWebToolsPalette(UBApplication::mainWindow);
        UBApplication::boardController->paletteManager()->setCurrentWebToolsPalette(mToolsCurrentPalette);
#ifndef Q_OS_WIN
        if (UBPlatformUtils::hasVirtualKeyboard() && UBApplication::boardController->paletteManager()->mKeyboardPalette)
            connect(UBApplication::boardController->paletteManager()->mKeyboardPalette, SIGNAL(closed()),
                    UBApplication::boardController->paletteManager()->mKeyboardPalette, SLOT(onDeactivated()));
#endif

        connect(mMainWindow->actionWebTrapFlash, SIGNAL(triggered()), this, SLOT(trapFlash()));
        connect(mMainWindow->actionWebCustomCapture, SIGNAL(triggered()), this, SLOT(customCapture()));
        connect(mMainWindow->actionWebWindowCapture, SIGNAL(triggered()), this, SLOT(captureWindow()));
        connect(mMainWindow->actionWebOEmbed, SIGNAL(triggered()), this, SLOT(captureoEmbed()));
        connect(mMainWindow->actionEduMedia, SIGNAL(triggered()), this, SLOT(captureEduMedia()));

        connect(mMainWindow->actionWebShowHideOnDisplay, SIGNAL(toggled(bool)), this, SLOT(toogleMirroring(bool)));
        connect(mMainWindow->actionWebTrap, SIGNAL(toggled(bool)), this, SLOT(toggleWebTrap(bool)));

        mToolsCurrentPalette->hide();
        mToolsCurrentPalette->adjustSizeAndPosition();

        if (controlView()){
            int left = controlView()->width() - 20 - mToolsCurrentPalette->width();
            int top = (controlView()->height() - mToolsCurrentPalette->height()) / 2;
            mToolsCurrentPalette->setCustomPosition(true);
            mToolsCurrentPalette->move(left, top);
        }
        mMainWindow->actionWebTools->trigger();
    }
}


void UBWebController::toggleWebTrap(bool checked)
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->tabWidget()->currentWebView())
    {}// FIXME    mCurrentWebBrowser->tabWidget()->currentWebView()->setIsTrapping(checked);
}


void UBWebController::captureWindow()
{
    QPixmap webPagePixmap = captureCurrentPage();

    if (!webPagePixmap.isNull())
        emit imageCaptured(webPagePixmap, true, mCurrentWebBrowser->tabWidget()->currentWebView()->url());
}


void UBWebController::customCapture()
{
    mToolsCurrentPalette->setVisible(false);
    qApp->processEvents();

    UBCustomCaptureWindow customCaptureWindow(mCurrentWebBrowser);

    customCaptureWindow.show();

    if (customCaptureWindow.execute(getScreenPixmap()) == QDialog::Accepted)
    {
        QPixmap selectedPixmap = customCaptureWindow.getSelectedPixmap();
        emit imageCaptured(selectedPixmap, false, mCurrentWebBrowser->tabWidget()->currentWebView()->url());
    }

    mToolsCurrentPalette->setVisible(true);
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

    if(mCurrentWebBrowser )
    {} // FIXME enables search on wide monitors mCurrentWebBrowser->adaptToolBar(highResolution);

}


void UBWebController::showTabAtTop(bool attop)
{
    if (mCurrentWebBrowser)
        mCurrentWebBrowser->tabWidget()->setTabPosition(attop ? QTabWidget::North : QTabWidget::South);
}

void UBWebController::captureoEmbed()
{
    if ( mCurrentWebBrowser && mCurrentWebBrowser->tabWidget()->currentWebView()){
        // TODO : Uncomment the next lines to continue the youtube button bugfix
        //    getEmbeddableContent();

        // And comment from here

        WebView* webView = mCurrentWebBrowser->tabWidget()->currentWebView();
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
        // --> Until here
    }
}

void UBWebController::lookForEmbedContent(QString* pHtml, QString tag, QString attribute, QList<QUrl> *pList)
{
    if(NULL != pHtml && NULL != pList){
        QVector<QString> urlsFound;
        // Check for <embed> content
        QRegExp exp(QString("<%0(.*)").arg(tag));
        exp.indexIn(*pHtml);
        QStringList strl = exp.capturedTexts();
        if(2 <= strl.size() && strl.at(1) != ""){
            // Here we call this regular expression:
            // src\s?=\s?['"]([^'"]*)['"]
            // It says: give me all characters that are after src=" (or src = ")
            QRegExp src(QString("%0\\s?=\\s?['\"]([^'\"]*)['\"]").arg(attribute));
            for(int i=1; i<strl.size(); i++){
                src.indexIn(strl.at(i));
                QStringList urls = src.capturedTexts();
                if(2 <= urls.size() && urls.at(1) != "" && !urlsFound.contains(urls.at(1))){
                    urlsFound << urls.at(1);
                    pList->append(QUrl(urls.at(1)));
                }
            }
        }
    }
}

void UBWebController::checkForOEmbed(QString *pHtml)
{
    mOEmbedParser.parse(*pHtml);
}

QUrl UBWebController::guessUrlFromString(const QString &string)
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

/* currently not needed
void UBWebController::getEmbeddableContent()
{
    // Get the source code of the page
    if(mCurrentWebBrowser){
        QNetworkAccessManager* pNam = mCurrentWebBrowser->tabWidget()->currentWebView()->webPage()->networkAccessManager();
        if(NULL != pNam){
            QString html = mCurrentWebBrowser->tabWidget()->currentWebView()->webPage()->mainFrame()->toHtml();
            mOEmbedParser.setNetworkAccessManager(pNam);

            // First, we have to check if there is some oembed content
            checkForOEmbed(&html);

            // Note: The other contents will be verified once the oembed ones have been checked
        }
    }
}
*/
void UBWebController::captureEduMedia()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->tabWidget()->currentWebView())
    {
        WebView* webView = mCurrentWebBrowser->tabWidget()->currentWebView();
        QUrl currentUrl = webView->url();
/* FIXME DOM access not possible with QWebEngine
        if (isEduMedia(currentUrl))
        {
            QWebElementCollection objects = webView->page()->currentFrame()->findAllElements("object");

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
        }*/
    }
    else
    {
        UBApplication::showMessage("Cannot find any reference to eduMedia content");
    }
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
    UBApplication::applicationController->showInternet();
    if (UBSettings::settings()->webUseExternalBrowser->get().toBool())
    {
        QDesktopServices::openUrl(url);
    }
    else
    {
        WebView* view = mCurrentWebBrowser->tabWidget()->createTab();
        view->load(url);
    }
}


WebView* UBWebController::createNewTab()
{
    if (mCurrentWebBrowser)
        UBApplication::applicationController->showInternet();

    return mCurrentWebBrowser->tabWidget()->createTab();
}


void UBWebController::copy()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->tabWidget()->currentWebView())
    {
        WebView* webView = mCurrentWebBrowser->tabWidget()->currentWebView();
        QAction *act = webView->pageAction(QWebEnginePage::Copy);
        if(act)
            act->trigger();
    }
}


void UBWebController::paste()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->tabWidget()->currentWebView())
    {
        WebView* webView = mCurrentWebBrowser->tabWidget()->currentWebView();
        QAction *act = webView->pageAction(QWebEnginePage::Paste);
        if(act)
            act->trigger();
    }
}


void UBWebController::cut()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->tabWidget()->currentWebView())
    {
        WebView* webView = mCurrentWebBrowser->tabWidget()->currentWebView();
        QAction *act = webView->pageAction(QWebEnginePage::Cut);
        if(act)
            act->trigger();
    }
}

void UBWebController::onOEmbedParsed(QVector<sOEmbedContent> contents)
{
    QList<QUrl> urls;

    foreach(sOEmbedContent cnt, contents){
        urls << QUrl(cnt.url);
    }

    // TODO : Implement this
    //lookForEmbedContent(&html, "embed", "src", &urls);
    //lookForEmbedContent(&html, "video", "src", &contentUrls);
    //lookForEmbedContent(&html, "object", "data", &contentUrls);

    // TODO: check the hidden iFrame

    if(!urls.empty()){
        QUrl contentUrl;    // The selected content url

        if(1 == urls.size()){
            contentUrl = urls.at(0);
        }else{
            // TODO : Display a dialog box asking the user which content to get and set contentUrl to the selected content

        }

        UBGraphicsW3CWidgetItem * widget = UBApplication::boardController->activeScene()->addOEmbed(contentUrl);

        if(widget)
        {
            UBApplication::applicationController->showBoard();
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
    }
}
