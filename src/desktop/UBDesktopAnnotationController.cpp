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




#include "UBDesktopAnnotationController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"

#include "web/UBWebController.h"

#include "gui/UBMainWindow.h"

#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsPolygonItem.h"

#include "UBCustomCaptureWindow.h"
#include "UBDesktopPalette.h"
#include "UBDesktopPropertyPalette.h"

#include "gui/UBKeyboardPalette.h"
#include "gui/UBResources.h"

#include "core/memcheck.h"

UBDesktopAnnotationController::UBDesktopAnnotationController(QObject *parent, UBRightPalette* rightPalette)
        : QObject(parent)
        , mTransparentDrawingView(0)
        , mTransparentDrawingScene(0)
        , mDesktopPalette(NULL)
        , mDesktopPenPalette(NULL)
        , mDesktopMarkerPalette(NULL)
        , mDesktopEraserPalette(NULL)
        , mRightPalette(rightPalette)
        , mWindowPositionInitialized(false)
        , mIsFullyTransparent(false)
        , mDesktopToolsPalettePositioned(false)
        , mPendingPenButtonPressed(false)
        , mPendingMarkerButtonPressed(false)
        , mPendingEraserButtonPressed(false)
        , mbArrowClicked(false)
        , mCustomCaptureClicked(false)
        , mBoardStylusTool(UBDrawingController::drawingController()->stylusTool())
        , mDesktopStylusTool(UBDrawingController::drawingController()->stylusTool())
{

    mTransparentDrawingView = new UBBoardView(UBApplication::boardController, static_cast<QWidget*>(0), false, true); // deleted in UBDesktopAnnotationController::destructor
    mTransparentDrawingView->setAttribute(Qt::WA_TranslucentBackground, true);
#ifdef Q_OS_OSX
    // didn't find the equivalent in Qt6
    #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        mTransparentDrawingView->setAttribute(Qt::WA_MacNoShadow, true);
    #endif
#endif //Q_OS_OSX

    mTransparentDrawingView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Window | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    mTransparentDrawingView->setCacheMode(QGraphicsView::CacheNone);
    mTransparentDrawingView->resize(UBApplication::displayManager->screenSize(ScreenRole::Desktop));

    mTransparentDrawingView->setMouseTracking(true);

    mTransparentDrawingView->setAcceptDrops(true);

    QString backgroundStyle = "QWidget {background-color: rgba(127, 127, 127, 0)}";
    mTransparentDrawingView->setStyleSheet(backgroundStyle);

    mTransparentDrawingScene = std::make_shared<UBGraphicsScene>(nullptr, false);
    updateColors();

    mTransparentDrawingView->setScene(mTransparentDrawingScene.get());
    mTransparentDrawingScene->setDrawingMode(true);

    mDesktopPalette = new UBDesktopPalette(mTransparentDrawingView, rightPalette); 
    // This was not fix, parent reverted
    // FIX #633: The palette must be 'floating' in order to stay on top of the library palette

    if (UBPlatformUtils::hasVirtualKeyboard())
    {
        connect( UBApplication::boardController->paletteManager()->mKeyboardPalette, SIGNAL(keyboardActivated(bool)), 
                 mTransparentDrawingView, SLOT(virtualKeyboardActivated(bool)));

#ifdef UB_REQUIRES_MASK_UPDATE
        connect(UBApplication::boardController->paletteManager()->mKeyboardPalette, SIGNAL(moved(QPoint)), this, SLOT(refreshMask()));
        connect(UBApplication::mainWindow->actionVirtualKeyboard, SIGNAL(triggered(bool)), this, SLOT(refreshMask()));
        connect(mDesktopPalette,SIGNAL(refreshMask()), this, SLOT(refreshMask()));
#endif
    }

    connect(mDesktopPalette, SIGNAL(uniboardClick()), this, SLOT(goToUniboard()));
    connect(mDesktopPalette, SIGNAL(customClick()), this, SLOT(customCapture()));
    connect(mDesktopPalette, SIGNAL(screenClick()), this, SLOT(screenCapture()));
    connect(UBApplication::mainWindow->actionPointer, SIGNAL(triggered()), this, SLOT(onToolClicked()));
    connect(UBApplication::mainWindow->actionSelector, SIGNAL(triggered()), this, SLOT(onToolClicked()));
    connect(mDesktopPalette, SIGNAL(maximized()), this, SLOT(onDesktopPaletteMaximized()));
    connect(mDesktopPalette, SIGNAL(minimizeStart(eMinimizedLocation)), this, SLOT(onDesktopPaletteMinimize()));
    connect(mDesktopPalette, SIGNAL(mouseEntered()), mTransparentDrawingScene.get(), SLOT(hideTool()));
    connect(mRightPalette, SIGNAL(mouseEntered()), mTransparentDrawingScene.get(), SLOT(hideTool()));
    connect(mRightPalette, SIGNAL(pageSelectionChangedRequired()), this, SLOT(updateBackground()));

    connect(mTransparentDrawingView, SIGNAL(resized(QResizeEvent*)), this, SLOT(onTransparentWidgetResized()));


    connect(UBDrawingController::drawingController(), SIGNAL(stylusToolChanged(int)), this, SLOT(stylusToolChanged(int)));

    // Add the desktop associated palettes
    mDesktopPenPalette = new UBDesktopPenPalette(mTransparentDrawingView, rightPalette, "desktop");

    connect(mDesktopPalette, SIGNAL(maximized()), mDesktopPenPalette, SLOT(onParentMaximized()));
    connect(mDesktopPalette, SIGNAL(minimizeStart(eMinimizedLocation)), mDesktopPenPalette, SLOT(onParentMinimized()));

    mDesktopMarkerPalette = new UBDesktopMarkerPalette(mTransparentDrawingView, rightPalette, "desktop");
    mDesktopEraserPalette = new UBDesktopEraserPalette(mTransparentDrawingView, rightPalette, "desktop");

    mDesktopPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopPenPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopMarkerPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopEraserPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);


    // Hack : the size of the property palettes is computed the first time the palette is visible
    //        In order to prevent palette overlap on if the desktop palette is on the right of the
    //        screen, a setVisible(true) followed by a setVisible(false) is done.
    mDesktopPenPalette->setVisible(true);
    mDesktopMarkerPalette->setVisible(true);
    mDesktopEraserPalette->setVisible(true);
    mDesktopPenPalette->setVisible(false);
    mDesktopMarkerPalette->setVisible(false);
    mDesktopEraserPalette->setVisible(false);

    connect(UBApplication::mainWindow->actionEraseDesktopAnnotations, SIGNAL(triggered()), this, SLOT(eraseDesktopAnnotations()));
    connect(UBApplication::boardController, SIGNAL(backgroundChanged()), this, SLOT(updateColors()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(updateColors()));
    connect(&mHoldTimerPen, SIGNAL(timeout()), this, SLOT(penActionReleased()));
    connect(&mHoldTimerMarker, SIGNAL(timeout()), this, SLOT(markerActionReleased()));
    connect(&mHoldTimerEraser, SIGNAL(timeout()), this, SLOT(eraserActionReleased()));

#ifdef UB_REQUIRES_MASK_UPDATE
    connect(mDesktopPalette, SIGNAL(moving()), this, SLOT(refreshMask()));
    connect(UBApplication::boardController->paletteManager()->rightPalette(), SIGNAL(resized()), this, SLOT(refreshMask()));
    connect(UBApplication::boardController->paletteManager()->addItemPalette(), SIGNAL(closed()), this, SLOT(refreshMask()));
#endif
    onDesktopPaletteMaximized();

    // FIX #633: Ensure that these palettes stay on top of the other elements
    //mDesktopEraserPalette->raise();
    //mDesktopMarkerPalette->raise();
    //mDesktopPenPalette->raise();
}

