#include "UBBoardController.h"

#include <QtGui>
#include <QtWebKit>

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBPersistenceManager.h"
#include "core/UBApplicationController.h"
#include "core/UBDocumentManager.h"
#include "core/UBMimeData.h"

#include "network/UBHttpGet.h"

#include "gui/UBMessageWindow.h"
#include "gui/UBResources.h"
#include "gui/UBToolbarButtonGroup.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolWidget.h"

#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsItemUndoCommand.h"
#include "domain/UBGraphicsProxyWidget.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsVideoItem.h"
#include "domain/UBGraphicsAudioItem.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBW3CWidget.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBPageSizeUndoCommand.h"

#include "tools/UBToolsManager.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "board/UBLibraryController.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardView.h"

#include "podcast/UBPodcastController.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "UBBoardPaletteManager.h"

UBBoardController::UBBoardController(UBMainWindow* mainWindow)
    : QObject(mainWindow->centralWidget())
    , mMainWindow(mainWindow)
    , mActiveDocument(0)
    , mActiveScene(0)
    , mActiveSceneIndex(-1)
    , mLibraryController(0)
    , mPaletteManager(0)
    , mSoftwareUpdateDialog(0)
    , mMessageWindow(0)
    , mControlView(0)
    , mDisplayView(0)
    , mControlContainer(0)
    , mControlLayout(0)
    , mZoomFactor(1.0)
    , mIsClosing(false)
    , mSystemScaleFactor(1.0)
    , mCleanupDone(false)
{
    mLibraryController = new UBLibraryController(mMainWindow->centralWidget(), this);

    mZoomFactor = UBSettings::settings()->boardZoomFactor->get().toDouble();

    int penColorIndex = UBSettings::settings()->penColorIndex();
    int markerColorIndex = UBSettings::settings()->markerColorIndex();

    mPenColorOnDarkBackground = UBSettings::settings()->penColors(true).at(penColorIndex);
    mPenColorOnLightBackground = UBSettings::settings()->penColors(false).at(penColorIndex);
    mMarkerColorOnDarkBackground = UBSettings::settings()->markerColors(true).at(markerColorIndex);
    mMarkerColorOnLightBackground = UBSettings::settings()->markerColors(false).at(markerColorIndex);
}


void UBBoardController::init()
{
    setupViews();
    setupToolbar();

    connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentWillBeDeleted(UBDocumentProxy*))
            , this, SLOT(documentWillBeDeleted(UBDocumentProxy*)));

    connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneCreated(UBDocumentProxy*, int))
            , this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));

    connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneDeleted(UBDocumentProxy*, int))
            , this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));

    connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneMoved(UBDocumentProxy*, int))
            , this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));

    connect(UBApplication::undoStack, SIGNAL(canUndoChanged(bool))
            , this, SLOT(undoRedoStateChange(bool)));

    connect(UBApplication::undoStack, SIGNAL(canRedoChanged (bool))
            , this, SLOT(undoRedoStateChange(bool)));

    connect(UBDrawingController::drawingController(), SIGNAL(stylusToolChanged(int))
            , this, SLOT(setToolCursor(int)));

    connect(UBDrawingController::drawingController(), SIGNAL(stylusToolChanged(int))
            , this, SLOT(stylusToolChanged(int)));

    connect(UBApplication::app(), SIGNAL(lastWindowClosed())
            , this, SLOT(lastWindowClosed()));

    UBDocumentProxy* doc = UBPersistenceManager::persistenceManager()->createDocument();

    setActiveDocumentScene(doc);

    undoRedoStateChange(true);
}


UBBoardController::~UBBoardController()
{
    delete mDisplayView;
}


void UBBoardController::setupViews()
{
    mControlContainer = new QWidget(mMainWindow->centralWidget());

    mControlLayout = new QHBoxLayout(mControlContainer);
    mControlLayout->setContentsMargins(0, 0, 0, 0);

    mControlView = new UBBoardView(this, mControlContainer);
    mControlView->setInteractive(true);
    mControlView->setMouseTracking(true);

    mControlView->grabGesture(Qt::SwipeGesture);

    mControlView->setTransformationAnchor(QGraphicsView::NoAnchor);

    mControlLayout->addWidget(mControlView);
    mControlContainer->setObjectName("ubBoardControlContainer");
    mMainWindow->addBoardWidget(mControlContainer);

    connect(mControlView, SIGNAL(resized(QResizeEvent*)), this, SLOT(boardViewResized(QResizeEvent*)));

    // TODO UB 4.x Optimization do we have to create the display view even if their is
    // only 1 screen
    //
    mDisplayView = new UBBoardView(this, UBItemLayerType::FixedBackground, UBItemLayerType::Tool, 0);
    mDisplayView->setInteractive(false);
    mDisplayView->setTransformationAnchor(QGraphicsView::NoAnchor);

    mMessageWindow = new UBMessageWindow(mControlView);
    mMessageWindow->hide();

    mPaletteManager = new UBBoardPaletteManager(mControlContainer, this);
    connect(this, SIGNAL(activeSceneChanged()), mPaletteManager, SLOT(activeSceneChanged()));

}


void UBBoardController::setupLayout()
{
    if(mPaletteManager)
        mPaletteManager->setupLayout();
}


void UBBoardController::setBoxing(QRect displayRect)
{
    if (displayRect.isNull())
    {
        mControlLayout->setContentsMargins(0, 0, 0, 0);
        return;
    }

    qreal controlWidth = (qreal)mMainWindow->centralWidget()->width();
    qreal controlHeight = (qreal)mMainWindow->centralWidget()->height();
    qreal displayWidth = (qreal)displayRect.width();
    qreal displayHeight = (qreal)displayRect.height();

    qreal displayRatio = displayWidth / displayHeight;
    qreal controlRatio = controlWidth / controlHeight;

    if (displayRatio < controlRatio)
    {
        // Pillarboxing
        int boxWidth = (controlWidth - (displayWidth * (controlHeight / displayHeight))) / 2;
        mControlLayout->setContentsMargins(boxWidth, 0, boxWidth, 0);
    }
    else if (displayRatio > controlRatio)
    {
        // Letterboxing
        int boxHeight = (controlHeight - (displayHeight * (controlWidth / displayWidth))) / 2;
        mControlLayout->setContentsMargins(0, boxHeight, 0, boxHeight);
    }
    else
    {
        // No boxing
        mControlLayout->setContentsMargins(0, 0, 0, 0);
    }
}


QSize UBBoardController::displayViewport()
{
    return mDisplayView->geometry().size();
}


QSize UBBoardController::controlViewport()
{
    return mControlView->geometry().size();
}


QRectF UBBoardController::controlGeometry()
{
    return mControlView->geometry();
}


