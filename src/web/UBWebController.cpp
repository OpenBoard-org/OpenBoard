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




#include <QtGui>
#include <QMenu>
#include <QWebEngineCookieStore>
#include <QWebEngineHistory>
#include <QWebEngineHistoryItem>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QtWebEngineWidgetsVersion>

#include "frameworks/UBPlatformUtils.h"

#include "UBWebController.h"
#include "UBEmbedController.h"

#include "web/simplebrowser/browserwindow.h"
#include "web/simplebrowser/webview.h"
#include "web/simplebrowser/tabwidget.h"

#include "network/UBCookieJar.h"
#include "network/UBNetworkAccessManager.h"

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
    , mCurrentWebBrowser(nullptr)
    , mBrowserWidget(nullptr)
    , mEmbedController(nullptr)
    , mToolsCurrentPalette(nullptr)
    , mToolsPalettePositionned(false)
    , mDownloadViewIsVisible(false)
{
    connect(mMainWindow->actionOpenTutorial, SIGNAL(triggered()), this, SLOT(onOpenTutorial()));

    bool privateBrowsing = UBSettings::settings()->webPrivateBrowsing->get().toBool();
    qDebug() << "Private browsing" << privateBrowsing;

    if (privateBrowsing)
    {
        // create off-the-record profile that leaves no record on the local machine, and has no persistent data or cache
        mWebProfile = new QWebEngineProfile();
    }
    else
    {
        mWebProfile = QWebEngineProfile::defaultProfile();
        mWebProfile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    }

    // compute a system specific user agent string
    QString originalUserAgent = mWebProfile->httpUserAgent();
    QRegularExpression exp("\\(([^;]*);([^)]*)\\)");

    QString p1;
    QString p2;

    QRegularExpressionMatch match = exp.match(originalUserAgent);

    if (match.hasMatch())
    {
        p1 = match.captured(1);
        p2 = match.captured(2);
    }

    QString userAgent = UBSettings::settings()->alternativeUserAgent->get().toString();
    userAgent = userAgent.arg(p1).arg(p2);

    mInterceptor = new UBUserAgentInterceptor(userAgent.toUtf8(), mWebProfile);

#if QTWEBENGINEWIDGETS_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    mWebProfile->setUrlRequestInterceptor(mInterceptor);
#else
    mWebProfile->setRequestInterceptor(mInterceptor);
#endif

    QWebEngineSettings* settings = mWebProfile->settings();
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);

    // install cookie filter
    QWebEngineCookieStore* cookieStore = mWebProfile->cookieStore();

    QByteArray value = UBSettings::settings()->webCookiePolicy->get().toByteArray();
    QMetaEnum cookiePolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("CookiePolicy"));
    int enumOrdinal = cookiePolicyEnum.keyToValue(value);
    CookiePolicy cookiePolicy = enumOrdinal == -1 ?
                DenyThirdParty :
                static_cast<CookiePolicy>(enumOrdinal);

    qDebug() << "Using cookie policy" << value;
    cookieStore->setCookieFilter(
                [cookiePolicy](const QWebEngineCookieStore::FilterRequest &request)
    {
        switch (cookiePolicy) {
        case DenyAll:
            return false;

        case DenyThirdParty:
            return !request.thirdParty;

        case AcceptAll:
            return true;
        }

        return false;
    }
    );

    // synchronize with QNetworkAccessManager
    QNetworkCookieJar* jar = UBNetworkAccessManager::defaultAccessManager()->cookieJar();

    connect(cookieStore, &QWebEngineCookieStore::cookieAdded, [jar](const QNetworkCookie &cookie){
        jar->insertCookie(cookie);
    });
    connect(cookieStore, &QWebEngineCookieStore::cookieRemoved, [jar](const QNetworkCookie &cookie){
        jar->deleteCookie(cookie);
    });

    // remember settings for cleanup
    cookieAutoDelete = UBSettings::settings()->webCookieAutoDelete->get().toBool();
    cookieKeepDomains = UBSettings::settings()->webCookieKeepDomains->get().toStringList();
}

