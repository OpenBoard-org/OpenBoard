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



#include "UBDesktopPropertyPalette.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolbarButtonGroup.h"
#include "gui/UBRightPalette.h"

#include "core/memcheck.h"

UBDesktopPropertyPalette::UBDesktopPropertyPalette(QWidget *parent, UBRightPalette* _rightPalette)
    :UBPropertyPalette(Qt::Horizontal, parent)
    ,rightPalette(_rightPalette)
{}

int UBDesktopPropertyPalette::getParentRightOffset()
{
    return rightPalette->width();
}


UBDesktopPenPalette::UBDesktopPenPalette(QWidget *parent, UBRightPalette* rightPalette)
    : UBDesktopPropertyPalette(parent, rightPalette)
{
    // Setup color choice widget
    QList<QAction *> colorActions;
    colorActions.append(UBApplication::mainWindow->actionColor0);
    colorActions.append(UBApplication::mainWindow->actionColor1);
    colorActions.append(UBApplication::mainWindow->actionColor2);
    colorActions.append(UBApplication::mainWindow->actionColor3);
    colorActions.append(UBApplication::mainWindow->actionColor4);

    UBToolbarButtonGroup *colorChoice =
            new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, colorActions);

    colorChoice->displayText(false);

    //connect(colorChoice, SIGNAL(activated(int)), this, SLOT(UBApplication::boardController->setColorIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), colorChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), this, SLOT(close()));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), colorChoice, SLOT(colorPaletteChanged()));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), this, SLOT(close()));

    layout()->addWidget(colorChoice);

    // Setup line width choice widget
    QList<QAction *> lineWidthActions;
    lineWidthActions.append(UBApplication::mainWindow->actionLineSmall);
    lineWidthActions.append(UBApplication::mainWindow->actionLineMedium);
    lineWidthActions.append(UBApplication::mainWindow->actionLineLarge);

    UBToolbarButtonGroup *lineWidthChoice =
            new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, lineWidthActions);
    lineWidthChoice->displayText(false);

    connect(lineWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setLineWidthIndex(int)));
    connect(lineWidthChoice, SIGNAL(activated(int)), this, SLOT(close()));
    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), lineWidthChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), this, SLOT(close()));

    onParentMaximized();

    layout()->addWidget(lineWidthChoice);
}


void UBDesktopPenPalette::onButtonReleased()
{
    close();
}

/**
 * \brief Disconnect the released event of the buttons
 */
void UBDesktopPenPalette::onParentMinimized()
{
    for(int i = 0; i < mButtons.size(); i++)
    {
        disconnect(mButtons.at(i), SIGNAL(released()), this, SLOT(onButtonReleased()));
    }
}

/**
 * \brief Connect the released event of the buttons
 */
void UBDesktopPenPalette::onParentMaximized()
{
    for(int i = 0; i < mButtons.size(); i++)
    {
        connect(mButtons.at(i), SIGNAL(released()), this, SLOT(onButtonReleased()));
    }
}


UBDesktopEraserPalette::UBDesktopEraserPalette(QWidget *parent, UBRightPalette* rightPalette)
    : UBDesktopPropertyPalette(parent, rightPalette)
{
    // Setup eraser width choice widget
    QList<QAction *> eraserWidthActions;
    eraserWidthActions.append(UBApplication::mainWindow->actionEraserSmall);
    eraserWidthActions.append(UBApplication::mainWindow->actionEraserMedium);
    eraserWidthActions.append(UBApplication::mainWindow->actionEraserLarge);

    UBToolbarButtonGroup *eraserWidthChoice = new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, eraserWidthActions);

    connect(eraserWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setEraserWidthIndex(int)));
    connect(eraserWidthChoice, SIGNAL(activated(int)), this, SLOT(close()));
    connect(UBApplication::mainWindow->actionEraseDesktopAnnotations, SIGNAL(triggered()), this, SLOT(close()));

    eraserWidthChoice->displayText(false);
    eraserWidthChoice->setCurrentIndex(UBSettings::settings()->eraserWidthIndex());

    layout()->addWidget(eraserWidthChoice);

    addAction(UBApplication::mainWindow->actionEraseDesktopAnnotations);
}


UBDesktopMarkerPalette::UBDesktopMarkerPalette(QWidget *parent, UBRightPalette* rightPalette)
    : UBDesktopPropertyPalette(parent, rightPalette)
{
    // Setup color choice widget
    QList<QAction *> colorActions;
    colorActions.append(UBApplication::mainWindow->actionColor0);
    colorActions.append(UBApplication::mainWindow->actionColor1);
    colorActions.append(UBApplication::mainWindow->actionColor2);
    colorActions.append(UBApplication::mainWindow->actionColor3);
    colorActions.append(UBApplication::mainWindow->actionColor4);

    UBToolbarButtonGroup *colorChoice = new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, colorActions);
    colorChoice->displayText(false);

    //connect(colorChoice, SIGNAL(activated(int)), this, SLOT(UBApplication::boardController->setColorIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), colorChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), this, SLOT(close()));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), colorChoice, SLOT(colorPaletteChanged()));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), this, SLOT(close()));

    layout()->addWidget(colorChoice);

    // Setup line width choice widget
    QList<QAction *> lineWidthActions;
    lineWidthActions.append(UBApplication::mainWindow->actionLineSmall);
    lineWidthActions.append(UBApplication::mainWindow->actionLineMedium);
    lineWidthActions.append(UBApplication::mainWindow->actionLineLarge);

    UBToolbarButtonGroup *lineWidthChoice = new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, lineWidthActions);
    lineWidthChoice->displayText(false);

    connect(lineWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setLineWidthIndex(int)));
    connect(lineWidthChoice, SIGNAL(activated(int)), this, SLOT(close()));
    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), lineWidthChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), this, SLOT(close()));

    layout()->addWidget(lineWidthChoice);
}