UBDesktopAnnotationController::~UBDesktopAnnotationController()
{
    delete mTransparentDrawingView;
}

void UBDesktopAnnotationController::updateColors(){
    if(UBApplication::boardController->activeScene()->isDarkBackground()){
        mTransparentDrawingScene->setSceneBackground(true, nullptr);
    }else{
        mTransparentDrawingScene->setSceneBackground(false, nullptr);
    }
}

UBDesktopPalette* UBDesktopAnnotationController::desktopPalette()
{
    return mDesktopPalette;
}

QPainterPath UBDesktopAnnotationController::desktopPalettePath() const
{
    QPainterPath result;
    if (mDesktopPalette && mDesktopPalette->isVisible()) {
        result.addRect(mDesktopPalette->geometry());
    }
    if (mDesktopPenPalette && mDesktopPenPalette->isVisible()) {
        result.addRect(mDesktopPenPalette->geometry());
    }
    if (mDesktopMarkerPalette && mDesktopMarkerPalette->isVisible()) {
        result.addRect(mDesktopMarkerPalette->geometry());
    }
    if (mDesktopEraserPalette && mDesktopEraserPalette->isVisible()) {
        result.addRect(mDesktopEraserPalette->geometry());
    }

    return result;
}

/**
 * \brief Toggle the visibility of the pen associated palette
 * @param checked as the visibility state
 */
