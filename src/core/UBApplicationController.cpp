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



#include "UBApplicationController.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBVersion.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBDocumentManager.h"
#include "core/UBDisplayManager.h"


#include "board/UBBoardView.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBDrawingController.h"


#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "domain/UBGraphicsWidgetItem.h"

#include "desktop/UBDesktopPalette.h"
#include "desktop/UBDesktopAnnotationController.h"

#include "web/UBWebController.h"

#include "gui/UBScreenMirror.h"
#include "gui/UBMainWindow.h"
#include "gui/UBStartupHintsPalette.h"

#include "domain/UBGraphicsPixmapItem.h"

#include "podcast/UBPodcastController.h"

#include "network/UBNetworkAccessManager.h"

#include "ui_mainWindow.h"



#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#endif

#include "core/memcheck.h"

UBApplicationController::UBApplicationController(UBBoardView *pControlView,
                                                 UBBoardView *pDisplayView,
                                                 UBMainWindow* pMainWindow,
                                                 QObject* parent,
                                                 UBRightPalette* rightPalette)
    : QObject(parent)
    , mMainWindow(pMainWindow)
    , mControlView(pControlView)
    , mDisplayView(pDisplayView)
    , mMirror(0)
    , mMainMode(Board)
    , mAutomaticCheckForUpdates(false)
    , mCheckingForUpdates(false)
    , mIsShowingDesktop(false)
{
    mUninoteController = new UBDesktopAnnotationController(this, rightPalette);

    UBDisplayManager* displayManager = UBApplication::displayManager;

    connect(displayManager, SIGNAL(screenLayoutChanged()), this, SLOT(screenLayoutChanged()));
    connect(displayManager, SIGNAL(screenLayoutChanged()), mUninoteController, SLOT(screenLayoutChanged()));
    connect(displayManager, SIGNAL(screenLayoutChanged()), UBApplication::webController, SLOT(screenLayoutChanged()));
    connect(displayManager, &UBDisplayManager::availableScreenCountChanged,this,  [this](){
        initPreviousViews();
        UBApplication::displayManager->setPreviousDisplaysWidgets(mPreviousViews);
    });
    connect(mUninoteController, SIGNAL(imageCaptured(const QPixmap &, bool)), this, SLOT(addCapturedPixmap(const QPixmap &, bool)));
    connect(mUninoteController, SIGNAL(restoreUniboard()), this, SLOT(hideDesktop()));

    mBlackScene = std::make_shared<UBGraphicsScene>(nullptr);
    mBlackScene->setSceneBackground(true, nullptr);

    if (displayManager->numScreens() >= 2 && displayManager->useMultiScreen())
    {
        mMirror = new UBScreenMirror();
    }

    connect(UBApplication::webController, SIGNAL(imageCaptured(const QPixmap &, bool, const QUrl&))
            , this, SLOT(addCapturedPixmap(const QPixmap &, bool, const QUrl&)));

    mNetworkAccessManager = new QNetworkAccessManager (this);
    QTimer::singleShot (1000, this, SLOT (checkAtLaunch()));
}


UBApplicationController::~UBApplicationController()
{
    foreach(UBBoardView* view, mPreviousViews)
    {
        delete view;
    }
    delete mMirror;
    delete mUninoteController;
}


void UBApplicationController::initViewState(int horizontalPosition, int verticalPostition)
{
    mInitialHScroll = horizontalPosition;
    mInitialVScroll = verticalPostition;
}


void UBApplicationController::initScreenLayout(bool useMultiscreen)
{
    UBDisplayManager* displayManager = UBApplication::displayManager;
    UBBoardController* boardController = UBApplication::boardController;

    displayManager->initScreensByRole();
    initPreviousViews();
    displayManager->assignRoles();
    displayManager->setControlWidget(mMainWindow);
    displayManager->setDisplayWidget(mDisplayView);

    displayManager->setPreviousDisplaysWidgets(mPreviousViews);
    displayManager->setDesktopWidget(mUninoteController->drawingView());

    displayManager->setUseMultiScreen(useMultiscreen);

    adjustPreviousViews(boardController->activeSceneIndex(), boardController->selectedDocument());
    displayManager->positionScreens();
}


