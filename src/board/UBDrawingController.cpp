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




#include "UBDrawingController.h"

#include "core/UBSettings.h"
#include "core/UBApplication.h"

#include "domain/UBGraphicsScene.h"
#include "board/UBBoardController.h"

#include "gui/UBMainWindow.h"
#include "core/memcheck.h"

UBDrawingController* UBDrawingController::sDrawingController = 0;


UBDrawingController* UBDrawingController::drawingController()
{
    if(!sDrawingController)
        sDrawingController = new UBDrawingController();

    return sDrawingController;
}

void UBDrawingController::destroy()
{
    if(sDrawingController)
        delete sDrawingController;
    sDrawingController = NULL;
}

UBDrawingController::UBDrawingController(QObject * parent)
    : QObject(parent)
    , mActiveRuler(NULL)
    , mStylusTool((UBStylusTool::Enum)-1)
    , mLatestDrawingTool((UBStylusTool::Enum)-1)
    , mIsDesktopMode(false)
{
    connect(UBSettings::settings(), SIGNAL(colorContextChanged()), this, SIGNAL(colorPaletteChanged()));

    connect(UBApplication::mainWindow->actionPen, SIGNAL(triggered(bool)), this, SLOT(penToolSelected(bool)));
    connect(UBApplication::mainWindow->actionEraser, SIGNAL(triggered(bool)), this, SLOT(eraserToolSelected(bool)));
    connect(UBApplication::mainWindow->actionMarker, SIGNAL(triggered(bool)), this, SLOT(markerToolSelected(bool)));
    connect(UBApplication::mainWindow->actionSelector, SIGNAL(triggered(bool)), this, SLOT(selectorToolSelected(bool)));
    connect(UBApplication::mainWindow->actionPlay, SIGNAL(triggered(bool)), this, SLOT(playToolSelected(bool)));
    connect(UBApplication::mainWindow->actionHand, SIGNAL(triggered(bool)), this, SLOT(handToolSelected(bool)));
    connect(UBApplication::mainWindow->actionZoomIn, SIGNAL(triggered(bool)), this, SLOT(zoomInToolSelected(bool)));
    connect(UBApplication::mainWindow->actionZoomOut, SIGNAL(triggered(bool)), this, SLOT(zoomOutToolSelected(bool)));
    connect(UBApplication::mainWindow->actionPointer, SIGNAL(triggered(bool)), this, SLOT(pointerToolSelected(bool)));
    connect(UBApplication::mainWindow->actionLine, SIGNAL(triggered(bool)), this, SLOT(lineToolSelected(bool)));
    connect(UBApplication::mainWindow->actionText, SIGNAL(triggered(bool)), this, SLOT(textToolSelected(bool)));
    connect(UBApplication::mainWindow->actionCapture, SIGNAL(triggered(bool)), this, SLOT(captureToolSelected(bool)));
}


UBDrawingController::~UBDrawingController()
{
    // NOOP
}


int UBDrawingController::stylusTool()
{
    return mStylusTool;
}


int UBDrawingController::latestDrawingTool()
{
    return mLatestDrawingTool;
}


void UBDrawingController::setStylusTool(int tool)
{
    if (tool != mStylusTool)
    {
        UBApplication::boardController->activeScene()->deselectAllItems();
        if (mStylusTool == UBStylusTool::Pen || mStylusTool == UBStylusTool::Marker
                || mStylusTool == UBStylusTool::Line)
        {
            mLatestDrawingTool = mStylusTool;
        }

        if (tool == UBStylusTool::Pen || tool == UBStylusTool::Line)
        {
             emit lineWidthIndexChanged(UBSettings::settings()->penWidthIndex());
             emit colorIndexChanged(UBSettings::settings()->penColorIndex());
        }
        else if (tool == UBStylusTool::Marker)
        {
            emit lineWidthIndexChanged(UBSettings::settings()->markerWidthIndex());
            emit colorIndexChanged(UBSettings::settings()->markerColorIndex());
        }

        UBStylusTool::Enum previousTool = mStylusTool;
        mStylusTool = (UBStylusTool::Enum)tool;


        if (mStylusTool == UBStylusTool::Pen)
            UBApplication::mainWindow->actionPen->setChecked(true);
        else if (mStylusTool == UBStylusTool::Eraser)
            UBApplication::mainWindow->actionEraser->setChecked(true);
        else if (mStylusTool == UBStylusTool::Marker)
            UBApplication::mainWindow->actionMarker->setChecked(true);
        else if (mStylusTool == UBStylusTool::Selector)
            UBApplication::mainWindow->actionSelector->setChecked(true);
        else if (mStylusTool == UBStylusTool::Play)
            UBApplication::mainWindow->actionPlay->setChecked(true);
        else if (mStylusTool == UBStylusTool::Hand)
            UBApplication::mainWindow->actionHand->setChecked(true);
        else if (mStylusTool == UBStylusTool::ZoomIn)
            UBApplication::mainWindow->actionZoomIn->setChecked(true);
        else if (mStylusTool == UBStylusTool::ZoomOut)
            UBApplication::mainWindow->actionZoomOut->setChecked(true);
        else if (mStylusTool == UBStylusTool::Pointer)
            UBApplication::mainWindow->actionPointer->setChecked(true);
        else if (mStylusTool == UBStylusTool::Line)
            UBApplication::mainWindow->actionLine->setChecked(true);
        else if (mStylusTool == UBStylusTool::Text)
            UBApplication::mainWindow->actionText->setChecked(true);
        else if (mStylusTool == UBStylusTool::Capture)
            UBApplication::mainWindow->actionCapture->setChecked(true);

        emit stylusToolChanged(tool, previousTool);
        if (mStylusTool != UBStylusTool::Selector)
            emit colorPaletteChanged();
    }
}