void UBBoardController::setupToolbar()
{
    UBSettings *settings = UBSettings::settings();

    // Setup color choice widget
    QList<QAction *> colorActions;
    colorActions.append(mMainWindow->actionColor0);
    colorActions.append(mMainWindow->actionColor1);
    colorActions.append(mMainWindow->actionColor2);
    colorActions.append(mMainWindow->actionColor3);

    UBToolbarButtonGroup *colorChoice =
            new UBToolbarButtonGroup(mMainWindow->boardToolBar, colorActions);

    mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, colorChoice);

    connect(settings->appToolBarDisplayText, SIGNAL(changed(QVariant)), colorChoice, SLOT(displayText(QVariant)));
    connect(colorChoice, SIGNAL(activated(int)), this, SLOT(setColorIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), colorChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), colorChoice, SLOT(colorPaletteChanged()));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), this, SLOT(colorPaletteChanged()));

    colorChoice->displayText(QVariant(settings->appToolBarDisplayText->get().toBool()));
    colorChoice->colorPaletteChanged();

    // Setup line width choice widget
    QList<QAction *> lineWidthActions;
    lineWidthActions.append(mMainWindow->actionLineSmall);
    lineWidthActions.append(mMainWindow->actionLineMedium);
    lineWidthActions.append(mMainWindow->actionLineLarge);

    UBToolbarButtonGroup *lineWidthChoice =
            new UBToolbarButtonGroup(mMainWindow->boardToolBar, lineWidthActions);

    connect(settings->appToolBarDisplayText, SIGNAL(changed(QVariant)), lineWidthChoice, SLOT(displayText(QVariant)));

    connect(lineWidthChoice, SIGNAL(activated(int))
            , UBDrawingController::drawingController(), SLOT(setLineWidthIndex(int)));

    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int))
            , lineWidthChoice, SLOT(setCurrentIndex(int)));

    lineWidthChoice->displayText(QVariant(settings->appToolBarDisplayText->get().toBool()));

    mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, lineWidthChoice);

    // Setup eraser width choice widget
    QList<QAction *> eraserWidthActions;
    eraserWidthActions.append(mMainWindow->actionEraserSmall);
    eraserWidthActions.append(mMainWindow->actionEraserMedium);
    eraserWidthActions.append(mMainWindow->actionEraserLarge);

    UBToolbarButtonGroup *eraserWidthChoice =
            new UBToolbarButtonGroup(mMainWindow->boardToolBar, eraserWidthActions);

    mMainWindow->boardToolBar->insertWidget(mMainWindow->actionBackgrounds, eraserWidthChoice);

    connect(settings->appToolBarDisplayText, SIGNAL(changed(QVariant)), eraserWidthChoice, SLOT(displayText(QVariant)));
    connect(eraserWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setEraserWidthIndex(int)));

    eraserWidthChoice->displayText(QVariant(settings->appToolBarDisplayText->get().toBool()));
    eraserWidthChoice->setCurrentIndex(settings->eraserWidthIndex());

    mMainWindow->boardToolBar->insertSeparator(mMainWindow->actionBackgrounds);
    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->boardToolBar, mMainWindow->actionBoard);
    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->tutorialToolBar, mMainWindow->actionBoard);

    UBApplication::app()->decorateActionMenu(mMainWindow->actionMenu);

    mMainWindow->actionBoard->setVisible(false);

    mMainWindow->webToolBar->hide();
    mMainWindow->documentToolBar->hide();
    mMainWindow->tutorialToolBar->hide();

    connectToolbar();
    initToolbarTexts();
}


void UBBoardController::setToolCursor(int tool)
{
    if (mActiveScene)
    {
        mActiveScene->setToolCursor(tool);
    }

    mControlView->setToolCursor(tool);
}


void UBBoardController::connectToolbar()
{
    connect(mMainWindow->actionAdd, SIGNAL(triggered()), this, SLOT(addItem()));
    connect(mMainWindow->actionNewPage, SIGNAL(triggered()), this, SLOT(addScene()));
    connect(mMainWindow->actionDuplicatePage, SIGNAL(triggered()), this, SLOT(duplicateScene()));

    connect(mMainWindow->actionClearPage, SIGNAL(triggered()), this, SLOT(clearScene()));
    connect(mMainWindow->actionEraseItems, SIGNAL(triggered()), this, SLOT(clearSceneItems()));
    connect(mMainWindow->actionEraseAnnotations, SIGNAL(triggered()), this, SLOT(clearSceneAnnotation()));

    connect(mMainWindow->actionUndo, SIGNAL(triggered()), UBApplication::undoStack, SLOT(undo()));
    connect(mMainWindow->actionRedo, SIGNAL(triggered()), UBApplication::undoStack, SLOT(redo()));
    connect(mMainWindow->actionBack, SIGNAL( triggered()), this, SLOT(previousScene()));
    connect(mMainWindow->actionForward, SIGNAL(triggered()), this, SLOT(nextScene()));
    connect(mMainWindow->actionSleep, SIGNAL(triggered()), this, SLOT(blackout()));
    connect(mMainWindow->actionVirtualKeyboard, SIGNAL(triggered(bool)), this, SLOT(showKeyboard(bool)));
    connect(mMainWindow->actionImportPage, SIGNAL(triggered()), this, SLOT(importPage()));
}


void UBBoardController::initToolbarTexts()
{
    QList<QAction*> allToolbarActions;

    allToolbarActions << mMainWindow->boardToolBar->actions();
    allToolbarActions << mMainWindow->webToolBar->actions();
    allToolbarActions << mMainWindow->documentToolBar->actions();

    foreach(QAction* action, allToolbarActions)
    {
        QString nominalText = action->text();
        QString shortText = truncate(nominalText, 48);
        QPair<QString, QString> texts(nominalText, shortText);

        mActionTexts.insert(action, texts);
    }
}


void UBBoardController::setToolbarTexts()
{
    bool highResolution = mMainWindow->width() > 1024;
    QSize iconSize;

    if (highResolution)
        iconSize = QSize(48, 32);
    else
        iconSize = QSize(32, 32);

    mMainWindow->boardToolBar->setIconSize(iconSize);
    mMainWindow->webToolBar->setIconSize(iconSize);
    mMainWindow->documentToolBar->setIconSize(iconSize);

    foreach(QAction* action, mActionTexts.keys())
    {
        QPair<QString, QString> texts = mActionTexts.value(action);

        if (highResolution)
            action->setText(texts.first);
        else
        {
            action->setText(texts.second);
        }

        action->setToolTip(texts.first);
    }
}


QString UBBoardController::truncate(QString text, int maxWidth)
{
    QFontMetricsF fontMetrics(mMainWindow->font());
    return fontMetrics.elidedText(text, Qt::ElideRight, maxWidth);
}


void UBBoardController::stylusToolDoubleClicked(int tool)
{
    if (tool == UBStylusTool::ZoomIn || tool == UBStylusTool::ZoomOut)
    {
        zoomRestore();
    }
    else if (tool == UBStylusTool::Hand)
    {
        centerRestore();
    }
}



void UBBoardController::addScene()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    persistCurrentScene();

    UBPersistenceManager::persistenceManager()->createDocumentSceneAt(mActiveDocument, mActiveSceneIndex + 1);
    mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    setActiveDocumentScene(mActiveDocument, mActiveSceneIndex + 1);
    QApplication::restoreOverrideCursor();
}