void UBDesktopAnnotationController::desktopPenActionToggled(bool checked)
{
    setAssociatedPalettePosition(mDesktopPenPalette, "actionPen");
    mDesktopPenPalette->setVisible(checked);
    mDesktopMarkerPalette->setVisible(false);
    mDesktopEraserPalette->setVisible(false);
}

/**
 * \brief Toggle the visibility of the marker associated palette
 * @param checked as the visibility state
 */
void UBDesktopAnnotationController::desktopMarkerActionToggled(bool checked)
{
    setAssociatedPalettePosition(mDesktopMarkerPalette, "actionMarker");
    mDesktopMarkerPalette->setVisible(checked);
    mDesktopPenPalette->setVisible(false);
    mDesktopEraserPalette->setVisible(false);
}

/**
 * \brief Toggle the visibility of the eraser associated palette
 * @param checked as the visibility state
 */
void UBDesktopAnnotationController::desktopEraserActionToggled(bool checked)
{
    setAssociatedPalettePosition(mDesktopEraserPalette, "actionEraser");
    mDesktopEraserPalette->setVisible(checked);
    mDesktopPenPalette->setVisible(false);
    mDesktopMarkerPalette->setVisible(false);
}

/**
 * \brief Set the location of the properties palette
 * @param palette as the palette
 * @param actionName as the name of the related action
 */
void UBDesktopAnnotationController::setAssociatedPalettePosition(UBActionPalette *palette, const QString &actionName)
{
    QPoint desktopPalettePos = mDesktopPalette->geometry().topLeft();
    QList<QAction*> actions = mDesktopPalette->actions();
    int yPen = 0;

    foreach(QAction* act, actions)
    {
        if(act->objectName() == actionName)
        {
            int iAction = actions.indexOf(act);
            yPen = iAction * (mDesktopPalette->buttonSize().height() + 2 * mDesktopPalette->border() +6); // This is the mysterious value (6)
            break;
        }
    }

    // First determine if the palette must be shown on the left or on the right
    if(desktopPalettePos.x() <= (mTransparentDrawingView->width() - (palette->width() + mDesktopPalette->width() + mRightPalette->width() + 20))) // we take a small margin of 20 pixels
    {
        // Display it on the right
        desktopPalettePos += QPoint(mDesktopPalette->width(), yPen);
    }
    else
    {
        // Display it on the left
        desktopPalettePos += QPoint(0 - palette->width(), yPen);
    }

    palette->setCustomPosition(true);
    palette->move(desktopPalettePos);
}

void UBDesktopAnnotationController::eraseDesktopAnnotations()
{
    if (mTransparentDrawingScene)
    {
        mTransparentDrawingScene->clearContent(UBGraphicsScene::clearAnnotations);
    }
}


UBBoardView* UBDesktopAnnotationController::drawingView()
{
    return mTransparentDrawingView;
}


