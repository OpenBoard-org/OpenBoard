/*
 * UNWindowController.cpp
 *
 *  Created on: Jan 15, 2009
 *      Author: julienbachmann
 */

#include <QDesktopWidget>

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

#include "domain/UBGraphicsScene.h"

#include "UBCustomCaptureWindow.h"
#include "UBWindowCapture.h"
#include "UBDesktopPalette.h"
#include "UBDesktopToolsPalette.h"
#include "UBDesktopPenPalette.h"
#include "UBDesktopMarkerPalette.h"
#include "UBDesktopEraserPalette.h"

#include "gui/UBKeyboardPalette.h"

#include "core/memcheck.h"

UBDesktopAnnotationController::UBDesktopAnnotationController(QObject *parent)
        : QObject(parent)
        , mTransparentDrawingView(0)
        , mTransparentDrawingScene(0)
        , mDesktopPalette(NULL)
        , mDesktopToolsPalette(NULL)
        , mDesktopPenPalette(NULL)
        , mDesktopMarkerPalette(NULL)
        , mDesktopEraserPalette(NULL)
        , mLibPalette(NULL)
        , mWindowPositionInitialized(0)
        , mIsFullyTransparent(false)
        , mDesktopToolsPalettePositioned(false)
        , mPendingPenButtonPressed(false)
        , mPendingMarkerButtonPressed(false)
        , mPendingEraserButtonPressed(false)
        , mbArrowClicked(false)
        , mBoardStylusTool(UBStylusTool::Pen)
        , mDesktopStylusTool(UBStylusTool::Selector)
        , mKeyboardPalette(0)
{

    mTransparentDrawingView = new UBBoardView(UBApplication::boardController, 0); // deleted in UBDesktopAnnotationController::destructor

    mTransparentDrawingView->setAttribute(Qt::WA_TranslucentBackground, true);
	// !!!! Should be included into Windows after QT recompilation
#ifdef Q_WS_MAC
    //mTransparentDrawingView->setAttribute(Qt::WA_MacNoShadow, true);
#endif
    mTransparentDrawingView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Window);
    mTransparentDrawingView->setCacheMode(QGraphicsView::CacheNone);
    mTransparentDrawingView->resize(UBApplication::desktop()->width(), UBApplication::desktop()->height());

    mTransparentDrawingView->setMouseTracking(true);

    mTransparentDrawingView->setAcceptDrops(false);

    QString backgroundStyle = "QWidget {background-color: rgba(127, 127, 127, 0)}";
    mTransparentDrawingView->setStyleSheet(backgroundStyle);

    mTransparentDrawingScene = new UBGraphicsScene(0);
    mTransparentDrawingView->setScene(mTransparentDrawingScene);

    mLibPalette = new UBLibPalette(mTransparentDrawingView);

    mDesktopPalette = new UBDesktopPalette(mTransparentDrawingView);

    if (UBPlatformUtils::hasVirtualKeyboard())
    {
        mKeyboardPalette = UBKeyboardPalette::create(mTransparentDrawingView);
        mKeyboardPalette->setParent(mTransparentDrawingView);
        connect(mKeyboardPalette, SIGNAL(keyboardActivated(bool)), mTransparentDrawingView, SLOT(virtualKeyboardActivated(bool))); 
        connect(mKeyboardPalette, SIGNAL(moved(QPoint)), this, SLOT(refreshMask()));
    }

    connect(mDesktopPalette, SIGNAL(uniboardClick()), this, SLOT(goToUniboard()));
    connect(mDesktopPalette, SIGNAL(customClick()), this, SLOT(customCapture()));
    connect(mDesktopPalette, SIGNAL(windowClick()), this, SLOT(windowCapture()));
    connect(mDesktopPalette, SIGNAL(screenClick()), this, SLOT(screenCapture()));
    connect(mDesktopPalette, SIGNAL(maximized()), this, SLOT(onDesktopPaletteMaximized()));
    connect(mDesktopPalette, SIGNAL(minimizeStart(eMinimizedLocation)), this, SLOT(onDesktopPaletteMinimize()));
    connect(UBApplication::mainWindow->actionVirtualKeyboard, SIGNAL(triggered(bool)), this, SLOT(showKeyboard(bool)));