void UBApplicationController::screenLayoutChanged()
{
    initViewState(mControlView->horizontalScrollBar()->value(),
            mControlView->verticalScrollBar()->value());

    adaptToolBar();

    UBApplication::boardController->adjustDisplayViews();

    if (UBApplication::displayManager->hasDisplay())
    {
        UBApplication::boardController->setBoxing(mDisplayView->geometry());
    }
    else
    {
       UBApplication::boardController->setBoxing(QRect());
    }

    // update mirror if necessary
    UBDisplayManager* displayManager = UBApplication::displayManager;

    if (displayManager->numScreens() > 1 && displayManager->useMultiScreen() && !mMirror)
    {
        mMirror = new UBScreenMirror();
    }
    else if ((displayManager->numScreens() == 1 || !displayManager->useMultiScreen()) && mMirror)
    {
        delete mMirror;
        mMirror = nullptr;
    }
}


void UBApplicationController::adaptToolBar()
{
    bool highResolution = mMainWindow->width() > 1024;

    mMainWindow->actionClearPage->setVisible(Board == mMainMode && highResolution);
    mMainWindow->actionBoard->setVisible(Board != mMainMode || highResolution);
    mMainWindow->actionDocument->setVisible(Document != mMainMode || highResolution);
    mMainWindow->actionWeb->setVisible(Internet != mMainMode || highResolution);
    mMainWindow->boardToolBar->setIconSize(QSize(highResolution ? 48 : 42, mMainWindow->boardToolBar->iconSize().height()));

    mMainWindow->actionBoard->setEnabled(mMainMode != Board);
    mMainWindow->actionWeb->setEnabled(mMainMode != Internet);
    mMainWindow->actionDocument->setEnabled(mMainMode != Document);

    if (Document == mMainMode)
    {
        connect(UBApplication::instance(), SIGNAL(focusChanged(QWidget *, QWidget *)), UBApplication::documentController, SLOT(focusChanged(QWidget *, QWidget *)));
    }
    else
    {
        disconnect(UBApplication::instance(), SIGNAL(focusChanged(QWidget *, QWidget *)), UBApplication::documentController, SLOT(focusChanged(QWidget *, QWidget *)));
        if (Board == mMainMode)
            mMainWindow->actionDuplicate->setEnabled(true);
    }

    UBApplication::boardController->setToolbarTexts();

    UBApplication::webController->adaptToolBar();

}


void UBApplicationController::adjustDisplayView()
{
    if (mDisplayView)
    {
        qreal systemDisplayViewScaleFactor = 1.0;

        QSize pageSize = mControlView->size();
        QSize displaySize = mDisplayView->size();

        qreal hFactor = ((qreal)displaySize.width()) / ((qreal)pageSize.width());
        qreal vFactor = ((qreal)displaySize.height()) / ((qreal)pageSize.height());

        systemDisplayViewScaleFactor = qMax(hFactor, vFactor);

        QTransform tr;
        qreal scaleFactor = systemDisplayViewScaleFactor
                * UBApplication::boardController->currentZoom()
                * UBApplication::boardController->systemScaleFactor();

        tr.scale(scaleFactor, scaleFactor);

        QTransform recentTransform = mDisplayView->transform();

        if (recentTransform != tr)
            mDisplayView->setTransform(tr);

        QRect rect = mControlView->rect();
        mDisplayView->centerOn(mControlView->mapToScene(rect.center()));
    }
}


void UBApplicationController::adjustPreviousViews(int pActiveSceneIndex, std::shared_ptr<UBDocumentProxy> pActiveDocument)
{
    int viewIndex = pActiveSceneIndex;

    foreach(UBBoardView* previousView, mPreviousViews)
    {
        if (viewIndex > 0)
        {
            viewIndex--;

            std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pActiveDocument, viewIndex);

            if (scene)
            {
                previousView->setScene(scene.get());

                qreal ratio = ((qreal)previousView->geometry().width()) / ((qreal)previousView->geometry().height());
                QRectF sceneRect = scene->normalizedSceneRect(ratio);
                qreal scaleRatio = previousView->geometry().width() / sceneRect.width();

                previousView->resetTransform();

                previousView->scale(scaleRatio, scaleRatio);

                previousView->centerOn(sceneRect.center());
            }
        }
        else
        {
            previousView->setScene(mBlackScene.get());
        }
    }
}


void UBApplicationController::blackout()
{
    UBApplication::displayManager->blackout();
}