UBWebController::~UBWebController()
{
    if (cookieAutoDelete)
    {
        QWebEngineCookieStore* cookieStore = mWebProfile->cookieStore();

        if (cookieKeepDomains.empty())
        {
            cookieStore->deleteAllCookies();
        }
        else
        {
            UBCookieJar* jar = dynamic_cast<UBCookieJar*>(UBNetworkAccessManager::defaultAccessManager()->cookieJar());

            if (jar)
            {
                for (const QNetworkCookie& cookie : jar->cookieList())
                {
                    QString domain = cookie.domain();
                    bool keep = false;

                    for (QString keepDomain : cookieKeepDomains)
                    {
                        if (keepDomain.startsWith('.'))
                        {
                            // check for suffix match
                            keep = domain.endsWith(keepDomain);
                        }
                        else
                        {
                            // check for exact match
                            keep = domain == keepDomain;
                        }

                        if (keep)
                        {
                            break;
                        }
                    }

                    if (!keep)
                    {
                        cookieStore->deleteCookie(cookie);
                    }
                }
            }
        }
    }
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

            mDownloadManagerWidget.setParent(mCurrentWebBrowser, Qt::Tool);

            UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->webToolBar, mMainWindow->actionBoard, 32);
            QToolBar* navigationBar = mCurrentWebBrowser->createToolBar(mMainWindow->webToolBar);
            mMainWindow->webToolBar->insertWidget(mMainWindow->actionBoard, navigationBar);
            UBApplication::app()->decorateActionMenu(mMainWindow->actionMenu);

            showTabAtTop(UBSettings::settings()->appToolBarPositionedAtTop->get().toBool());

            adaptToolBar();

            mEmbedController = new UBEmbedController(mCurrentWebBrowser);

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

            connect(mMainWindow->actionWebBigger, SIGNAL(triggered()), mCurrentWebBrowser, SLOT(zoomIn()));
            connect(mMainWindow->actionWebSmaller, SIGNAL(triggered()), mCurrentWebBrowser, SLOT(zoomOut()));

            mHistoryBackMenu = new QMenu(mMainWindow);
            connect(mHistoryBackMenu, SIGNAL(aboutToShow()),this, SLOT(aboutToShowBackMenu()));
            connect(mHistoryBackMenu, SIGNAL(triggered(QAction *)), this, SLOT(openActionUrl(QAction *)));

            // setup history drop down menus
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

            QObject::connect(
                mWebProfile, &QWebEngineProfile::downloadRequested,
                &mDownloadManagerWidget, &DownloadManagerWidget::downloadRequested);

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
    {
        mDownloadManagerWidget.show();
    }
}

UBEmbedParser *UBWebController::embedParser(const QWebEngineView* view) const
{
    return view->findChild<UBEmbedParser*>("UBEmbedParser");
}

void UBWebController::show()
{
    webBrowserInstance();
}

QWidget *UBWebController::controlView() const
{
    return mBrowserWidget;
}

QWebEngineProfile *UBWebController::webProfile() const
{
    return mWebProfile;
}

QList<UBEmbedContent> UBWebController::getEmbeddedContent(const QWebEngineView *view)
{
    UBEmbedParser* parser = embedParser(view);

    if (parser)
    {
        return parser->embeddedContent();
    }

    return QList<UBEmbedContent>();
}

BrowserWindow* UBWebController::browserWindow() const
{
    return mCurrentWebBrowser;
}