//    connect(mDesktopPalette, SIGNAL(showVirtualKeyboard(bool)), this, SLOT());

    connect(mTransparentDrawingView, SIGNAL(resized(QResizeEvent*)), this, SLOT(onTransparentWidgetResized()));


    connect(UBDrawingController::drawingController(), SIGNAL(stylusToolChanged(int))
            , this, SLOT(stylusToolChanged(int)));

    // Add the desktop associated palettes
    mDesktopToolsPalette = new UBDesktopToolsPalette(mTransparentDrawingView);
    mDesktopPenPalette = new UBDesktopPenPalette(mTransparentDrawingView);

    connect(mDesktopPalette, SIGNAL(maximized()), mDesktopPenPalette, SLOT(onParentMaximized()));
    connect(mDesktopPalette, SIGNAL(minimizeStart(eMinimizedLocation)), mDesktopPenPalette, SLOT(onParentMinimized()));

    mDesktopMarkerPalette = new UBDesktopMarkerPalette(mTransparentDrawingView);
    mDesktopEraserPalette = new UBDesktopEraserPalette(mTransparentDrawingView);

    mDesktopPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopToolsPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopPenPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopMarkerPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);
    mDesktopEraserPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);

    mDesktopToolsPalette->setVisible(UBApplication::mainWindow->actionDesktopTools->isChecked());

    // Hack : the size of the property palettes is computed the first time the palette is visible
    //        In order to prevent palette overlap on if the desktop palette is on the right of the
    //        screen, a setVisible(true) followed by a setVisible(false) is done.
    mDesktopPenPalette->setVisible(true);
    mDesktopMarkerPalette->setVisible(true);
    mDesktopEraserPalette->setVisible(true);
    mDesktopPenPalette->setVisible(false);
    mDesktopMarkerPalette->setVisible(false);
    mDesktopEraserPalette->setVisible(false);

    connect(UBApplication::mainWindow->actionDesktopTools, SIGNAL(triggered(bool)), this, SLOT(desktopToolsActionToogled(bool)));
    connect(UBApplication::mainWindow->actionEraseDesktopAnnotations, SIGNAL(triggered()), this, SLOT(eraseDesktopAnnotations()));

    connect(&mHoldTimerPen, SIGNAL(timeout()), this, SLOT(penActionReleased()));
    connect(&mHoldTimerMarker, SIGNAL(timeout()), this, SLOT(markerActionReleased()));
    connect(&mHoldTimerEraser, SIGNAL(timeout()), this, SLOT(eraserActionReleased()));

    connect(mDesktopPalette, SIGNAL(moving()), this, SLOT(refreshMask()));
    connect(mLibPalette, SIGNAL(resized()), this, SLOT(refreshMask()));

    onDesktopPaletteMaximized();
}

void UBDesktopAnnotationController::showKeyboard(bool show)
{
    mKeyboardPalette->setVisible(show);
    updateMask(true);

//    mDesktopPalette->showVirtualKeyboard(show);
}

UBDesktopAnnotationController::~UBDesktopAnnotationController()
{
    delete mTransparentDrawingScene;
    delete mTransparentDrawingView;

	/*
    if(NULL != mDesktopPenPalette)
    {
        delete mDesktopPalette;
        mDesktopPenPalette = NULL;
    }
    if(NULL != mDesktopMarkerPalette)
    {
        delete mDesktopMarkerPalette;
        mDesktopMarkerPalette = NULL;
    }
    if(NULL != mDesktopEraserPalette)
    {
        delete mDesktopEraserPalette;
        mDesktopEraserPalette = NULL;
    }
    if(NULL != mLibPalette)
    {
        delete mLibPalette;
        mLibPalette = NULL;
    }
	*/
}


UBDesktopPalette* UBDesktopAnnotationController::desktopPalette()
{
    return mDesktopPalette;
}


void UBDesktopAnnotationController::desktopToolsActionToogled(bool checked)
{
    if (!mDesktopToolsPalettePositioned)
    {
        QPoint pos = mDesktopPalette->geometry().bottomLeft();
        pos += QPoint(0, 10);

        mDesktopToolsPalette->setCustomPosition(true);
        mDesktopToolsPalette->move(pos);

        mDesktopToolsPalettePositioned = true;
    }

    mDesktopToolsPalette->setVisible(checked);
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
            yPen = iAction * mDesktopPalette->buttonSize().height();

            // Add the borders
            yPen += (iAction) * (mDesktopPalette->border() + 4); // 4 has been set after some experiment. We must determine why this value is good

            break;
        }
    }

    // First determine if the palette must be shown on the left or on the right
    if(desktopPalettePos.x() <= (mTransparentDrawingView->width() - (palette->width() + mDesktopPalette->width() + 20))) // we take a small margin of 20 pixels
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
        mTransparentDrawingScene->clearAnnotations();
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
        QRect desktopRect = QApplication::desktop()->screenGeometry(mDesktopPalette->pos());

        //mDesktopPalette->move((desktopRect.right() - (mDesktopPalette->width() + 20)), desktopRect.top() + 150);
        mDesktopPalette->move(5, desktopRect.top() + 150);

        mWindowPositionInitialized = true;
    }

    updateBackground();

    mBoardStylusTool = UBDrawingController::drawingController()->stylusTool();

    UBDrawingController::drawingController()->setStylusTool(mDesktopStylusTool);

    mTransparentDrawingView->showFullScreen();

    UBPlatformUtils::setDesktopMode(true);

    mDesktopPalette->appear();