void UBApplicationController::addCapturedPixmap(const QPixmap &pPixmap, bool pageMode, const QUrl& sourceUrl)
{
    if (!pPixmap.isNull())
    {
        qreal sf = UBApplication::boardController->systemScaleFactor();
        qreal scaledWidth = ((qreal)pPixmap.width()) / sf;
        qreal scaledHeight = ((qreal)pPixmap.height()) / sf;

        QSize pageNominalSize = UBApplication::boardController->activeScene()->nominalSize();

        int newWidth = qMin((int)scaledWidth, pageNominalSize.width());
        int newHeight = qMin((int)scaledHeight, pageNominalSize.height());

        if (pageMode)
        {
            newHeight = pPixmap.height();
        }

        QSizeF scaledSize(scaledWidth, scaledHeight);
        scaledSize.scale(newWidth, newHeight, Qt::KeepAspectRatio);

        qreal scaleFactor = qMin(scaledSize.width() / (qreal)pPixmap.width(), scaledSize.height() / (qreal)pPixmap.height());

        QPointF pos(0.0, 0.0);

        if (pageMode)
        {
            pos.setY(pageNominalSize.height() / -2  + scaledSize.height() / 2);
        }

        UBApplication::boardController->paletteManager()->addItem(pPixmap, pos, scaleFactor, sourceUrl);
    }
}


void UBApplicationController::addCapturedEmbedCode(const QString& embedCode)
{
    if (!embedCode.isEmpty())
    {
        showBoard();

        const QString userWidgetPath = UBSettings::settings()->userInteractiveDirectory() + "/" + tr("Web"); // TODO UB 4.x synch with w3cWidget
        QDir userWidgetDir(userWidgetPath);

        int width = 300;
        int height = 150;

        QString widgetPath = UBGraphicsW3CWidgetItem::createHtmlWrapperInDir(embedCode, userWidgetDir,
                QSize(width, height), UBStringUtils::toCanonicalUuid(QUuid::createUuid()));

        if (widgetPath.length() > 0)
            UBApplication::boardController->downloadURL(QUrl::fromLocalFile(widgetPath));
    }
}


void UBApplicationController::showBoard()
{
    mMainWindow->webToolBar->hide();
    mMainWindow->documentToolBar->hide();
    mMainWindow->boardToolBar->show();

    if (mMainMode == Document)
    {
//        int selectedSceneIndex = UBApplication::documentController->getSelectedItemIndex();
//        if (selectedSceneIndex != -1)
//        {
//            UBApplication::boardController->setActiveDocumentScene(UBApplication::documentController->selectedDocument(), selectedSceneIndex);
//        }
    }

    mMainMode = Board;

    adaptToolBar();

    mirroringEnabled(false);

    mMainWindow->switchToBoardWidget();

    if (UBApplication::boardController)
        UBApplication::boardController->show();

    mIsShowingDesktop = false;
    UBPlatformUtils::hideMenuBarAndDock();
    UBDrawingController::drawingController()->setInDesktopMode(false);

    mUninoteController->hideWindow();

    UBPlatformUtils::showFullScreen(mMainWindow);

    emit mainModeChanged(Board);

    UBApplication::boardController->freezeW3CWidgets(false);
}


void UBApplicationController::showInternet()
{

    if (UBApplication::boardController)
    {
        UBApplication::boardController->persistCurrentScene();
        UBApplication::boardController->hide();
    }

    if (UBSettings::settings()->webUseExternalBrowser->get().toBool())
    {
        showDesktop(true);
        UBApplication::webController->show();
    }
    else
    {
        mMainWindow->boardToolBar->hide();
        mMainWindow->documentToolBar->hide();
        mMainWindow->webToolBar->show();

        mMainMode = Internet;

        adaptToolBar();

        mMainWindow->show();
        mUninoteController->hideWindow();

        UBApplication::webController->show();

        UBApplication::displayManager->adjustScreens();

        emit mainModeChanged(Internet);
    }
}


void UBApplicationController::showDocument()
{
    mMainWindow->webToolBar->hide();
    mMainWindow->boardToolBar->hide();
    UBPlatformUtils::hideMenuBarAndDock();
    mMainWindow->documentToolBar->show();

    mMainMode = Document;

    adaptToolBar();

    mirroringEnabled(false);

    mMainWindow->switchToDocumentsWidget();

    if (UBApplication::boardController)
    {
        UBApplication::boardController->persistCurrentScene();
        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(UBApplication::boardController->selectedDocument());

        UBApplication::boardController->hide();
    }

    if (UBApplication::documentController)
    {
        UBApplication::documentController->show();
    }

    mMainWindow->show();

    mUninoteController->hideWindow();

    emit mainModeChanged(Document);
}