QWebEnginePage::PermissionPolicy UBWebController::hasFeaturePermission(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
{
    QPair<QUrl,QWebEnginePage::Feature> featurePermission(securityOrigin, feature);

    if (mFeaturePermissions.contains(featurePermission))
    {
        return mFeaturePermissions[featurePermission];
    }

    return QWebEnginePage::PermissionUnknown;
}

void UBWebController::setFeaturePermission(const QUrl &securityOrigin, QWebEnginePage::Feature feature, QWebEnginePage::PermissionPolicy policy)
{
    QPair<QUrl,QWebEnginePage::Feature> featurePermission(securityOrigin, feature);
    mFeaturePermissions[featurePermission] = policy;
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


void UBWebController::trap()
{
    mEmbedController->showEmbedDialog();
    activePageChanged();
}

void UBWebController::activePageChanged()
{
    if (mCurrentWebBrowser && mCurrentWebBrowser->currentTab())
    {
        WebView* view = mCurrentWebBrowser->currentTab();

        if (mEmbedController)
            mEmbedController->updateEmbeddableContentFromView(view);

        emit activeWebPageChanged(mCurrentWebBrowser->currentTab());
    }
}

void UBWebController::captureCurrentPage()
{
    QPixmap* pix;

    if (mCurrentWebBrowser
            && mCurrentWebBrowser->currentTab()
            && mCurrentWebBrowser->currentTab()->page())
    {
        WebView* view = mCurrentWebBrowser->currentTab();
        QWebEnginePage* page = view->page();
        QSize size = page->contentsSize().toSize();
        QPoint scrollPosition = page->scrollPosition().toPoint();
        QSize viewportSize = view->size();

        // pix is deleted at the final run of captureStripe
        pix = new QPixmap(size);
        QPointF pos(0, 0);

        // capture complete web page in stripes, starting at top left
        captureStripe(pos, viewportSize, pix, scrollPosition);
    }
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

        connect(mMainWindow->actionCaptureWebContent, SIGNAL(triggered()), this, SLOT(trap()));
        connect(mMainWindow->actionWebCustomCapture, SIGNAL(triggered()), this, SLOT(customCapture()));
        connect(mMainWindow->actionWebWindowCapture, SIGNAL(triggered()), this, SLOT(captureWindow()));
        connect(mMainWindow->actionWebShowHideOnDisplay, SIGNAL(toggled(bool)), this, SLOT(toogleMirroring(bool)));

        mToolsCurrentPalette->hide();
        mToolsCurrentPalette->adjustSizeAndPosition();

        if (controlView())
        {
            int left = controlView()->width() - 20 - mToolsCurrentPalette->width();
            int top = (controlView()->height() - mToolsCurrentPalette->height()) / 2;
            mToolsCurrentPalette->setCustomPosition(true);
            mToolsCurrentPalette->move(left, top);
        }

        mMainWindow->actionWebTools->trigger();
    }
}


void UBWebController::captureWindow()
{
    captureCurrentPage();
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
    return UBApplication::displayManager->grab(ScreenRole::Control);
}


void UBWebController::screenLayoutChanged()
{
    bool hasDisplay = (UBApplication::applicationController &&
                       UBApplication::displayManager &&
                       UBApplication::displayManager->hasDisplay());

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
}


void UBWebController::showTabAtTop(bool attop)
{
    if (mCurrentWebBrowser)
        mCurrentWebBrowser->tabWidget()->setTabPosition(attop ? QTabWidget::North : QTabWidget::South);
}


QUrl UBWebController::guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegularExpression test(QRegularExpression::anchoredPattern("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    QRegularExpressionMatch match = test.match(urlStr);
    bool hasSchema = match.hasMatch();
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
    // create and attach an UBEmbedParser to the view
    if (!embedParser(webView))
    {
        UBEmbedParser* parser = new UBEmbedParser(webView);
        connect(parser, &UBEmbedParser::parseResult, this, &UBWebController::onEmbedParsed);
    }
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

void UBWebController::onEmbedParsed(QWebEngineView *view, bool hasEmbeddedContent)
{
    // check: is this for current tab?
    if (view == mBrowserWidget)
    {
        // enable/disable embed button
        UBApplication::mainWindow->actionWebOEmbed->setEnabled(hasEmbeddedContent);

        if (mEmbedController)
        {
            mEmbedController->updateEmbeddableContentFromView(view);
        }
    }
}

void UBWebController::onOpenTutorial()
{
    loadUrl(QUrl(UBSettings::settings()->tutorialUrl->get().toString()));
}

void UBWebController::captureStripe(QPointF pos, QSize size, QPixmap* pix, QPointF scrollPosition)
{
    WebView* view = mCurrentWebBrowser->currentTab();
    QString scrollto = QString("window.scrollTo(%1,%2)").arg(pos.x()).arg(pos.y());
    view->page()->runJavaScript(scrollto, [this,pos,size,pix,scrollPosition](const QVariant&){
        // we need some time for rendering - there is no signal when finished, so just wait
        QTimer::singleShot(100, [this,pos,size,pix,scrollPosition](){
            WebView* view = mCurrentWebBrowser->currentTab();
            QPixmap stripe(size);

            {
                // render stripe, local block to release QPainter
                QPainter p(&stripe);
                view->render(&p);
            }
            {
                // copy rendered stripe to pix
                QPointF actualPos = view->page()->scrollPosition();
                QRectF target(actualPos, size);
                QPainter p(pix);
                p.drawPixmap(target.toRect(), stripe);
            }

            QPointF nextPos = pos + QPointF(0, size.height() / view->zoomFactor());

            if (nextPos.y() < pix->height() / view->zoomFactor())
            {
                // capture next stripe
                captureStripe(nextPos, size, pix, scrollPosition);
            }
            else
            {
                QPixmap captured = *pix;
                // allocated at captureCurrentPage()
                delete pix;
                emit imageCaptured(captured, true, view->url());

                // scroll back to initial position
                QString scrollto = QString("window.scrollTo(%1,%2)").arg(scrollPosition.x()).arg(scrollPosition.y());
                view->page()->runJavaScript(scrollto);
            }
        });
    });
}

UBUserAgentInterceptor::UBUserAgentInterceptor(const QByteArray &alternativeUserAgent, QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent), mAlternativeUserAgent(alternativeUserAgent)
{
    QStringList userAgentDomains = UBSettings::settings()->alternativeUserAgentDomains->get().toStringList();

    // convert patterns to regular expressions
    for (QString& pattern : userAgentDomains) {
        // escape dots
        pattern.replace(".", "\\.");

        // replace wildcards
        pattern.replace("*", "\\w*");
    }

    // set patterns in brachets, join with | and anchor at end of string with $
    QString domains = "(" + userAgentDomains.join(")|(") + ")$";

    mDomainMatcher.setPattern(domains);

    // handle invalid pattern
    if (!mDomainMatcher.isValid())
    {
        // this works!
        qDebug() << "Wrong pattern syntax " << domains << "fallback to google.*";
        mDomainMatcher.setPattern("google\\.\\w*$");
    }

    mDomainMatcher.optimize();
}

void UBUserAgentInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    QUrl url = info.requestUrl();

    if (mDomainMatcher.match(url.host()).hasMatch())
    {
        info.setHttpHeader("User-Agent", mAlternativeUserAgent);
    }
}