#ifdef Q_WS_X11
    updateMask(true);
#endif
}


void UBDesktopAnnotationController::close()
{
    // NOOP
}


void UBDesktopAnnotationController::stylusToolChanged(int tool)
{
    UBStylusTool::Enum eTool = (UBStylusTool::Enum)tool;
    mDesktopPalette->notifySelectorSelection(UBStylusTool::Selector == eTool);

    if(UBStylusTool::Selector != eTool)
    {
        UBApplication::mainWindow->actionVirtualKeyboard->setChecked(false);
        mKeyboardPalette->setVisible(false);
    }

    updateBackground();
}


void UBDesktopAnnotationController::updateBackground()
{
    QBrush newBrush;

    if (mIsFullyTransparent
            || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector)
    {
        newBrush = QBrush(Qt::transparent);
#ifdef Q_WS_X11
        updateMask(true);
#endif
    }
    else
    {
#if defined(Q_WS_MAC)
        newBrush = QBrush(QColor(127, 127, 127, 15));
#else
        newBrush = QBrush(QColor(127, 127, 127, 1));
#endif
#ifdef Q_WS_X11
        updateMask(false);
#endif
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
    hideWindow();

    UBPlatformUtils::setDesktopMode(false);

    UBApplication::mainWindow->actionVirtualKeyboard->setEnabled(true);

    emit restoreUniboard();
}


void UBDesktopAnnotationController::customCapture()
{
    mIsFullyTransparent = true;
    updateBackground();

    mDesktopPalette->disappearForCapture();
    UBCustomCaptureWindow customCaptureWindow(mDesktopPalette);

    // need to show the window before execute it to avoid some glitch on windows.
    customCaptureWindow.show();

    if (customCaptureWindow.execute(getScreenPixmap()) == QDialog::Accepted)
    {
        QPixmap selectedPixmap = customCaptureWindow.getSelectedPixmap();
        emit imageCaptured(selectedPixmap, false);
    }

    mDesktopPalette->appear();

    mIsFullyTransparent = false;
    updateBackground();
}


void UBDesktopAnnotationController::windowCapture()
{
    mIsFullyTransparent = true;
    updateBackground();

    mDesktopPalette->disappearForCapture();

    UBWindowCapture util(this);

    if (util.execute() == QDialog::Accepted)
    {
        QPixmap windowPixmap = util.getCapturedWindow();

        // on Mac OS X we can only know that user cancel the operatiion by checking is the image is null
        // because the screencapture utility always return code 0 event if user cancel the application
        if (!windowPixmap.isNull())
        {
            emit imageCaptured(windowPixmap, false);
        }
    }

    mDesktopPalette->appear();

    mIsFullyTransparent = false;

    updateBackground();
}


void UBDesktopAnnotationController::screenCapture()
{
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
    QDesktopWidget *desktop = QApplication::desktop();

    // we capture the screen in which the mouse is.
    const QRect primaryScreenRect = desktop->screenGeometry(QCursor::pos());
    QCoreApplication::flush();

    return QPixmap::grabWindow(desktop->winId(), primaryScreenRect.x()
                               , primaryScreenRect.y(), primaryScreenRect.width(), primaryScreenRect.height());
}


void UBDesktopAnnotationController::updateShowHideState(bool pEnabled)
{
    mDesktopPalette->updateShowHideState(pEnabled);
}


void UBDesktopAnnotationController::screenLayoutChanged()
{
    if (UBApplication::applicationController &&
            UBApplication::applicationController->displayManager() &&
            UBApplication::applicationController->displayManager()->hasDisplay())
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
    QPoint palettePos = mDesktopPalette->pos();
    QPoint buttonPos = mDesktopPalette->buttonPos(UBApplication::mainWindow->actionPen);

    int iX = cursorPos.x() - (palettePos.x() + buttonPos.x());    // x position of the cursor in the palette
    int iY = cursorPos.y() - (palettePos.y() + buttonPos.y());    // y position of the cursor in the palette

    if(iX >= 37 && iX <= 44 && iY >= 37 && iY <= 44)
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
    qDebug() << "penActionReleased()";
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
    QPoint palettePos = mDesktopPalette->pos();
    QPoint buttonPos = mDesktopPalette->buttonPos(UBApplication::mainWindow->actionMarker);

    int iX = cursorPos.x() - (palettePos.x() + buttonPos.x());    // x position of the cursor in the palette
    int iY = cursorPos.y() - (palettePos.y() + buttonPos.y());    // y position of the cursor in the palette

    if(iX >= 37 && iX <= 44 && iY >= 37 && iY <= 44)
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
    qDebug() << "markerActionReleased()";
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
    QPoint palettePos = mDesktopPalette->pos();
    QPoint buttonPos = mDesktopPalette->buttonPos(UBApplication::mainWindow->actionEraser);

    int iX = cursorPos.x() - (palettePos.x() + buttonPos.x());    // x position of the cursor in the palette
    int iY = cursorPos.y() - (palettePos.y() + buttonPos.y());    // y position of the cursor in the palette

    if(iX >= 37 && iX <= 44 && iY >= 37 && iY <= 44)
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
    qDebug() << "eraserActionReleased()";
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
}

/**
 * \brief Toggle the given palette visiblity
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

    // Marker
    UBActionPaletteButton* pMarkerButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionMarker);
    if(NULL != pMarkerButton)
    {
        connect(pMarkerButton, SIGNAL(pressed()), this, SLOT(markerActionPressed()));
        connect(pMarkerButton, SIGNAL(released()), this, SLOT(markerActionReleased()));
    }

    // Eraser
    UBActionPaletteButton* pEraserButton = mDesktopPalette->getButtonFromAction(UBApplication::mainWindow->actionEraser);
    if(NULL != pEraserButton)
    {
        connect(pEraserButton, SIGNAL(pressed()), this, SLOT(eraserActionPressed()));
        connect(pEraserButton, SIGNAL(released()), this, SLOT(eraserActionReleased()));
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

/**
 * \brief Resize the library palette.
 */
void UBDesktopAnnotationController::onTransparentWidgetResized()
{
//    qDebug() << "mTransparentDrawingView (" << mTransparentDrawingView->width() << "," << mTransparentDrawingView->height() << ")";
//    qDebug() << "mLibPalette (" << mLibPalette->width() << "," << mLibPalette->height() << ")";
      mLibPalette->resize(mLibPalette->width(), mTransparentDrawingView->height());
}

void UBDesktopAnnotationController::updateMask(bool bTransparent)
{
    if(bTransparent)
    {
        // Here we have to generate a new mask. This method is certainly resource
        // consuming but for the moment this is the only solution that I found.
        mMask = QPixmap(mTransparentDrawingView->width(), mTransparentDrawingView->height());

        QPainter p;

        p.begin(&mMask);

        p.setPen(Qt::red);
        p.setBrush(QBrush(Qt::red));

        // Here we draw the widget mask
        if(mDesktopPalette->isVisible())
        {
            p.drawRect(mDesktopPalette->geometry().x(), mDesktopPalette->geometry().y(), mDesktopPalette->width(), mDesktopPalette->height());
        }
        if(mKeyboardPalette->isVisible())
        {
            p.drawRect(mKeyboardPalette->geometry().x(), mKeyboardPalette->geometry().y(), mKeyboardPalette->width(), mKeyboardPalette->height());
        }
        if(mLibPalette->isVisible())
        {
            p.drawRect(mLibPalette->geometry().x(), mLibPalette->geometry().y(), mLibPalette->width(), mLibPalette->height());
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

            if(pCrntItem->isVisible())
            {
                QPainterPath crntPath = pCrntItem->shape();
                QRectF rect = crntPath.boundingRect();

                annotationPainter.drawRect(rect);
            }
        }

        annotationPainter.end();

        mTransparentDrawingView->setMask(mMask.createMaskFromColor(Qt::black));
    }
    else
    {
        // Remove the mask
        QPixmap noMask(mTransparentDrawingView->width(), mTransparentDrawingView->height());
        mTransparentDrawingView->setMask(noMask.mask());
    }
}

void UBDesktopAnnotationController::refreshMask()
{
    if(mIsFullyTransparent
            || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector)
    {
        updateMask(true);
    }
}