void UBApplicationController::showDesktop(bool dontSwitchFrontProcess)
{
    if (UBApplication::boardController)
        UBApplication::boardController->hide();

    mMainWindow->hide();
    mUninoteController->showWindow();

    if (mMirror)
    {
        // grab from screen instead of widget
        mMirror->setSourceWidget(nullptr);
    }

    mIsShowingDesktop = true;
    emit desktopMode(true);

    if (!dontSwitchFrontProcess) {
        UBPlatformUtils::bringPreviousProcessToFront();
    }

    UBDrawingController::drawingController()->setInDesktopMode(true);
    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
}


void UBApplicationController::checkUpdate(const QUrl& url)
{
    QUrl jsonUrl = url;
    if (url.isEmpty())
        jsonUrl = UBSettings::settings()->appSoftwareUpdateURL->get().toUrl();

    qDebug() << "Checking for update at url: " << jsonUrl.toString();

    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(updateRequestFinished(QNetworkReply*)));

    mNetworkAccessManager->get(QNetworkRequest(jsonUrl));

}



void UBApplicationController::updateRequestFinished(QNetworkReply * reply)
{
    if (reply->error()) {
        qWarning() << "Error downloading update file: " << reply->errorString();
        return;
    }

    // Check if we are being redirected. If so, call checkUpdate again

    QVariant redirect_target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirect_target.isNull()) {
        // The returned URL might be relative. resolved() creates an absolute url from it
        QUrl redirect_url(reply->url().resolved(redirect_target.toUrl()));

        checkUpdate(redirect_url);
        return;
    }

    // No error and no redirect => we read the whole response

    QString responseString = QString(reply->readAll());

    if (!responseString.isEmpty() &&
            responseString.contains("version") &&
            responseString.contains("url")) {

        reply->close();
        reply->deleteLater();

        downloadJsonFinished(responseString);
    }
}

void UBApplicationController::initPreviousViews()
{
    int numPreviousViews = UBApplication::displayManager->numPreviousViews();

    // create the missing views
    for (int i = mPreviousViews.count(); i < numPreviousViews; i++)
    {
        UBBoardView *previousView = new UBBoardView(UBApplication::boardController, UBItemLayerType::FixedBackground, UBItemLayerType::Tool, 0);
        previousView->setInteractive(false);
        mPreviousViews.append(previousView);
    }

    // delete the superfluous views
    while (mPreviousViews.count() > numPreviousViews)
    {
        UBBoardView* view = mPreviousViews.takeLast();
        delete view;
    }
}


void UBApplicationController::downloadJsonFinished(QString currentJson)
{
    /*
      The .json files simply specify the latest version number available, and
      the URL to send the user to, so they can download it.

      They look like:

          {
            "version": "1.3.5",
            "url": "http://openboard.ch"
          }
    */

    QJsonObject jsonObject = QJsonDocument::fromJson(currentJson.toUtf8()).object();

    UBVersion installedVersion (qApp->applicationVersion());
    UBVersion jsonVersion (jsonObject.value("version").toString());

    qDebug() << "json version: " << jsonVersion.toUInt();
    qDebug() << "installed version: " << installedVersion.toUInt();

    if (jsonVersion > installedVersion) {
        if (UBApplication::mainWindow->yesNoQuestion(tr("Update available"), tr ("New update available, would you go to the web page ?"))){
            QUrl url(jsonObject.value("url").toString());
            QDesktopServices::openUrl(url);
        }
    }
    else if (isNoUpdateDisplayed) {
        mMainWindow->information(tr("Update"), tr("No update available"));
    }
}

void UBApplicationController::checkAtLaunch()
{
    if(UBSettings::settings()->appEnableAutomaticSoftwareUpdates->get().toBool()){
        isNoUpdateDisplayed = false;
        checkUpdate();
    }
}

void UBApplicationController::checkUpdateRequest()
{
    isNoUpdateDisplayed = true;
    checkUpdate();
}

void UBApplicationController::hideDesktop()
{
    if (mMainMode == Board)
    {
        showBoard();
    }
    else if (mMainMode == Internet)
    {
        showInternet();
    }
    else if (mMainMode == Document)
    {
        showDocument();
    }

    mIsShowingDesktop = false;

    UBApplication::displayManager->adjustScreens();

    emit desktopMode(false);
}

void UBApplicationController::setMirrorSourceWidget(QWidget* pWidget)
{
    if (mMirror)
    {
        mMirror->setSourceWidget(pWidget);
    }
}


