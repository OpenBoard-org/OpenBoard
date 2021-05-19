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
#include <QMenu>
#include <QXmlQuery>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QWebEngineHistory>
#include <QWebEngineHistoryItem>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>

#include "frameworks/UBPlatformUtils.h"

#include "UBWebController.h"
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
    connect(mMainWindow->actionOpenTutorial,SIGNAL(triggered()),this, SLOT(onOpenTutorial()));

    // note: do not delete profiles at application cleanup as they are still used by some web page
    mWebProfile = new QWebEngineProfile("OpenBoardWeb");
    QWebEngineSettings* settings = mWebProfile->settings();
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);

    mWidgetProfile = new QWebEngineProfile();
    settings = mWidgetProfile->settings();
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
}


UBWebController::~UBWebController()
{
    // NOOP
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
            mCurrentWebBrowser = new BrowserWindow(nullptr, mWebProfile);

            mMainWindow->addWebWidget(mCurrentWebBrowser);

            connect(mCurrentWebBrowser, SIGNAL(activeViewChange(QWidget*)), this, SLOT(setSourceWidget(QWidget*)));

            m_downloadManagerWidget.setParent(mCurrentWebBrowser, Qt::Tool);

            UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->webToolBar, mMainWindow->actionBoard, 32);
            QToolBar* navigationBar = mCurrentWebBrowser->createToolBar(mMainWindow->webToolBar);
            mMainWindow->webToolBar->insertWidget(mMainWindow->actionBoard, navigationBar);
            UBApplication::app()->decorateActionMenu(mMainWindow->actionMenu);

            showTabAtTop(UBSettings::settings()->appToolBarPositionedAtTop->get().toBool());

            adaptToolBar();

            mTrapFlashController = new UBTrapFlashController(mCurrentWebBrowser);

            connect(mCurrentWebBrowser, SIGNAL(activeViewPageChanged()), this, SLOT(activePageChanged()));
            connect(mCurrentWebBrowser->tabWidget(), &TabWidget::tabCreated, this, &UBWebController::tabCreated);

            // initialize the browser
            mCurrentWebBrowser->init();

            TabWidget* tabWidget = mCurrentWebBrowser->tabWidget();

            // signals are not emitted for first tab, so call explicitly
            setSourceWidget(mCurrentWebBrowser->currentTab());
            tabCreated(mCurrentWebBrowser->currentTab());

            // connect buttons
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

            mHistoryBackMenu = new QMenu(mMainWindow);
            connect(mHistoryBackMenu, SIGNAL(aboutToShow()),this, SLOT(aboutToShowBackMenu()));
            connect(mHistoryBackMenu, SIGNAL(triggered(QAction *)), this, SLOT(openActionUrl(QAction *)));

            for (QWidget* menuWidget : mMainWindow->actionWebBack->associatedWidgets())
            {
                QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

                if (tb && !tb->menu())
                {
                    tb->setMenu(mHistoryBackMenu);
                    tb->setStyleSheet("QToolButton::menu-indicator { subcontrol-position: bottom left; }");
                }
            }

            mHistoryForwardMenu = new QMenu(mMainWindow);
            connect(mHistoryForwardMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowForwardMenu()));
            connect(mHistoryForwardMenu, SIGNAL(triggered(QAction *)), this, SLOT(openActionUrl(QAction *)));

            for (QWidget* menuWidget : mMainWindow->actionWebForward->associatedWidgets())
            {
                QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

                if (tb && !tb->menu())
                {
                    tb->setMenu(mHistoryForwardMenu);
                    tb->setStyleSheet("QToolButton { padding-right: 8px; }");
                }
            }



            mCurrentWebBrowser->currentTab()->load(currentUrl);

            mCurrentWebBrowser->tabWidget()->tabBar()->show();
            // FIXME mCurrentWebBrowser->tabWidget()->lineEdits()->show();

            QObject::connect(
                mWebProfile, &QWebEngineProfile::downloadRequested,
                &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);

            connect(mMainWindow->actionWebTools, &QAction::triggered, [this](){
                mToolsCurrentPalette->setVisible(mMainWindow->actionWebTools->isChecked());
            });
        }

        UBApplication::applicationController->setMirrorSourceWidget(mCurrentWebBrowser->currentTab());
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

UBOEmbedParser *UBWebController::embedParser(const QWebEngineView* view) const
{
    return view->findChild<UBOEmbedParser*>("UBOEmbedParser");
}

void UBWebController::show()
{
    webBrowserInstance();
}

QWidget *UBWebController::controlView() const
{
    return mBrowserWidget;
}