void UBDesktopAnnotationController::showWindow()
{
    mDesktopPalette->setDisplaySelectButtonVisible(true);

    connect(UBApplication::applicationController, SIGNAL(desktopMode(bool))
            , mDesktopPalette, SLOT(setDisplaySelectButtonVisible(bool)));

    mDesktopPalette->show();

    bool showDisplay = UBSettings::settings()->webShowPageImmediatelyOnMirroredScreen->get().toBool();

    mDesktopPalette->showHideClick(showDisplay);
    mDesktopPalette->updateShowHideState(showDisplay);

    if (!mWindowPositionInitialized)
    {
        QRect desktopRect = UBApplication::displayManager->screenGeometry(ScreenRole::Desktop);

        mDesktopPalette->move(5, desktopRect.top() + 150);

        mWindowPositionInitialized = true;
        mDesktopPalette->maximizeMe();
    }

    updateBackground();

    mBoardStylusTool = UBDrawingController::drawingController()->stylusTool();

    UBDrawingController::drawingController()->setStylusTool(mDesktopStylusTool);

#ifndef Q_OS_LINUX
    UBPlatformUtils::showFullScreen(mTransparentDrawingView);
#else
    // this is necessary to avoid hiding the panels on Unity and Cinnamon
    // if finer control is necessary, use qgetenv("XDG_CURRENT_DESKTOP")
    mTransparentDrawingView->show();
#endif
    UBPlatformUtils::hideMenuBarAndDock();

    mDesktopPalette->appear();

#ifdef UB_REQUIRES_MASK_UPDATE
    updateMask(true);
#endif // UB_REQUIRES_MASK_UPDATE
}


void UBDesktopAnnotationController::close()
{
    if (mTransparentDrawingView)
        mTransparentDrawingView->hide();

    mDesktopPalette->hide();
}


void UBDesktopAnnotationController::stylusToolChanged(int tool)
{
    Q_UNUSED(tool);
#ifdef Q_OS_OSX
    /* no longer needed
     if (UBDrawingController::drawingController()->isInDesktopMode())
     {
         UBStylusTool::Enum eTool = (UBStylusTool::Enum)tool;
         if(eTool == UBStylusTool::Selector)
         {
             UBPlatformUtils::toggleFinder(true);
         }
         else
         {
             UBPlatformUtils::toggleFinder(false);
         }
     }*/
#endif

    updateBackground();
}


void UBDesktopAnnotationController::updateBackground()
{
    QBrush newBrush;

    if (mIsFullyTransparent
            || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector)
    {
        newBrush = QBrush(Qt::transparent);
#ifdef UB_REQUIRES_MASK_UPDATE
        updateMask(true);
#endif //UB_REQUIRES_MASK_UPDATE
    }
    else
    {
#if defined(Q_OS_OSX)
        newBrush = QBrush(QColor(127, 127, 127, 15));
#else
        newBrush = QBrush(QColor(127, 127, 127, 1));
#endif
#ifdef UB_REQUIRES_MASK_UPDATE
        updateMask(false);
#endif //UB_REQUIRES_MASK_UPDATE
    }

    if (mTransparentDrawingScene && mTransparentDrawingScene->backgroundBrush() != newBrush)
        mTransparentDrawingScene->setBackgroundBrush(newBrush);
}


void UBDesktopAnnotationController::hideWindow()
{
    if (mTransparentDrawingView)
        mTransparentDrawingView->hide();

    mDesktopPalette->hide();

    mDesktopStylusTool = UBDrawingController::drawingController()->stylusTool();
    UBDrawingController::drawingController()->setStylusTool(mBoardStylusTool);
}


void UBDesktopAnnotationController::goToUniboard()
{
    UBApplication::applicationController->showBoard();
}


void UBDesktopAnnotationController::customCapture()
{
    if (!mCustomCaptureClicked)
    {
        mCustomCaptureClicked = true;
        onToolClicked();
        mIsFullyTransparent = true;
        updateBackground();

        mDesktopPalette->disappearForCapture();
        UBCustomCaptureWindow customCaptureWindow(mDesktopPalette);
        // need to show the window before execute it to avoid some glitch on windows.

    #ifndef Q_OS_WIN // Working only without this call on win32 desktop mode
        UBPlatformUtils::showFullScreen(&customCaptureWindow);
    #endif

        if (customCaptureWindow.execute(getScreenPixmap()) == QDialog::Accepted)
        {
            QPixmap selectedPixmap = customCaptureWindow.getSelectedPixmap();
            emit imageCaptured(selectedPixmap, false);
        }


        mDesktopPalette->appear();

        mCustomCaptureClicked = false;
        mIsFullyTransparent = false;
        updateBackground();
    }
}