void UBApplicationController::mirroringEnabled(bool enabled)
{
    if (mMirror)
    {
        if (enabled)
        {
            mMirror->start();
            UBApplication::displayManager->setDisplayWidget(mMirror);

        }
        else
        {
            UBApplication::displayManager->setDisplayWidget(mDisplayView);
            mMirror->stop();
        }

        mMirror->setVisible(enabled && UBApplication::displayManager->numScreens() > 1);
        mUninoteController->updateShowHideState(enabled);
        UBApplication::mainWindow->actionWebShowHideOnDisplay->setChecked(enabled);
    }
    else
    {
        UBApplication::displayManager->setDisplayWidget(mDisplayView);
    }
}



void UBApplicationController::closing()
{
    if (mMirror)
        mMirror->stop();

    if (mUninoteController)
    {
        mUninoteController->hideWindow();
        mUninoteController->close();
    }

    /*

    if (UBApplication::documentController)
        UBApplication::documentController->closing();

    */

    UBPersistenceManager::persistenceManager()->closing(); // ALTI/AOU - 20140616 : to update the file "documents/folders.xml"
}


void UBApplicationController::showMessage(const QString& message, bool showSpinningWheel)
{
    if (!UBApplication::closingDown())
    {
        if (mMainMode == Document)
        {
            UBApplication::boardController->hideMessage();
            UBApplication::documentController->showMessage(message, showSpinningWheel);
        }
        else
        {
            UBApplication::documentController->hideMessage();
            UBApplication::boardController->showMessage(message, showSpinningWheel);
        }
    }
}


void UBApplicationController::importFile(const QString& pFilePath)
{
    const QFile fileToOpen(pFilePath);

    if (!fileToOpen.exists())
        return;

    bool success = false;

    std::shared_ptr<UBDocumentProxy> document = UBDocumentManager::documentManager()->importFile(fileToOpen, "");

    success = (document != 0);

    if (success && document)
    {
        if (UBApplication::boardController)
        {
            UBApplication::boardController->setActiveDocumentScene(document, 0, true, true);
        }

        if (UBApplication::documentController)
        {
            UBApplication::documentController->selectDocument(document, true, true);
        }

        // This import operation happens when double-clicking on a UBZ for example.
        // The document is added and set as current document, so the user probably wants to see it immediately.
        showBoard();
    }
}

void UBApplicationController::useMultiScreen(bool use)
{
    if (use && !mMirror)
        mMirror = new UBScreenMirror();
    if (!use && mMirror) {
        mirroringEnabled(false);
        delete mMirror;
        mMirror = NULL;
    }

    UBApplication::displayManager->setUseMultiScreen(use);
    UBApplication::displayManager->adjustScreens();
    UBSettings::settings()->appUseMultiscreen->set(use);

}


QStringList UBApplicationController::widgetInlineJavaScripts()
{
    QString scriptDirPath = UBPlatformUtils::applicationResourcesDirectory() + "/widget-inline-js";
    QDir scriptDir(scriptDirPath);

    QStringList scripts;

    if (scriptDir.exists())
    {
        QStringList files = scriptDir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

        foreach(QString file, files)
        {
            QFile scriptFile(scriptDirPath + "/" + file);
            if (file.endsWith(".js") && scriptFile.open(QIODevice::ReadOnly))
            {
                QString s = QString::fromUtf8(scriptFile.readAll());

                if (s.length() > 0)
                    scripts << s;

            }
        }
    }

    std::sort(scripts.begin(), scripts.end());

    return scripts;
}



void UBApplicationController::actionCut()
{
    if (!UBApplication::closingDown())
    {
        if (mMainMode == Board)
        {
            UBApplication::boardController->cut();
        }
        else if(mMainMode == Document)
        {
            UBApplication::documentController->cut();
        }
        else if(mMainMode == Internet)
        {
            UBApplication::webController->cut();
        }
    }
}


void UBApplicationController::actionCopy()
{
    if (!UBApplication::closingDown())
    {
        if (mMainMode == Board)
        {
            UBApplication::boardController->copy();
        }
        else if(mMainMode == Document)
        {
            UBApplication::documentController->copy();
        }
        else if(mMainMode == Internet)
        {
            UBApplication::webController->copy();
        }
    }
}


void UBApplicationController::actionPaste()
{
    if (!UBApplication::closingDown())
    {
        if (mMainMode == Board)
        {
            UBApplication::boardController->paste();
        }
        else if (mMainMode == Document)
        {
            UBApplication::documentController->paste();
        }
        else if(mMainMode == Internet)
        {
            UBApplication::webController->paste();
        }
    }
}