void UBBoardController::addScene(UBGraphicsScene* scene, bool replaceActiveIfEmpty)
{
    if (scene)
    {
        UBGraphicsScene* clone = scene->sceneDeepCopy();

        if (scene->document() && (scene->document() != mActiveDocument))
        {
            foreach(QUrl relativeFile, scene->relativeDependencies())
            {
                QString source = scene->document()->persistencePath() + "/" + relativeFile.toString();
                QString target = mActiveDocument->persistencePath() + "/" + relativeFile.toString();

                QFileInfo fi(target);
                QDir d = fi.dir();

                d.mkpath(d.absolutePath());
                QFile::copy(source, target);
            }
        }

        if (replaceActiveIfEmpty && mActiveScene->isEmpty())
        {
            UBPersistenceManager::persistenceManager()->persistDocumentScene(mActiveDocument, clone, mActiveSceneIndex);
            setActiveDocumentScene(mActiveDocument, mActiveSceneIndex);
        }
        else
        {
            persistCurrentScene();
            UBPersistenceManager::persistenceManager()->insertDocumentSceneAt(mActiveDocument, clone, mActiveSceneIndex + 1);
            setActiveDocumentScene(mActiveDocument, mActiveSceneIndex + 1);
        }

        mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
    }
}


void UBBoardController::addScene(UBDocumentProxy* proxy, int sceneIndex, bool replaceActiveIfEmpty)
{
    UBGraphicsScene* scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(proxy, sceneIndex);

    if (scene)
    {
        addScene(scene, replaceActiveIfEmpty);
    }
}


void UBBoardController::duplicateScene()
{
    if (UBApplication::applicationController->displayMode() != UBApplicationController::Board)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    persistCurrentScene();

    UBPersistenceManager::persistenceManager()->duplicateDocumentScene(mActiveDocument, mActiveSceneIndex);
    mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    setActiveDocumentScene(mActiveDocument, mActiveSceneIndex + 1);
    QApplication::restoreOverrideCursor();
}


void UBBoardController::clearScene()
{
    if (mActiveScene)
    {
        mActiveScene->clearItemsAndAnnotations();
        updateActionStates();
    }
}


void UBBoardController::clearSceneItems()
{
    if (mActiveScene)
    {
        mActiveScene->clearItems();
        updateActionStates();
    }
}


void UBBoardController::clearSceneAnnotation()
{
    if (mActiveScene)
    {
        mActiveScene->clearAnnotations();
        updateActionStates();
    }
}


void UBBoardController::showDocumentsDialog()
{
    if (mActiveDocument)
        persistCurrentScene();

    UBApplication::mainWindow->actionLibrary->setChecked(false);

}

void UBBoardController::libraryDialogClosed(int ret)
{
    Q_UNUSED(ret);

    mMainWindow->actionLibrary->setChecked(false);
}


void UBBoardController::blackout()
{
    UBApplication::applicationController->blackout();
}


void UBBoardController::showKeyboard(bool show)
{
    mPaletteManager->showVirtualKeyboard(show);
}


void UBBoardController::zoomIn(QPointF scenePoint)
{
    if (mControlView->transform().m11() > UB_MAX_ZOOM)
    {
        qApp->beep();
        return;
    }
    zoom(mZoomFactor, scenePoint);
}


void UBBoardController::zoomOut(QPointF scenePoint)
{
    if ((mControlView->horizontalScrollBar()->maximum() == 0) && (mControlView->verticalScrollBar()->maximum() == 0))
    {
        // Do not zoom out if we reached the maximum
        qApp->beep();
        return;
    }

    qreal newZoomFactor = 1 / mZoomFactor;

    zoom(newZoomFactor, scenePoint);
}


void UBBoardController::zoomRestore()
{
    QTransform tr;

    tr.scale(mSystemScaleFactor, mSystemScaleFactor);
    mControlView->setTransform(tr);

    centerRestore();

    foreach(QGraphicsItem *gi, mActiveScene->selectedItems ())
    {
        //force item to redraw the frame (for the anti scale calculation)
        gi->setSelected(false);
        gi->setSelected(true);
    }

    emit zoomChanged(1.0);
}


void UBBoardController::centerRestore()
{
    centerOn(QPointF(0,0));
}


void UBBoardController::centerOn(QPointF scenePoint)
{
    mControlView->centerOn(scenePoint);
    UBApplication::applicationController->adjustDisplayView();
}


void UBBoardController::zoom(const qreal ratio, QPointF scenePoint)
{

    QPointF viewCenter = mControlView->mapToScene(QRect(0, 0, mControlView->width(), mControlView->height()).center());
    QPointF offset = scenePoint - viewCenter;
    QPointF scalledOffset = offset / ratio;

    mControlView->scale(ratio, ratio);

    qreal currentZoom = mControlView->viewportTransform().m11() / mSystemScaleFactor;

    QPointF newCenter = scenePoint - scalledOffset;

    mControlView->centerOn(newCenter);

    emit zoomChanged(currentZoom);
    UBApplication::applicationController->adjustDisplayView();

    emit controlViewportChanged();
}


void UBBoardController::handScroll(qreal dx, qreal dy)
{
    mControlView->horizontalScrollBar()->setValue(mControlView->horizontalScrollBar()->value() - dx);
    mControlView->verticalScrollBar()->setValue(mControlView->verticalScrollBar()->value() - dy);

    UBApplication::applicationController->adjustDisplayView();

    emit controlViewportChanged();
}


void UBBoardController::previousScene()
{
    if (mActiveSceneIndex > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        persistCurrentScene();
        setActiveDocumentScene(mActiveDocument, mActiveSceneIndex - 1);
        QApplication::restoreOverrideCursor();
    }

    updateActionStates();
}


void UBBoardController::nextScene()
{
    if (mActiveSceneIndex < mActiveDocument->pageCount() - 1)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        persistCurrentScene();
        setActiveDocumentScene(mActiveDocument, mActiveSceneIndex + 1);
        QApplication::restoreOverrideCursor();
    }

    updateActionStates();

}


void UBBoardController::firstScene()
{
    if (mActiveSceneIndex > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        persistCurrentScene();
        setActiveDocumentScene(mActiveDocument, 0);
        QApplication::restoreOverrideCursor();
    }

    updateActionStates();
}


void UBBoardController::lastScene()
{
    if (mActiveSceneIndex < mActiveDocument->pageCount() - 1)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        persistCurrentScene();
        setActiveDocumentScene(mActiveDocument, mActiveDocument->pageCount() - 1);
        QApplication::restoreOverrideCursor();
    }

    updateActionStates();
}


