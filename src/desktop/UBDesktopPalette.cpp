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




#include "UBDesktopPalette.h"

#include <QtGui>

#include "frameworks/UBPlatformUtils.h"

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "gui/UBMainWindow.h"

#include "core/memcheck.h"

UBDesktopPalette::UBDesktopPalette(QWidget *parent, UBRightPalette* _rightPalette)
    : UBActionPalette(Qt::TopLeftCorner, parent)
    , mShowHideAction(NULL)
    , mDisplaySelectAction(NULL)
    , rightPalette(_rightPalette)
    , mMinimizedLocation(eMinimizedLocation_None)
    , pendingButton(nullptr)
{

    mActionUniboard = new QAction(QIcon(":/images/toolbar/board.png"), tr("Show OpenBoard"), this);
    connect(mActionUniboard, SIGNAL(triggered()), this, SIGNAL(uniboardClick()));

    mActionCustomSelect = new QAction(QIcon(":/images/toolbar/captureArea.png"), tr("Capture Part of the Screen"), this);
    connect(mActionCustomSelect, SIGNAL(triggered()), this, SIGNAL(customClick()));

    mDisplaySelectAction = new QAction(QIcon(":/images/toolbar/captureScreen.png"), tr("Capture the Screen"), this);
    connect(mDisplaySelectAction, SIGNAL(triggered()), this, SIGNAL(screenClick()));

    QIcon showHideIcon;
    showHideIcon.addPixmap(QPixmap(":/images/toolbar/eyeOpened.png"), QIcon::Normal , QIcon::On);
    showHideIcon.addPixmap(QPixmap(":/images/toolbar/eyeClosed.png"), QIcon::Normal , QIcon::Off);
    mShowHideAction = new QAction(showHideIcon, "", this);
    mShowHideAction->setCheckable(true);

    connect(mShowHideAction, SIGNAL(triggered(bool)), this, SLOT(showHideClick(bool)));

    createAndConnectButtons();
    setButtonIconSize(QSize(42, 42));

    adjustSizeAndPosition();

    //  This palette can be minimized
    QIcon maximizeIcon;
    maximizeIcon.addPixmap(QPixmap(":/images/toolbar/stylusTab.png"), QIcon::Normal, QIcon::On);
    mMaximizeAction = new QAction(maximizeIcon, tr("Show the stylus palette"), this);
    connect(mMaximizeAction, SIGNAL(triggered()), this, SLOT(maximizeMe()));
}


void UBDesktopPalette::createAndConnectButtons(){
    addAction(mActionUniboard);
    UBActionPaletteButton* button = addAction(UBApplication::mainWindow->actionPen);
    connect(button, &UBActionPaletteButton::pressed, [=](){
        actionPressed(button, UBApplication::mainWindow->actionPen, UBStylusTool::Pen);
    });
    connect(button, &UBActionPaletteButton::released, [=](){
        actionReleased(UBApplication::mainWindow->actionPen);
    });
    button = addAction(UBApplication::mainWindow->actionEraser);
    connect(button, &UBActionPaletteButton::pressed, [=](){
        actionPressed(button, UBApplication::mainWindow->actionEraser, UBStylusTool::Eraser);
    });
    connect(button, &UBActionPaletteButton::released, [=](){
        actionReleased(UBApplication::mainWindow->actionEraser);
    });
    button = addAction(UBApplication::mainWindow->actionMarker);
    connect(button, &UBActionPaletteButton::pressed, [=](){
        actionPressed(button, UBApplication::mainWindow->actionMarker, UBStylusTool::Marker);
    });
    connect(button, &UBActionPaletteButton::released, [=](){
        actionReleased(UBApplication::mainWindow->actionMarker);
    });
    button = addAction(UBApplication::mainWindow->actionSelector);
    connect(button, &UBActionPaletteButton::pressed, this, [=](){
        emit hideOtherPalettes(nullptr);
    });
    connect(button, &UBActionPaletteButton::released, [=](){
        actionReleased(UBApplication::mainWindow->actionSelector);
    });
    button = addAction(UBApplication::mainWindow->actionPointer);
    connect(button, &UBActionPaletteButton::pressed, this, [=](){
        emit hideOtherPalettes(nullptr);
    });
    connect(button, &UBActionPaletteButton::released, [=](){
        actionReleased(UBApplication::mainWindow->actionPointer);
    });
    if (UBPlatformUtils::hasVirtualKeyboard())
        addAction(UBApplication::mainWindow->actionVirtualKeyboard);
    addAction(mActionCustomSelect);
    addAction(mDisplaySelectAction);
    addAction(mShowHideAction);

    actionChanged();
}

UBDesktopPalette::~UBDesktopPalette()
{

}

void UBDesktopPalette::disappearForCapture()
{
    setWindowOpacity(0.0);
    qApp->processEvents();
}


void UBDesktopPalette::appear()
{
    setWindowOpacity(1.0);
}


void UBDesktopPalette::showHideClick(bool checked)
{
    UBApplication::applicationController->mirroringEnabled(checked);
}


void UBDesktopPalette::updateShowHideState(bool pShowEnabled)
{
    if (mShowHideAction)
        mShowHideAction->setChecked(pShowEnabled);

    if (mShowHideAction->isChecked())
        mShowHideAction->setToolTip(tr("Show Board on Secondary Screen"));
    else
        mShowHideAction->setToolTip(tr("Show Desktop on Secondary Screen"));

    if (pShowEnabled)
        raise();
}


void UBDesktopPalette::setShowHideButtonVisible(bool visible)
{
    mShowHideAction->setVisible(visible);
}