QWebEngineProfile *UBWebController::widgetProfile() const
{
    return mWidgetProfile;
}

void UBWebController::injectScripts(QWebEngineView *view)
{
    // inject the QWebChannel interface and initialization script
    QFile js(":/qtwebchannel/qwebchannel.js");

    if (js.open(QIODevice::ReadOnly))
    {
        qDebug() << "Injecting qwebchannel.js";
        QString src = js.readAll();

        QFile asyncwrapper(UBPlatformUtils::applicationResourcesDirectory() + "/etc/asyncAPI.js");

        if (asyncwrapper.open(QIODevice::ReadOnly))
        {
            src += asyncwrapper.readAll();
        }

        QWebEngineScript script;
        script.setName("qwebchannel");
        script.setInjectionPoint(QWebEngineScript::DocumentCreation);
        script.setWorldId(QWebEngineScript::MainWorld);
        script.setSourceCode(src);
        view->page()->scripts().insert(script);
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
    if (mCurrentWebBrowser && mCurrentWebBrowser->currentTab())
    {
        WebView* view = mCurrentWebBrowser->currentTab();

        if (mTrapFlashController && view->page())
            mTrapFlashController->updateTrapFlashFromPage(view->page());

        mMainWindow->actionWebTrap->setChecked(false);

        QUrl latestUrl = view->url();

        UBOEmbedParser* parser = embedParser(view);
        UBApplication::mainWindow->actionWebOEmbed->setEnabled(parser ? parser->hasEmbeddedContent() : false);

        // And remove this line once the previous one is uncommented
        //UBApplication::mainWindow->actionWebOEmbed->setEnabled(isOEmbedable(latestUrl));
        UBApplication::mainWindow->actionEduMedia->setEnabled(isEduMedia(latestUrl));

        emit activeWebPageChanged(mCurrentWebBrowser->currentTab());
    }
}


QPixmap UBWebController::captureCurrentPage()
{
    QPixmap pix;

    if (mCurrentWebBrowser
            && mCurrentWebBrowser->currentTab()
            && mCurrentWebBrowser->currentTab()->page())
    {
        WebView* view = mCurrentWebBrowser->currentTab();
        QWebEnginePage* frame = mCurrentWebBrowser->currentTab()->page();
        QSize size = frame->contentsSize().toSize();

        qDebug() << size;

        /*
         * FIXME capturing a complete web page
         *
         * QWebEngine does not use a QPainter for rendering and only renders
         * those parts of a web page which are really visible. Therefore there
         * is no way to render a directly complete web page into a QPixmap.
         * The only solution I see is to use injected JavaScript to scroll
         * through the page and render the visible parts. Finally, we can
         * return to the initial scroll position.
         */
        // NOTE JS results are only asynchronous
//        QVariant top = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientTop");
//        QVariant left = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientLeft");
//        QVariant width = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientWidth");
//        QVariant height = frame->evaluateJavaScript("document.getElementsByTagName('body')[0].clientHeight");

//        QSize vieportSize = mCurrentWebBrowser->currentTab()->page()->viewportSize();
//        mCurrentWebBrowser->currentTab()->page()->setViewportSize(frame->contentsSize());
//        pix = QPixmap(size);
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
// FIXME        mCurrentWebBrowser->currentTab()->page()->setViewportSize(vieportSize);
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
        connect(mMainWindow->actionWebOEmbed, SIGNAL(triggered()), this, SLOT(createEmbeddedContentWidget()));
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
    if (mCurrentWebBrowser && mCurrentWebBrowser->currentTab())
    {}// FIXME    mCurrentWebBrowser->currentTab()->setIsTrapping(checked);
}


void UBWebController::captureWindow()
{
    QPixmap webPagePixmap = captureCurrentPage();

    if (!webPagePixmap.isNull())
        emit imageCaptured(webPagePixmap, true, mCurrentWebBrowser->currentTab()->url());
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
        emit imageCaptured(selectedPixmap, false, mCurrentWebBrowser->currentTab()->url());
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
    QUrl url = QUrl::fromUserInput(string);

    // finally for cases where the user just types in a hostname add http
    if (url.scheme().isEmpty())
        url = QUrl::fromEncoded("http://" + string.toUtf8(), QUrl::TolerantMode);

    return url;
}

void UBWebController::tabCreated(WebView *webView)
{
    // create and attach an UBOEmbedParser to the view
    UBOEmbedParser* parser = new UBOEmbedParser(webView);
    connect(parser, &UBOEmbedParser::parseResult, this, &UBWebController::onOEmbedParsed);
}

/**/
void UBWebController::captureEduMedia()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->currentTab())
    {
        WebView* webView = mCurrentWebBrowser->currentTab();
        QUrl currentUrl = webView->url();
/* FIXME DOM access not possible with QWebEngine, but eduMedia may be obsolete, anyway
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
    if (mCurrentWebBrowser && mCurrentWebBrowser->currentTab())
    {
        WebView* webView = mCurrentWebBrowser->currentTab();
        QAction *act = webView->pageAction(QWebEnginePage::Copy);
        if(act)
            act->trigger();
    }
}


void UBWebController::paste()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->currentTab())
    {
        WebView* webView = mCurrentWebBrowser->currentTab();
        QAction *act = webView->pageAction(QWebEnginePage::Paste);
        if(act)
            act->trigger();
    }
}


void UBWebController::cut()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->currentTab())
    {
        WebView* webView = mCurrentWebBrowser->currentTab();
        QAction *act = webView->pageAction(QWebEnginePage::Cut);
        if(act)
            act->trigger();
    }
}

void UBWebController::aboutToShowBackMenu()
{
    mHistoryBackMenu->clear();

    if (!mCurrentWebBrowser->currentTab())
        return;

    QWebEngineHistory *history = mCurrentWebBrowser->currentTab()->history();

    int historyCount = history->count();
    int historyLimit = history->backItems(historyCount).count() - UBSettings::settings()->historyLimit->get().toReal();
    if (historyLimit < 0)
        historyLimit = 0;

    for (int i = history->backItems(historyCount).count() - 1; i >= historyLimit; --i)
    {
        QWebEngineHistoryItem item = history->backItems(historyCount).at(i);

        QAction *action = new QAction(this);
        action->setData(-1*(historyCount-i-1));

        // TODO fetch icon or keep a cache somewhere
//        if (!item.iconUrl().isEmpty())
//            action->setIcon(item.icon());
        action->setText(item.title().isEmpty() ? item.url().toString() : item.title());
        mHistoryBackMenu->addAction(action);
    }
}

void UBWebController::aboutToShowForwardMenu()
{
    mHistoryForwardMenu->clear();

    if (!mCurrentWebBrowser->currentTab())
        return;

    QWebEngineHistory *history = mCurrentWebBrowser->currentTab()->history();
    int historyCount = history->count();

    int historyLimit = history->forwardItems(historyCount).count();
    if (historyLimit > UBSettings::settings()->historyLimit->get().toReal())
        historyLimit = UBSettings::settings()->historyLimit->get().toReal();

    for (int i = 0; i < historyLimit; ++i)
    {
        QWebEngineHistoryItem item = history->forwardItems(historyCount).at(i);

        QAction *action = new QAction(this);
        action->setData(historyCount-i);

        // TODO fetch icon or keep a cache somewhere
//        if (!item.iconUrl().isEmpty())
//            action->setIcon(item.icon());
        action->setText(item.title().isEmpty() ? item.url().toString() : item.title());
        mHistoryForwardMenu->addAction(action);
    }
}

void UBWebController::openActionUrl(QAction *action)
{
    QWebEngineHistory *history = mCurrentWebBrowser->currentTab()->history();

    int offset = action->data().toInt();

    if (offset < 0)
        history->goToItem(history->backItems(-1*offset).first());
    else if (offset > 0)
        history->goToItem(history->forwardItems(history->count() - offset + 1).back());
 }

void UBWebController::onOEmbedParsed(QWebEngineView *view, bool hasEmbeddedContent)
{
    // check: is this for current tab?
    if (view == mBrowserWidget)
    {
        // enable/disable embed button
        UBApplication::mainWindow->actionWebOEmbed->setEnabled(hasEmbeddedContent);
    }
}

void UBWebController::onOpenTutorial()
{
    loadUrl(QUrl(UBSettings::settings()->tutorialUrl->get().toString()));
}

void UBWebController::createEmbeddedContentWidget()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->currentTab())
    {
        WebView* webView = mCurrentWebBrowser->currentTab();
        UBOEmbedParser* parser = embedParser(webView);

        // FIXME workaround: just use the first entry in contents
        if (parser && parser->hasEmbeddedContent()) {
            UBOEmbedContent content = parser->embeddedContent()[0];
            qDebug() << "Embedding" << content.title();
            UBGraphicsW3CWidgetItem* widget = UBApplication::boardController->activeScene()->addOEmbed(content);

            if (widget)
            {
                UBApplication::applicationController->showBoard();
                UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
            }
        }
    }
}