void UBBoardController::downloadURL(const QUrl& url, const QPointF& pPos, const QSize& pSize, bool isBackground)
{
    QString sUrl = url.toString();

    if(sUrl.startsWith("uniboardTool://"))
    {
        downloadFinished(true, url, "application/vnd.mnemis-uniboard-tool", QByteArray(), pPos, pSize, isBackground);
    }
    else if (sUrl.startsWith("file://") || sUrl.startsWith("/"))
    {
        QString fileName = url.toLocalFile();

        QString contentType = UBFileSystemUtils::mimeTypeFromFileName(fileName);

        bool shouldLoadFileData =
                contentType.startsWith("image")
                || contentType.startsWith("application/widget")
                || contentType.startsWith("application/vnd.apple-widget");

        QFile file(fileName);

        if (shouldLoadFileData)
            file.open(QIODevice::ReadOnly);

        downloadFinished(true, url, contentType, file.readAll(), pPos, pSize, isBackground);

        if (shouldLoadFileData)
            file.close();
    }
    else
    {
        UBHttpGet *http = new UBHttpGet(mActiveScene);

        showMessage(tr("Downloading content from %1").arg(url.toString()), true);

        connect(http, SIGNAL(downloadFinished(bool, QUrl, QString, QByteArray, QPointF, QSize, bool)),
                this, SLOT(downloadFinished(bool, QUrl, QString, QByteArray, QPointF, QSize, bool)));

        http->get(url, pPos, pSize, isBackground);
    }
}