void UBDesktopPalette::setDisplaySelectButtonVisible(bool visible)
{
    mDisplaySelectAction->setVisible(visible);
}

//  Called when the palette is near the border and must be minimized
void UBDesktopPalette::minimizeMe()
{
    clearLayout();

    addAction(mMaximizeAction);
    actionChanged();

    adjustSizeAndPosition();

#ifdef UB_REQUIRES_MASK_UPDATE
        emit refreshMask();
#endif
}

void UBDesktopPalette::minimizePalette(const QPoint& pos)
{

    if(mMinimizedLocation == eMinimizedLocation_None)
    {
    //  Verify if we have to minimize this palette
    if(pos.x() == 5)
    {
        mMinimizedLocation = eMinimizedLocation_Left;
    }
//    else if(pos.y() == 5)
//    {
//        mMinimizedLocation = eMinimizedLocation_Top;
//    }
    else if(pos.x() == parentWidget()->width() - getParentRightOffset() - width() - 5)
    {
        mMinimizedLocation = eMinimizedLocation_Right;
    }
//    else if(pos.y() == parentSize.height() - height() - 5)
//    {
//        mMinimizedLocation = eMinimizedLocation_Bottom;
//    }

    //  Minimize the Palette
    if(mMinimizedLocation != eMinimizedLocation_None)
    {
        minimizeMe();
    }
    }
    else
    {
    //  Restore the palette
    if(pos.x() > 5 &&
       pos.y() > 5 &&
       pos.x() < parentWidget()->width() - getParentRightOffset()  - width() - 5 &&
       pos.y() < parentWidget()->size().height() - height() - 5)
    {
        mMinimizedLocation = eMinimizedLocation_None;
        maximizeMe();
    }
    }
}

//  Called when the user wants to maximize the palette
void UBDesktopPalette::maximizeMe()
{
    clearLayout();

    createAndConnectButtons();


    adjustSizeAndPosition();

    // Notify that the maximization has been done
    emit maximized();
#ifdef UB_REQUIRES_MASK_UPDATE
        emit refreshMask();
#endif
}

void UBDesktopPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    QIcon penIcon;
    QIcon markerIcon;
    QIcon eraserIcon;
    penIcon.addFile(":images/stylusPalette/penArrow.svg", QSize(), QIcon::Normal, QIcon::Off);
    penIcon.addFile(":images/stylusPalette/penOnArrow.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionPen->setIcon(penIcon);
    markerIcon.addFile(":images/stylusPalette/markerArrow.svg", QSize(), QIcon::Normal, QIcon::Off);
    markerIcon.addFile(":images/stylusPalette/markerOnArrow.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionMarker->setIcon(markerIcon);
    eraserIcon.addFile(":images/stylusPalette/eraserArrow.svg", QSize(), QIcon::Normal, QIcon::Off);
    eraserIcon.addFile(":images/stylusPalette/eraserOnArrow.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionEraser->setIcon(eraserIcon);
}

void UBDesktopPalette::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    QIcon penIcon;
    QIcon markerIcon;
    QIcon eraserIcon;
    penIcon.addFile(":images/stylusPalette/pen.svg", QSize(), QIcon::Normal, QIcon::Off);
    penIcon.addFile(":images/stylusPalette/penOn.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionPen->setIcon(penIcon);
    markerIcon.addFile(":images/stylusPalette/marker.svg", QSize(), QIcon::Normal, QIcon::Off);
    markerIcon.addFile(":images/stylusPalette/markerOn.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionMarker->setIcon(markerIcon);
    eraserIcon.addFile(":images/stylusPalette/eraser.svg", QSize(), QIcon::Normal, QIcon::Off);
    eraserIcon.addFile(":images/stylusPalette/eraserOn.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionEraser->setIcon(eraserIcon);
}

int UBDesktopPalette::getParentRightOffset()
{
    return rightPalette->width();
}

void UBDesktopPalette::parentResized()
{
    QPoint p = pos();
    if (mMinimizedLocation == eMinimizedLocation_Right)
    {
        p.setX(parentWidget()->width() - getParentRightOffset() -width());
    }

    moveInsideParent(p);
}

void UBDesktopPalette::actionPressed(QToolButton* button, QAction* action, int stylusTool)
{
    emit hideOtherPalettes(action);
    UBDrawingController::drawingController()->setStylusTool(stylusTool);
    mButtonHoldTimer = QTime::currentTime();
    pendingButton = action;

    // Check if the mouse cursor is on the little arrow
    QPoint cursorPos = QCursor::pos();
    QPoint palettePos = mapToGlobal(QPoint(0, 0));  // global coordinates of palette
    QPoint clickedButtonPos = button->pos();

    int iX = cursorPos.x() - (palettePos.x() + clickedButtonPos.x());    // x position of the cursor in the palette
    int iY = cursorPos.y() - (palettePos.y() + clickedButtonPos.y());    // y position of the cursor in the palette

    if(iX >= 30 && iX <= 44 && iY >= 30 && iY <= 44)
    {
        emit togglePropertyPalette(action);
        pendingButton = nullptr;
        // We must switch the cursor. For some reason this works correctly even without this:
        // emit switchCursor_Pen();
    }
}

/**
 * \brief Handles the pen action released event
 */
void UBDesktopPalette::actionReleased(QAction* action)
{
    if(pendingButton == action)
    {
        if(mButtonHoldTimer.msecsTo(QTime::currentTime()) > PROPERTY_PALETTE_TIMER - 100)
        {
            emit togglePropertyPalette(action);
        }
        else
        {
            action->trigger();
        }
        pendingButton = nullptr;
    }
    action->setChecked(true);
    emit switchCursor(action);
}