void UBDesktopAnnotationController::screenCapture()
{
    onToolClicked();
    mIsFullyTransparent = true;
    updateBackground();

    mDesktopPalette->disappearForCapture();

    QPixmap originalPixmap = getScreenPixmap();

    mDesktopPalette->appear();

    emit imageCaptured(originalPixmap, false);

    mIsFullyTransparent = false;

    updateBackground();
}


QPixmap UBDesktopAnnotationController::getScreenPixmap()
{
    return UBApplication::displayManager->grab(ScreenRole::Control);
}


void UBDesktopAnnotationController::updateShowHideState(bool pEnabled)
{
    mDesktopPalette->updateShowHideState(pEnabled);
}


void UBDesktopAnnotationController::screenLayoutChanged()
{
    if (UBApplication::applicationController &&
            UBApplication::displayManager &&
            UBApplication::displayManager->hasDisplay())
    {
        mDesktopPalette->setShowHideButtonVisible(true);
    }
    else
    {
        mDesktopPalette->setShowHideButtonVisible(false);
    }
}

/**
 * \brief Handles the pen action pressed event
 */
void UBDesktopAnnotationController::penActionPressed()
{
    mbArrowClicked = false;
    mDesktopMarkerPalette->hide();
    mDesktopEraserPalette->hide();
    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Pen);
    mPenHoldTimer = QTime::currentTime();
    mPendingPenButtonPressed = true;

    // Check if the mouse cursor is on the little arrow
    QPoint cursorPos = QCursor::pos();
    QPoint palettePos = mDesktopPalette->mapToGlobal(QPoint(0, 0));  // global coordinates of palette
    QPoint buttonPos = mDesktopPalette->buttonPos(UBApplication::mainWindow->actionPen);

    int iX = cursorPos.x() - (palettePos.x() + buttonPos.x());    // x position of the cursor in the palette
    int iY = cursorPos.y() - (palettePos.y() + buttonPos.y());    // y position of the cursor in the palette

    if(iX >= 30 && iX <= 44 && iY >= 30 && iY <= 44)
    {
        mbArrowClicked = true;
        penActionReleased();
    }
    else
    {
        mHoldTimerPen.start(PROPERTY_PALETTE_TIMER);
    }
}

/**
 * \brief Handles the pen action released event
 */
void UBDesktopAnnotationController::penActionReleased()
{
    mHoldTimerPen.stop();
    if(mPendingPenButtonPressed)
    {
        if(mbArrowClicked || mPenHoldTimer.msecsTo(QTime::currentTime()) > PROPERTY_PALETTE_TIMER - 100)
        {
            togglePropertyPalette(mDesktopPenPalette);
        }
        else
        {
            UBApplication::mainWindow->actionPen->trigger();
        }
        mPendingPenButtonPressed = false;
    }
    UBApplication::mainWindow->actionPen->setChecked(true);

    switchCursor(UBStylusTool::Pen);
}

/**
 * \brief Handles the eraser action pressed event
 */
void UBDesktopAnnotationController::eraserActionPressed()
{
    mbArrowClicked = false;
    mDesktopPenPalette->hide();
    mDesktopMarkerPalette->hide();
    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Eraser);
    mEraserHoldTimer = QTime::currentTime();
    mPendingEraserButtonPressed = true;

    // Check if the mouse cursor is on the little arrow
    QPoint cursorPos = QCursor::pos();
    QPoint palettePos = mDesktopPalette->mapToGlobal(QPoint(0, 0));
    QPoint buttonPos = mDesktopPalette->buttonPos(UBApplication::mainWindow->actionEraser);

    int iX = cursorPos.x() - (palettePos.x() + buttonPos.x());    // x position of the cursor in the palette
    int iY = cursorPos.y() - (palettePos.y() + buttonPos.y());    // y position of the cursor in the palette

    if(iX >= 30 && iX <= 44 && iY >= 30 && iY <= 44)
    {
        mbArrowClicked = true;
        eraserActionReleased();
    }
    else
    {
        mHoldTimerEraser.start(PROPERTY_PALETTE_TIMER);
    }
}