void UBBoardController::downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground)
{
    QString mimeType = pContentTypeHeader;

    if (!pSuccess)
    {
        UBApplication::showMessage(tr("Downloading content %1 failed").arg(sourceUrl.toString()));
        return;
    }

    if (!sourceUrl.toString().startsWith("file://") && !sourceUrl.toString().startsWith("uniboardTool://"))
        UBApplication::showMessage(tr("Download finished"));

    if (mimeType == "image/jpeg"
            || mimeType == "image/png"
            || mimeType == "image/gif"
            || mimeType == "image/tiff"
            || mimeType == "image/bmp")
    {

        qDebug() << "accepting mime type" << mimeType << "as raster image";

        QImage img;
        img.loadFromData(pData);
        QPixmap pix = QPixmap::fromImage(img);

        UBGraphicsPixmapItem* pixItem = mActiveScene->addPixmap(pix, pPos, 1.);
        pixItem->setSourceUrl(sourceUrl);

        if (isBackground)
        {
            mActiveScene->setAsBackgroundObject(pixItem, true);
        }
        else
        {
            mActiveScene->scaleToFitDocumentSize(pixItem, true, UBSettings::objectInControlViewMargin);
            pixItem->setSelected(true);
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
    }
    else if (mimeType == "image/svg+xml")
    {
        qDebug() << "accepting mime type" << mimeType << "as vecto image";

        UBGraphicsSvgItem* svgItem = mActiveScene->addSvg(sourceUrl, pPos);
        svgItem->setSourceUrl(sourceUrl);

        if (isBackground)
        {
            mActiveScene->setAsBackgroundObject(svgItem);
        }
        else
        {
            mActiveScene->scaleToFitDocumentSize(svgItem, true, UBSettings::objectInControlViewMargin);
            svgItem->setSelected(true);
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
    }
    else if (mimeType == "application/vnd.apple-widget") //mime type invented by us :-(
    {
        qDebug() << "accepting mime type" << mimeType << "as Apple widget";

        QUrl widgetUrl = sourceUrl;

        if (pData.length() > 0)
        {
            widgetUrl = expandWidgetToTempDir(pData, "wdgt");
        }

        UBGraphicsWidgetItem* appleWidgetItem = mActiveScene->addAppleWidget(widgetUrl, pPos);

        appleWidgetItem->setSourceUrl(sourceUrl);

        if (isBackground)
        {
            mActiveScene->setAsBackgroundObject(appleWidgetItem);
        }
        else
        {
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
    }
    else if (mimeType == "application/widget")
    {
        qDebug() << "accepting mime type" << mimeType << "as W3C widget";
        QUrl widgetUrl = sourceUrl;

        if (pData.length() > 0)
        {
            widgetUrl = expandWidgetToTempDir(pData);
        }

        UBGraphicsWidgetItem* w3cWidgetItem = mActiveScene->addW3CWidget(widgetUrl, pPos);

        w3cWidgetItem->setSourceUrl(sourceUrl);

        if (isBackground)
        {
            mActiveScene->setAsBackgroundObject(w3cWidgetItem);
        }
        else
        {
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
    }
    else if (mimeType.startsWith("video/"))
    {
        qDebug() << "accepting mime type" << mimeType << "as video";

        UBGraphicsVideoItem *videoItem = 0;

        if (pData.length() > 0)
        {
            QUuid uuid = QUuid::createUuid();

            QUrl url = QUrl::fromLocalFile(UBPersistenceManager::persistenceManager()
                                           ->addVideoFileToDocument(mActiveDocument, sourceUrl, pData, uuid));

            videoItem = mActiveScene->addVideo(url, false, pPos);

            videoItem->setSourceUrl(sourceUrl);
            videoItem->setUuid(uuid);
        }
        else
        {
            videoItem = addVideo(sourceUrl, false, pPos);
        }

        if(videoItem){
            connect(this, SIGNAL(activeSceneChanged()), videoItem, SLOT(activeSceneChanged()));
        }

        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    }
    else if (mimeType.startsWith("audio/"))
    {
        qDebug() << "accepting mime type" << mimeType << "as audio";

        UBGraphicsAudioItem *audioItem = 0;

        if (pData.length() > 0)
        {
            QUuid uuid = QUuid::createUuid();

            QUrl url = QUrl::fromLocalFile(UBPersistenceManager::persistenceManager()
                                           ->addAudioFileToDocument(mActiveDocument, sourceUrl, pData, uuid));

            audioItem = mActiveScene->addAudio(url, false, pPos);

            audioItem->setSourceUrl(sourceUrl);
            audioItem->setUuid(uuid);
        }
        else
        {
            audioItem = addAudio(sourceUrl, false, pPos);
        }

        if(audioItem){
            connect(this, SIGNAL(activeSceneChanged()), audioItem, SLOT(activeSceneChanged()));
        }

        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    }

    else if (mimeType.startsWith("application/x-shockwave-flash"))
    {
        bool acceptFlash = true;

#ifdef Q_WS_X11
         acceptFlash = false;
#endif
        if (acceptFlash)
        {
            qDebug() << "accepting mime type" << mimeType << "as flash";

            QString sUrl = sourceUrl.toString();

            if (sUrl.startsWith("file://") || sUrl.startsWith("/"))
            {
                sUrl = sourceUrl.toLocalFile();
            }

            QTemporaryFile* eduMediaFile = 0;

            if (sUrl.toLower().contains("edumedia-sciences.com"))
            {
                eduMediaFile = new QTemporaryFile("XXXXXX.swf");
                if (eduMediaFile->open())
                {
                    eduMediaFile->write(pData);
                    QFileInfo fi(*eduMediaFile);
                    sUrl = fi.absoluteFilePath();
                }
            }

            QSize size;

            if (pSize.height() > 0 && pSize.width() > 0)
                size = pSize;
            else
                size = mActiveScene->nominalSize() * .8;

            QString widgetUrl = UBW3CWidget::createNPAPIWrapper(sUrl, mimeType, size);

            if (widgetUrl.length() > 0)
            {
                UBGraphicsWidgetItem *widgetItem = mActiveScene->addW3CWidget(QUrl::fromLocalFile(widgetUrl), pPos);

                widgetItem->setSourceUrl(sourceUrl);

                UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
            }

            if (eduMediaFile)
                delete eduMediaFile;
        }
        else
        {
            UBApplication::showMessage(tr("Flash is not supported on Uniboard Linux"));
        }
    }
    else if (mimeType.startsWith("application/pdf"))
    {
        qDebug() << "accepting mime type" << mimeType << "as PDF";
        qDebug() << "pdf data length: " << pData.size();
        qDebug() << "sourceurl : " + sourceUrl.toString();
        int result = 0;
        if(!sourceUrl.isEmpty()){
            QFile sourceFile(sourceUrl.toLocalFile());
            result = UBDocumentManager::documentManager()->addFileToDocument(mActiveDocument, sourceFile);
        }
        else if(pData.size()){
            QTemporaryFile pdfFile("XXXXXX.pdf");
            if (pdfFile.open())
            {
                pdfFile.write(pData);
                result = UBDocumentManager::documentManager()->addFileToDocument(mActiveDocument, pdfFile);
                pdfFile.close();
            }
        }

        if (result){
            mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        }
    }
    else if (mimeType.startsWith("application/vnd.mnemis-uniboard-tool"))
    {
        qDebug() << "accepting mime type" << mimeType << "as Uniboard Tool";

        if (sourceUrl.toString() == UBToolsManager::manager()->compass.id)
        {
            mActiveScene->addCompass(pPos);
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
        else if (sourceUrl.toString() == UBToolsManager::manager()->ruler.id)
        {
            mActiveScene->addRuler(pPos);
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
        else if (sourceUrl.toString() == UBToolsManager::manager()->protractor.id)
        {
            mActiveScene->addProtractor(pPos);
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
        else if (sourceUrl.toString() == UBToolsManager::manager()->mask.id)
        {
            mActiveScene->addMask();
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
        }
        else
        {
            UBApplication::showMessage(tr("Unknown tool type %1").arg(sourceUrl.toString()));
        }
    }
    else if (sourceUrl.toString().contains("edumedia-sciences.com"))
    {
        qDebug() << "accepting url " << sourceUrl.toString() << "as eduMedia content";

        QTemporaryFile eduMediaZipFile("XXXXXX.edumedia");
        if (eduMediaZipFile.open())
        {
            eduMediaZipFile.write(pData);
            eduMediaZipFile.close();

            QString tempDir = UBFileSystemUtils::createTempDir("uniboard-edumedia");

            UBFileSystemUtils::expandZipToDir(eduMediaZipFile, tempDir);

            QDir appDir(tempDir);

            foreach(QString subDirName, appDir.entryList(QDir::AllDirs))
            {
                QDir subDir(tempDir + "/" + subDirName + "/contents");

                foreach(QString fileName, subDir.entryList(QDir::Files))
                {
                    if (fileName.toLower().endsWith(".swf"))
                    {
                        QString swfFile = tempDir + "/" + subDirName + "/contents/" + fileName;

                        QSize size;

                        if (pSize.height() > 0 && pSize.width() > 0)
                            size = pSize;
                        else
                            size = mActiveScene->nominalSize() * .8;

                        QString widgetUrl = UBW3CWidget::createNPAPIWrapper(swfFile, "application/x-shockwave-flash", size);

                        if (widgetUrl.length() > 0)
                        {
                            UBGraphicsWidgetItem *widgetItem = mActiveScene->addW3CWidget(QUrl::fromLocalFile(widgetUrl), pPos);

                            widgetItem->setSourceUrl(sourceUrl);

                            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
                        }
                    }
                }
            }
        }
    }
    else
    {
        UBApplication::showMessage(tr("Unknown content type %1").arg(pContentTypeHeader));
        qWarning() << "ignoring mime type" << pContentTypeHeader ;
    }
}


void UBBoardController::setActiveDocumentScene(UBDocumentProxy* pDocumentProxy, const int pSceneIndex)
{
    saveViewState();

    bool documentChange = mActiveDocument != pDocumentProxy;

    int index = pSceneIndex;
    int sceneCount = pDocumentProxy->pageCount();
    if (index >= sceneCount && sceneCount > 0)
        index = sceneCount - 1;

    UBGraphicsScene* targetScene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, index);

    bool sceneChange = targetScene != mActiveScene;

    if (targetScene)
    {
        if(sceneChange)
            emit activeSceneWillChange();

        UBApplication::undoStack->clear();

        mActiveScene = targetScene;
        mActiveDocument = pDocumentProxy;
        mActiveSceneIndex = index;

        updateSystemScaleFactor();

        mControlView->setScene(mActiveScene);
        mDisplayView->setScene(mActiveScene);

        pDocumentProxy->setDefaultDocumentSize(mActiveScene->nominalSize());
        updatePageSizeState();

        adjustDisplayViews();

        UBSettings::settings()->setDarkBackground(mActiveScene->isDarkBackground());
        UBSettings::settings()->setCrossedBackground(mActiveScene->isCrossedBackground());
    }

    selectionChanged();

    updateBackgroundActionsState(mActiveScene->isDarkBackground(), mActiveScene->isCrossedBackground());
    updateBackgroundState();

    if(documentChange)
    {
        emit activeDocumentChanged();
        UBGraphicsTextItem::lastUsedTextColor = QColor();
    }

    // Notify the navigator palette that the document has changed
    mPaletteManager->navigatorPalette()->setDocument(pDocumentProxy);

    if (sceneChange)
        emit activeSceneChanged();
}


void UBBoardController::adjustDisplayViews()
{
    if (UBApplication::applicationController)
    {
        UBApplication::applicationController->adjustDisplayView();
        UBApplication::applicationController->adjustPreviousViews(mActiveSceneIndex, mActiveDocument);
    }
}


void UBBoardController::changeBackground(bool isDark, bool isCrossed)
{
    bool currentIsDark = mActiveScene->isDarkBackground();
    bool currentIsCrossed = mActiveScene->isCrossedBackground();

    if ((isDark != currentIsDark) || (currentIsCrossed != isCrossed))
    {
        UBSettings::settings()->setDarkBackground(isDark);
        UBSettings::settings()->setCrossedBackground(isCrossed);

        mActiveScene->setBackground(isDark, isCrossed);

        updateBackgroundState();

        emit backgroundChanged();
    }
}


void UBBoardController::boardViewResized(QResizeEvent* event)
{
    Q_UNUSED(event);

    int innerMargin = UBSettings::boardMargin;
    int userHeight = mControlContainer->height() - (2 * innerMargin);

    mMessageWindow->move(innerMargin, innerMargin + userHeight - mMessageWindow->height());
    mMessageWindow->adjustSizeAndPosition();

    UBApplication::applicationController->initViewState(
                mControlView->horizontalScrollBar()->value(),
                mControlView->verticalScrollBar()->value());

    updateSystemScaleFactor();

    mControlView->centerOn(0,0);

    mPaletteManager->containerResized();


}


void UBBoardController::documentWillBeDeleted(UBDocumentProxy* pProxy)
{
    if (mActiveDocument == pProxy)
    {
        if (!mIsClosing)
            setActiveDocumentScene(UBPersistenceManager::persistenceManager()->createDocument());
    }
}


void UBBoardController::showMessage(const QString& message, bool showSpinningWheel)
{
    mMessageWindow->showMessage(message, showSpinningWheel);
}


void UBBoardController::hideMessage()
{
    mMessageWindow->hideMessage();
}


void UBBoardController::setDisabled(bool disable)
{
    mMainWindow->boardToolBar->setDisabled(disable);
    mControlView->setDisabled(disable);
}


void UBBoardController::selectionChanged()
{
    updateActionStates();
}


void UBBoardController::undoRedoStateChange(bool canUndo)
{
    Q_UNUSED(canUndo);

    mMainWindow->actionUndo->setEnabled(UBApplication::undoStack->canUndo());
    mMainWindow->actionRedo->setEnabled(UBApplication::undoStack->canRedo());

    updateActionStates();
}


void UBBoardController::updateActionStates()
{
    mMainWindow->actionBack->setEnabled(mActiveDocument && (mActiveSceneIndex > 0));
    mMainWindow->actionForward->setEnabled(mActiveDocument && (mActiveSceneIndex < mActiveDocument->pageCount() - 1));
    mMainWindow->actionErase->setEnabled(mActiveScene && !mActiveScene->isEmpty());
}


UBDocumentProxy* UBBoardController::activeDocument() const
{
    return mActiveDocument;
}


UBGraphicsScene* UBBoardController::activeScene() const
{
    return mActiveScene;
}


int UBBoardController::activeSceneIndex() const
{
    return mActiveSceneIndex;
}


void UBBoardController::documentSceneChanged(UBDocumentProxy* pDocumentProxy, int pIndex)
{
    Q_UNUSED(pIndex);

    if(mActiveDocument == pDocumentProxy)
    {
        setActiveDocumentScene(mActiveDocument, mActiveSceneIndex);
    }
}


void UBBoardController::closing()
{
    mIsClosing = true;

    lastWindowClosed();
}


void UBBoardController::lastWindowClosed()
{
    if (!mCleanupDone)
    {
        if (mActiveDocument->pageCount() == 1 && (!mActiveScene || mActiveScene->isEmpty()))
        {
            UBPersistenceManager::persistenceManager()->deleteDocument(mActiveDocument);
        }
        else
        {
            persistCurrentScene();
        }

        UBPersistenceManager::persistenceManager()->purgeEmptyDocuments();

        mCleanupDone = true;
    }
}



void UBBoardController::setColorIndex(int pColorIndex)
{
    UBDrawingController::drawingController()->setColorIndex(pColorIndex);

    if (UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Marker &&
            UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Line &&
            UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Text &&
            UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Selector)
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Pen);
    }

    if (UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Pen ||
            UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Line ||
            UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Text ||
            UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector)
    {
        mPenColorOnDarkBackground = UBSettings::settings()->penColors(true).at(pColorIndex);
        mPenColorOnLightBackground = UBSettings::settings()->penColors(false).at(pColorIndex);

        if (UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector)
        {
            // If we are in mode board, then do that
            if(UBApplication::applicationController->displayMode() == UBApplicationController::Board)
            {
                UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Pen);
                mMainWindow->actionPen->setChecked(true);
            }
        }

        emit penColorChanged();
    }
    else if (UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Marker)
    {
        mMarkerColorOnDarkBackground = UBSettings::settings()->markerColors(true).at(pColorIndex);
        mMarkerColorOnLightBackground = UBSettings::settings()->markerColors(false).at(pColorIndex);
    }
}

void UBBoardController::colorPaletteChanged()
{
    mPenColorOnDarkBackground = UBSettings::settings()->penColor(true);
    mPenColorOnLightBackground = UBSettings::settings()->penColor(false);
    mMarkerColorOnDarkBackground = UBSettings::settings()->markerColor(true);
    mMarkerColorOnLightBackground = UBSettings::settings()->markerColor(false);
}


qreal UBBoardController::currentZoom()
{
    if (mControlView)
        return mControlView->viewportTransform().m11() / mSystemScaleFactor;
    else
        return 1.0;
}


UBToolWidget* UBBoardController::addTool(const QUrl& toolUrl)
{
    return addTool(toolUrl, mControlView->mapToScene(mControlView->rect().center()));
}


UBToolWidget* UBBoardController::addTool(const QUrl& toolUrl, QPointF scenePos)
{
    UBToolWidget *toolWidget = new UBToolWidget(toolUrl, mMainWindow); // Deleted in UBBoardController::removeTool
    QPoint pos = mControlView->mapToGlobal(mControlView->mapFromScene(scenePos));
    pos -= QPoint(toolWidget->width() / 2, toolWidget->height() / 2);

    toolWidget->move(pos);

    mTools.append(toolWidget);

    toolWidget->show();

    return toolWidget;
}


void UBBoardController::removeTool(UBToolWidget* toolWidget)
{
    toolWidget->hide();

    mTools.removeAll(toolWidget);

    delete toolWidget;
}


void UBBoardController::hide()
{
    UBApplication::mainWindow->actionLibrary->setChecked(false);

    controlViewHidden();
}


void UBBoardController::show()
{
    UBApplication::mainWindow->actionLibrary->setChecked(false);

    controlViewShown();
}


void UBBoardController::persistCurrentScene()
{
    if(UBPersistenceManager::persistenceManager()
            && mActiveDocument && mActiveScene
            && (mActiveSceneIndex >= 0))
    {
        emit activeSceneWillBePersisted();

        UBPersistenceManager::persistenceManager()->persistDocumentScene(mActiveDocument, mActiveScene, mActiveSceneIndex);
        UBMetadataDcSubsetAdaptor::persist(mActiveDocument);
    }
}


void UBBoardController::updateSystemScaleFactor()
{
    qreal newScaleFactor = 1.0;

    if (mActiveScene)
    {
        QSize pageNominalSize = mActiveScene->nominalSize();
        QSize controlSize = controlViewport();

        qreal hFactor = ((qreal)controlSize.width()) / ((qreal)pageNominalSize.width());
        qreal vFactor = ((qreal)controlSize.height()) / ((qreal)pageNominalSize.height());

        newScaleFactor = qMin(hFactor, vFactor);
    }

    if (mSystemScaleFactor != newScaleFactor)
    {
        mSystemScaleFactor = newScaleFactor;
        emit systemScaleFactorChanged(newScaleFactor);
    }

    UBGraphicsScene::SceneViewState viewState = mActiveScene->viewState();

    QTransform scalingTransform;

    qreal scaleFactor = viewState.zoomFactor * mSystemScaleFactor;
    scalingTransform.scale(scaleFactor, scaleFactor);

    mControlView->setTransform(scalingTransform);
    mControlView->horizontalScrollBar()->setValue(viewState.horizontalPosition);
    mControlView->verticalScrollBar()->setValue(viewState.verticalPostition);

}


void UBBoardController::setWidePageSize(bool checked)
{
    Q_UNUSED(checked);
    QSize newSize = UBSettings::settings()->documentSizes.value(DocumentSizeRatio::Ratio16_9);

    if (mActiveScene->nominalSize() != newSize)
    {
        UBPageSizeUndoCommand* uc = new UBPageSizeUndoCommand(mActiveScene, mActiveScene->nominalSize(), newSize);
        UBApplication::undoStack->push(uc);

        setPageSize(newSize);
    }
}


void UBBoardController::setRegularPageSize(bool checked)
{
    Q_UNUSED(checked);
    QSize newSize = UBSettings::settings()->documentSizes.value(DocumentSizeRatio::Ratio4_3);

    if (mActiveScene->nominalSize() != newSize)
    {
        UBPageSizeUndoCommand* uc = new UBPageSizeUndoCommand(mActiveScene, mActiveScene->nominalSize(), newSize);
        UBApplication::undoStack->push(uc);

        setPageSize(newSize);
    }
}


void UBBoardController::setPageSize(QSize newSize)
{
    if (mActiveScene->nominalSize() != newSize)
    {
        mActiveScene->setNominalSize(newSize);

        saveViewState();

        updateSystemScaleFactor();
        updatePageSizeState();
        adjustDisplayViews();
        mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
    }
}


void UBBoardController::updatePageSizeState()
{
    if (mActiveScene->nominalSize() == UBSettings::settings()->documentSizes.value(DocumentSizeRatio::Ratio16_9))
    {
        mMainWindow->actionWidePageSize->setChecked(true);
    }
    else if(mActiveScene->nominalSize() == UBSettings::settings()->documentSizes.value(DocumentSizeRatio::Ratio4_3))
    {
        mMainWindow->actionRegularPageSize->setChecked(true);
    }
    else
    {
        mMainWindow->actionCustomPageSize->setChecked(true);
    }
}


void UBBoardController::saveViewState()
{
    if (mActiveScene)
    {
        mActiveScene->setViewState(UBGraphicsScene::SceneViewState(currentZoom(),
                                                                   mControlView->horizontalScrollBar()->value(),
                                                                   mControlView->verticalScrollBar()->value()));
    }
}


void UBBoardController::updateBackgroundState()
{
    //adjust background style
    QString newBackgroundStyle;

    if (mActiveScene && mActiveScene->isDarkBackground())
    {
        newBackgroundStyle ="QWidget {background-color: #0E0E0E}";
    }
    else
    {
        newBackgroundStyle ="QWidget {background-color: #F1F1F1}";
    }

}


void UBBoardController::stylusToolChanged(int tool)
{
    Q_UNUSED(tool);

    updateBackgroundState();
}


QUrl UBBoardController::expandWidgetToTempDir(const QByteArray& pZipedData, const QString& ext)
{
    QUrl widgetUrl;
    QTemporaryFile tmp;

    if (tmp.open())
    {
        tmp.write(pZipedData);
        tmp.flush();
        tmp.close();

        QString tmpDir = UBFileSystemUtils::createTempDir() + "." + ext;

        if (UBFileSystemUtils::expandZipToDir(tmp, tmpDir))
        {
            widgetUrl = QUrl::fromLocalFile(tmpDir);
        }
    }

    return widgetUrl;
}


void UBBoardController::grabScene(const QRectF& pSceneRect)
{
    if (mActiveScene)
    {
        QImage image(pSceneRect.width(), pSceneRect.height(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QRectF targetRect(0, 0, pSceneRect.width(), pSceneRect.height());
        QPainter painter(&image);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::Antialiasing);

        mActiveScene->setRenderingContext(UBGraphicsScene::NonScreen);
        mActiveScene->setRenderingQuality(UBItem::RenderingQualityHigh);

        mActiveScene->render(&painter, targetRect, pSceneRect);

        mActiveScene->setRenderingContext(UBGraphicsScene::Screen);
        mActiveScene->setRenderingQuality(UBItem::RenderingQualityNormal);

        mPaletteManager->addItem(QPixmap::fromImage(image));
        mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
    }
}


void UBBoardController::controlViewHidden()
{
    foreach(UBToolWidget* tool, mTools)
    {
        tool->hide();
    }
}


void UBBoardController::controlViewShown()
{
    foreach(UBToolWidget* tool, mTools)
    {
        tool->show();
    }
}


UBGraphicsVideoItem* UBBoardController::addVideo(const QUrl& pSourceUrl, bool startPlay, const QPointF& pos)
{
    QUuid uuid = QUuid::createUuid();
    QUrl concreteUrl = pSourceUrl;
    concreteUrl = QUrl::fromLocalFile(UBPersistenceManager::persistenceManager()
                                      ->addVideoFileToDocument(mActiveDocument, pSourceUrl.toLocalFile(), uuid));

    UBGraphicsVideoItem* vi = mActiveScene->addVideo(concreteUrl, startPlay, pos);
    mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    if (vi) {
        vi->setUuid(uuid);
        vi->setSourceUrl(pSourceUrl);
    }

    return vi;

}

UBGraphicsAudioItem* UBBoardController::addAudio(const QUrl& pSourceUrl, bool startPlay, const QPointF& pos)
{
    QUuid uuid = QUuid::createUuid();
    QUrl concreteUrl = pSourceUrl;

    concreteUrl = QUrl(UBPersistenceManager::persistenceManager()
                       ->addAudioFileToDocument(mActiveDocument, pSourceUrl.toLocalFile(), uuid));

    UBGraphicsAudioItem* vi = mActiveScene->addAudio(concreteUrl, startPlay, pos);
    mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    if (vi){
        vi->setUuid(uuid);
        vi->setSourceUrl(pSourceUrl);
    }

    return vi;

}


void UBBoardController::cut()
{
    QList<UBItem*> selected;

    foreach(QGraphicsItem* gi, mActiveScene->selectedItems())
    {
        UBItem* ubItem = dynamic_cast<UBItem*>(gi);
        UBGraphicsItem *ubGi =  dynamic_cast<UBGraphicsItem*>(gi);

        if (ubItem && ubGi && !mActiveScene->tools().contains(gi))
        {
            selected << ubItem->deepCopy();
            ubGi->remove();
        }
    }

    if (selected.size() > 0)
    {
        QClipboard *clipboard = QApplication::clipboard();

        UBMimeDataGraphicsItem*  mimeGi = new UBMimeDataGraphicsItem(selected);

        mimeGi->setData(UBApplication::mimeTypeUniboardPageItem, QByteArray());
        clipboard->setMimeData(mimeGi);

        mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
    }
}


void UBBoardController::copy()
{
    QList<UBItem*> selected;

    foreach(QGraphicsItem* gi, mActiveScene->selectedItems())
    {
        UBItem* ubItem = dynamic_cast<UBItem*>(gi);

        if (ubItem && !mActiveScene->tools().contains(gi))
            selected << ubItem->deepCopy();
    }

    if (selected.size() > 0)
    {
        QClipboard *clipboard = QApplication::clipboard();

        UBMimeDataGraphicsItem*  mimeGi = new UBMimeDataGraphicsItem(selected);

        mimeGi->setData(UBApplication::mimeTypeUniboardPageItem, QByteArray());
        clipboard->setMimeData(mimeGi);

    }
}


void UBBoardController::paste()
{
    QClipboard *clipboard = QApplication::clipboard();
    QPointF pos(0, 0);
    processMimeData(clipboard->mimeData(), pos);

    mActiveDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
}


void UBBoardController::processMimeData(const QMimeData* pMimeData, const QPointF& pPos)
{
    if (pMimeData->hasFormat(UBApplication::mimeTypeUniboardPage))
    {
        const UBMimeData* mimeData = qobject_cast <const UBMimeData*>(pMimeData);

        if (mimeData)
        {
            int previousActiveSceneIndex = activeSceneIndex();
            int previousPageCount = activeDocument()->pageCount();

            foreach (UBMimeDataItem sourceItem, mimeData->items())
                addScene(sourceItem.documentProxy(), sourceItem.sceneIndex(), true);

            if (activeDocument()->pageCount() < previousPageCount + mimeData->items().count())
                setActiveDocumentScene(activeDocument(), previousActiveSceneIndex);
            else
                setActiveDocumentScene(activeDocument(), previousActiveSceneIndex + 1);

            return;
        }
    }

    if (pMimeData->hasFormat(UBApplication::mimeTypeUniboardPageItem))
    {
        const UBMimeDataGraphicsItem* mimeData = qobject_cast <const UBMimeDataGraphicsItem*>(pMimeData);

        if (mimeData)
        {
            foreach(UBItem* item, mimeData->items())
            {
                QGraphicsItem* gi = dynamic_cast<QGraphicsItem*>(item->deepCopy());

                if (gi)
                {
                    gi->setZValue(mActiveScene->getNextObjectZIndex());
                    mActiveScene->addItem(gi);
                    gi->setPos(gi->pos() + QPointF(50, 50));
                }
            }

            return;
        }
    }

    if (pMimeData->hasUrls())
    {
        QList<QUrl> urls = pMimeData->urls();

        int index = 0;

        foreach(const QUrl url, urls){
            QPointF pos(pPos + QPointF(index * 15, index * 15));
            downloadURL(url, pos);
            index++;
        }

        return;
    }

    if (pMimeData->hasImage())
    {
        QImage img = qvariant_cast<QImage> (pMimeData->imageData());
        QPixmap pix = QPixmap::fromImage(img);

        // validate that the image is really an image, webkit does not fill properly the image mime data
        if (pix.width() != 0 && pix.height() != 0)
        {
            mActiveScene->addPixmap(pix, pPos, 1.);
            return;
        }
    }

    if (pMimeData->hasText())
    {
        mActiveScene->addText(pMimeData->text(), pPos);
    }
}


void UBBoardController::togglePodcast(bool checked)
{
    if (UBPodcastController::instance())
        UBPodcastController::instance()->toggleRecordingPalette(checked);
}


void UBBoardController::moveGraphicsWidgetToControlView(UBGraphicsWidgetItem* graphicsWidget)
{
    QPoint controlViewPos = mControlView->mapFromScene(graphicsWidget->sceneBoundingRect().center());

    graphicsWidget->setSelected(false);

    UBAbstractWidget *aw = graphicsWidget->widgetWebView();
    graphicsWidget->setWidget(0);

    UBToolWidget *toolWidget = new UBToolWidget(aw, mControlContainer);

    graphicsWidget->scene()->removeItem(graphicsWidget); // TODO UB 4.6 probably leaking the frame

    toolWidget->centerOn(mControlView->mapTo(mControlContainer, controlViewPos));

    toolWidget->show();
}


void UBBoardController::moveToolWidgetToScene(UBToolWidget* toolWidget)
{
    int xIsOdd = toolWidget->width() % 2;
    int yIsOdd = toolWidget->height() % 2;

    QPoint mainWindowCenter = toolWidget->mapTo(mMainWindow, QPoint(toolWidget->width(), toolWidget->height()) / 2);

    UBAbstractWidget* webWidget = toolWidget->webWidget();
    webWidget->setParent(0);

    UBGraphicsWidgetItem* graphicsWidget = 0;

    UBW3CWidget* w3cWidget = qobject_cast<UBW3CWidget*>(webWidget);
    if (w3cWidget)
    {
        graphicsWidget = new UBGraphicsW3CWidgetItem(w3cWidget);
    }
    else
    {
        UBAppleWidget* appleWidget = qobject_cast<UBAppleWidget*>(webWidget);
        if (appleWidget)
        {
            graphicsWidget = new UBGraphicsAppleWidgetItem(appleWidget);
        }
    }

    QPoint controlViewCenter = mControlView->mapFrom(mMainWindow, mainWindowCenter);
    QPointF scenePos = mControlView->mapToScene(controlViewCenter) + QPointF(xIsOdd * 0.5, yIsOdd * 0.5);

    mActiveScene->addGraphicsWidget(graphicsWidget, scenePos);

    toolWidget->hide();
    toolWidget->deleteLater();
}


void UBBoardController::updateBackgroundActionsState(bool isDark, bool isCrossed)
{
    if (isDark && !isCrossed)
        mMainWindow->actionPlainDarkBackground->setChecked(true);
    else if (isDark && isCrossed)
        mMainWindow->actionCrossedDarkBackground->setChecked(true);
    else if (!isDark && isCrossed)
        mMainWindow->actionCrossedLightBackground->setChecked(true);
    else
        mMainWindow->actionPlainLightBackground->setChecked(true);
}


void UBBoardController::addItem()
{
    QString defaultPath = UBSettings::settings()->lastImportToLibraryPath->get().toString();

    QString extensions;
    foreach(QString ext, UBSettings::imageFileExtensions)
    {
        extensions += " *.";
        extensions += ext;
    }

    QString filename = QFileDialog::getOpenFileName(mControlContainer, tr("Add Item"),
                                                    defaultPath,
                                                    tr("All Supported (%1)").arg(extensions));

    if (filename.length() > 0)
    {
        mPaletteManager->addItem(QUrl::fromLocalFile(filename));
        QFileInfo source(filename);
        UBSettings::settings()->lastImportToLibraryPath->set(QVariant(source.absolutePath()));
    }
}


void UBBoardController::importPage()
{
    int pageCount = mActiveDocument->pageCount();

    if (UBApplication::documentController->addFileToDocument(mActiveDocument))
    {
        setActiveDocumentScene(mActiveDocument, pageCount);
    }
}