bool UBDrawingController::isDrawingTool()
{
    return (stylusTool() == UBStylusTool::Pen)
            || (stylusTool() == UBStylusTool::Marker)
            || (stylusTool() == UBStylusTool::Line);
}


int UBDrawingController::currentToolWidthIndex()
{
    if (stylusTool() == UBStylusTool::Pen || stylusTool() == UBStylusTool::Line)
        return UBSettings::settings()->penWidthIndex();
    else if (stylusTool() == UBStylusTool::Marker)
        return UBSettings::settings()->markerWidthIndex();
    else
        return -1;
}


qreal UBDrawingController::currentToolWidth()
{
    if (stylusTool() == UBStylusTool::Pen || stylusTool() == UBStylusTool::Line)
        return UBSettings::settings()->currentPenWidth();
    else if (stylusTool() == UBStylusTool::Marker)
        return UBSettings::settings()->currentMarkerWidth();
    else
        //failsafe
        return UBSettings::settings()->currentPenWidth();
}


void UBDrawingController::setLineWidthIndex(int index)
{
    if (stylusTool() == UBStylusTool::Marker)
    {
        UBSettings::settings()->setMarkerWidthIndex(index);
    }
    else
    {
        UBSettings::settings()->setPenWidthIndex(index);

        if(stylusTool() != UBStylusTool::Line
            && stylusTool() != UBStylusTool::Selector)
        {
            setStylusTool(UBStylusTool::Pen);
        }
    }

    emit lineWidthIndexChanged(index);
}


int UBDrawingController::currentToolColorIndex()
{
    if (stylusTool() == UBStylusTool::Pen || stylusTool() == UBStylusTool::Line)
    {
        return UBSettings::settings()->penColorIndex();
    }
    else if (stylusTool() == UBStylusTool::Marker)
    {
        return UBSettings::settings()->markerColorIndex();
    }
    else
    {
        return -1;
    }
}


QColor UBDrawingController::currentToolColor()
{
    return toolColor(UBSettings::settings()->isDarkBackground());
}


QColor UBDrawingController::toolColor(bool onDarkBackground)
{
    if (stylusTool() == UBStylusTool::Pen || stylusTool() == UBStylusTool::Line)
    {
        return UBSettings::settings()->penColor(onDarkBackground);
    }
    else if (stylusTool() == UBStylusTool::Marker)
    {
        return UBSettings::settings()->markerColor(onDarkBackground);
    }
    else
    {
        //failsafe
        if (onDarkBackground)
        {
            return Qt::white;
        }
        else
        {
            return Qt::black;
        }
    }
}


void UBDrawingController::setColorIndex(int index)
{
    Q_ASSERT(index >= 0 && index < UBSettings::settings()->colorPaletteSize);

    if (stylusTool() == UBStylusTool::Marker)
    {
        UBSettings::settings()->setMarkerColorIndex(index);
    }
    else
    {
        UBSettings::settings()->setPenColorIndex(index);
    }

    emit colorIndexChanged(index);
}


void UBDrawingController::setEraserWidthIndex(int index)
{
    setStylusTool(UBStylusTool::Eraser);
    UBSettings::settings()->setEraserWidthIndex(index);
}

void UBDrawingController::setPenColor(bool onDarkBackground, const QColor& color, int pIndex)
{
    if (onDarkBackground)
    {
        UBSettings::settings()->boardPenDarkBackgroundSelectedColors->setColor(pIndex, color);
    }
    else
    {
        UBSettings::settings()->boardPenLightBackgroundSelectedColors->setColor(pIndex, color);
    }

    emit colorPaletteChanged();
}


void UBDrawingController::setMarkerColor(bool onDarkBackground, const QColor& color, int pIndex)
{
    if (onDarkBackground)
    {
        UBSettings::settings()->boardMarkerDarkBackgroundSelectedColors->setColor(pIndex, color);
    }
    else
    {
        UBSettings::settings()->boardMarkerLightBackgroundSelectedColors->setColor(pIndex, color);
    }

    emit colorPaletteChanged();
}


void UBDrawingController::setMarkerAlpha(qreal alpha)
{
    UBSettings::settings()->boardMarkerLightBackgroundColors->setAlpha(alpha);
    UBSettings::settings()->boardMarkerLightBackgroundSelectedColors->setAlpha(alpha);

    UBSettings::settings()->boardMarkerDarkBackgroundColors->setAlpha(alpha);
    UBSettings::settings()->boardMarkerDarkBackgroundSelectedColors->setAlpha(alpha);

    UBSettings::settings()->boardMarkerAlpha->set(alpha);

    emit colorPaletteChanged();
}


void UBDrawingController::penToolSelected(bool checked)
{
    if (checked)
    {
        setStylusTool(UBStylusTool::Pen);

    }
}

void UBDrawingController::eraserToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Eraser);
}

void UBDrawingController::markerToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Marker);
}

void UBDrawingController::selectorToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Selector);
}

void UBDrawingController::playToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Play);
}

void UBDrawingController::handToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Hand);
}


void UBDrawingController::zoomInToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::ZoomIn);
}


void UBDrawingController::zoomOutToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::ZoomOut);
}


void UBDrawingController::pointerToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Pointer);
}


void UBDrawingController::lineToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Line);
}


void UBDrawingController::textToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Text);
}


void UBDrawingController::captureToolSelected(bool checked)
{
    if (checked)
        setStylusTool(UBStylusTool::Capture);
}