/**
 * \brief Handles the eraser action released event
 */
void UBDesktopAnnotationController::eraserActionReleased()
{
    mHoldTimerEraser.stop();
    if(mPendingEraserButtonPressed)
    {
        if(mbArrowClicked || mEraserHoldTimer.msecsTo(QTime::currentTime()) > PROPERTY_PALETTE_TIMER - 100)
        {
            togglePropertyPalette(mDesktopEraserPalette);
        }
        else
        {
            UBApplication::mainWindow->actionEraser->trigger();
        }
        mPendingEraserButtonPressed = false;
    }
    UBApplication::mainWindow->actionEraser->setChecked(true);

    switchCursor(UBStylusTool::Eraser);
}


/**
 * \brief Handles the marker action pressed event
 */
void UBDesktopAnnotationController::markerActionPressed()
{
    mbArrowClicked = false;
    mDesktopPenPalette->hide();
    mDesktopEraserPalette->hide();
    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Marker);
    mMarkerHoldTimer = QTime::currentTime();
    mPendingMarkerButtonPressed = true;

    // Check if the mouse cursor is on the little arrow
    QPoint cursorPos = QCursor::pos();
    QPoint palettePos = mDesktopPalette->mapToGlobal(QPoint(0, 0));
    QPoint buttonPos = mDesktopPalette->buttonPos(UBApplication::mainWindow->actionMarker);

    int iX = cursorPos.x() - (palettePos.x() + buttonPos.x());    // x position of the cursor in the palette
    int iY = cursorPos.y() - (palettePos.y() + buttonPos.y());    // y position of the cursor in the palette

    if(iX >= 30 && iX <= 44 && iY >= 30 && iY <= 44)
    {
        mbArrowClicked = true;
        markerActionReleased();
    }
    else
    {
        mHoldTimerMarker.start(PROPERTY_PALETTE_TIMER);
    }
}


/**
 * \brief Handles the marker action released event
 */
void UBDesktopAnnotationController::markerActionReleased()
{
    mHoldTimerMarker.stop();
    if(mPendingMarkerButtonPressed)
    {
        if(mbArrowClicked || mMarkerHoldTimer.msecsTo(QTime::currentTime()) > PROPERTY_PALETTE_TIMER - 100)
        {
            togglePropertyPalette(mDesktopMarkerPalette);
        }
        else
        {
            UBApplication::mainWindow->actionMarker->trigger();
        }
        mPendingMarkerButtonPressed = false;
    }
    UBApplication::mainWindow->actionMarker->setChecked(true);

    switchCursor(UBStylusTool::Marker);
}

void UBDesktopAnnotationController::selectorActionPressed()
{

}

void UBDesktopAnnotationController::selectorActionReleased()
{
    UBApplication::mainWindow->actionSelector->setChecked(true);
    switchCursor(UBStylusTool::Selector);
}


void UBDesktopAnnotationController::pointerActionPressed()
{

}

void UBDesktopAnnotationController::pointerActionReleased()
{
    UBApplication::mainWindow->actionPointer->setChecked(true);
    switchCursor(UBStylusTool::Pointer);
}


/**
 * \brief Toggle the given palette visibility
 * @param palette as the given palette
 */
void UBDesktopAnnotationController::togglePropertyPalette(UBActionPalette *palette)
{
    if(NULL != palette)
    {
        bool bShow = !palette->isVisible();
        if(mDesktopPenPalette == palette)
        {
            desktopPenActionToggled(bShow);
        }
        else if(mDesktopMarkerPalette == palette)
        {
            desktopMarkerActionToggled(bShow);
        }
        else if(mDesktopEraserPalette == palette)
        {
            desktopEraserActionToggled(bShow);
        }
    }
}


void UBDesktopAnnotationController::switchCursor(const int tool)
{
    mTransparentDrawingScene->setToolCursor(tool);
    mTransparentDrawingView->setToolCursor(tool);
}

/**
 * \brief Reconnect the pressed & released signals of the property palettes
 */
