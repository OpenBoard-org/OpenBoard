/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include "UBDesktopEraserPalette.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolbarButtonGroup.h"


#include "core/memcheck.h"

UBDesktopEraserPalette::UBDesktopEraserPalette(QWidget *parent)
    : UBPropertyPalette(Qt::Horizontal, parent)
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