void UBDesktopAnnotationController::onDesktopPaletteMaximized()
{
    // Pen
    UBActionPaletteButton* pPenButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionPen);
    if(NULL != pPenButton)
    {
        connect(pPenButton, SIGNAL(pressed()), this, SLOT(penActionPressed()));
        connect(pPenButton, SIGNAL(released()), this, SLOT(penActionReleased()));
    }

    // Eraser
    UBActionPaletteButton* pEraserButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionEraser);
    if(NULL != pEraserButton)
    {
        connect(pEraserButton, SIGNAL(pressed()), this, SLOT(eraserActionPressed()));
        connect(pEraserButton, SIGNAL(released()), this, SLOT(eraserActionReleased()));
    }

    // Marker
    UBActionPaletteButton* pMarkerButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionMarker);
    if(NULL != pMarkerButton)
    {
        connect(pMarkerButton, SIGNAL(pressed()), this, SLOT(markerActionPressed()));
        connect(pMarkerButton, SIGNAL(released()), this, SLOT(markerActionReleased()));
    }

    // Pointer
    UBActionPaletteButton* pSelectorButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionSelector);
    if(NULL != pSelectorButton)
    {
        connect(pSelectorButton, SIGNAL(pressed()), this, SLOT(selectorActionPressed()));
        connect(pSelectorButton, SIGNAL(released()), this, SLOT(selectorActionReleased()));
    }

    // Pointer
    UBActionPaletteButton* pPointerButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionPointer);
    if(NULL != pPointerButton)
    {
        connect(pPointerButton, SIGNAL(pressed()), this, SLOT(pointerActionPressed()));
        connect(pPointerButton, SIGNAL(released()), this, SLOT(pointerActionReleased()));
    }
}

/**
 * \brief Disconnect the pressed & release signals of the property palettes
 * This is done to prevent memory leaks
 */
void UBDesktopAnnotationController::onDesktopPaletteMinimize()
{
    // Pen
    UBActionPaletteButton* pPenButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionPen);
    if(NULL != pPenButton)
    {
        disconnect(pPenButton, SIGNAL(pressed()), this, SLOT(penActionPressed()));
        disconnect(pPenButton, SIGNAL(released()), this, SLOT(penActionReleased()));
    }

    // Marker
    UBActionPaletteButton* pMarkerButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionMarker);
    if(NULL != pMarkerButton)
    {
        disconnect(pMarkerButton, SIGNAL(pressed()), this, SLOT(markerActionPressed()));
        disconnect(pMarkerButton, SIGNAL(released()), this, SLOT(markerActionReleased()));
    }

    // Eraser
    UBActionPaletteButton* pEraserButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionEraser);
    if(NULL != pEraserButton)
    {
        disconnect(pEraserButton, SIGNAL(pressed()), this, SLOT(eraserActionPressed()));
        disconnect(pEraserButton, SIGNAL(released()), this, SLOT(eraserActionReleased()));
    }
}

void UBDesktopAnnotationController::TransparentWidgetResized()
{
    onTransparentWidgetResized();
}

/**
 * \brief Resize the library palette.
 */
void UBDesktopAnnotationController::onTransparentWidgetResized()
{
    int rW = UBApplication::boardController->paletteManager()->rightPalette()->width();
    int lW = UBApplication::boardController->paletteManager()->leftPalette()->width();

    int rH = mTransparentDrawingView->height();

    UBApplication::boardController->paletteManager()->rightPalette()->resize(rW+1, rH);
    UBApplication::boardController->paletteManager()->rightPalette()->resize(rW, rH);

    UBApplication::boardController->paletteManager()->leftPalette()->resize(lW+1, rH);
    UBApplication::boardController->paletteManager()->leftPalette()->resize(lW, rH);
}

void UBDesktopAnnotationController::updateMask(bool bTransparent)
{
    if(bTransparent)
    {
        // Here we have to generate a new mask This method is certainly resource
        // consuming but for the moment this is the only solution that I found.
        mMask = QPixmap(mTransparentDrawingView->width(), mTransparentDrawingView->height());
        mMask.fill(Qt::transparent);

        QPainter p;

        p.begin(&mMask);

        p.setPen(Qt::red);
        p.setBrush(QBrush(Qt::red));

        // Here we draw the widget mask
        if(mDesktopPalette->isVisible())
        {
            p.drawRect(mDesktopPalette->geometry().x(), mDesktopPalette->geometry().y(), mDesktopPalette->width(), mDesktopPalette->height());
        }
        if(UBApplication::boardController->paletteManager()->mKeyboardPalette->isVisible())
        {
            p.drawRect(UBApplication::boardController->paletteManager()->mKeyboardPalette->geometry().x(), UBApplication::boardController->paletteManager()->mKeyboardPalette->geometry().y(),
                       UBApplication::boardController->paletteManager()->mKeyboardPalette->width(), UBApplication::boardController->paletteManager()->mKeyboardPalette->height());
        }

        if(UBApplication::boardController->paletteManager()->leftPalette()->isVisible())
        {
            QRect leftPalette(UBApplication::boardController->paletteManager()->leftPalette()->geometry().x(),
                        UBApplication::boardController->paletteManager()->leftPalette()->geometry().y(),
                        UBApplication::boardController->paletteManager()->leftPalette()->width(),
                        UBApplication::boardController->paletteManager()->leftPalette()->height());

            QRect tabsPalette(UBApplication::boardController->paletteManager()->leftPalette()->getTabPaletteRect());

            p.drawRect(leftPalette);
            p.drawRect(tabsPalette);
        }

        if(UBApplication::boardController->paletteManager()->rightPalette()->isVisible())
        {
            QRect rightPalette(UBApplication::boardController->paletteManager()->rightPalette()->geometry().x(),
                        UBApplication::boardController->paletteManager()->rightPalette()->geometry().y(),
                        UBApplication::boardController->paletteManager()->rightPalette()->width(),
                        UBApplication::boardController->paletteManager()->rightPalette()->height());

            QRect tabsPalette(UBApplication::boardController->paletteManager()->rightPalette()->getTabPaletteRect());

            p.drawRect(rightPalette);
            p.drawRect(tabsPalette);
        }

        //Rquiered only for compiz wm
        //TODO. Window manager detection screen

        if (UBApplication::boardController->paletteManager()->addItemPalette()->isVisible()) {
            p.drawRect(UBApplication::boardController->paletteManager()->addItemPalette()->geometry());
        }


        p.end();

        // Then we add the annotations. We create another painter because we need to
        // apply transformations on it for coordinates matching
        QPainter annotationPainter;

        QTransform trans;
        trans.translate(mTransparentDrawingView->width()/2, mTransparentDrawingView->height()/2);

        annotationPainter.begin(&mMask);
        annotationPainter.setPen(Qt::red);
        annotationPainter.setBrush(Qt::red);

        annotationPainter.setTransform(trans);

        QList<QGraphicsItem*> allItems = mTransparentDrawingScene->items();

        for(int i = 0; i < allItems.size(); i++)
        {
            QGraphicsItem* pCrntItem = allItems.at(i);

            if(pCrntItem->isVisible() && pCrntItem->type() == UBGraphicsPolygonItem::Type)
            {
                QPainterPath crntPath = pCrntItem->shape();
                QRectF rect = crntPath.boundingRect();

                annotationPainter.drawRect(rect);
            }
        }

        annotationPainter.end();

        mTransparentDrawingView->setMask(mMask.mask());
    }
    else
    {
        mMask = QPixmap(mTransparentDrawingView->width(), mTransparentDrawingView->height());
        mMask.fill(Qt::transparent);

        QPainter p;

        p.begin(&mMask);

        p.setPen(Qt::red);
        p.setBrush(QBrush(Qt::red));

        p.drawRect(0, 0, mTransparentDrawingView->width(), mTransparentDrawingView->height());
        p.end();

        mTransparentDrawingView->setMask(mMask.mask());
    }
}

void UBDesktopAnnotationController::refreshMask()
{
    if (mTransparentDrawingScene && mTransparentDrawingView->isVisible()) {
        if(mIsFullyTransparent
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector
                //Needed to work correctly when another actions on stylus are checked
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Eraser
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Pointer
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Pen
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Marker)
        {
            updateMask(true);
        }
    }
}

void UBDesktopAnnotationController::onToolClicked()
{
    mDesktopEraserPalette->hide();
    mDesktopMarkerPalette->hide();
    mDesktopPenPalette->hide();
}
