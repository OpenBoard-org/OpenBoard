#include "UBDesktopToolsPalette.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"

#include "gui/UBMainWindow.h"
#include "gui/UBToolbarButtonGroup.h"


UBDesktopToolsPalette::UBDesktopToolsPalette(QWidget *parent)
    : UBActionPalette(Qt::Horizontal, parent)
{
    // Setup color choice widget
    QList<QAction *> colorActions;
    colorActions.append(UBApplication::mainWindow->actionColor0);
    colorActions.append(UBApplication::mainWindow->actionColor1);
    colorActions.append(UBApplication::mainWindow->actionColor2);
    colorActions.append(UBApplication::mainWindow->actionColor3);

    UBToolbarButtonGroup *colorChoice =
            new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, colorActions);

    colorChoice->displayText(false);

    //connect(colorChoice, SIGNAL(activated(int)), this, SLOT(UBApplication::boardController->setColorIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), colorChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), colorChoice, SLOT(colorPaletteChanged()));

    layout()->addWidget(colorChoice);

    // Setup line width choice widget
    QList<QAction *> lineWidthActions;
    lineWidthActions.append(UBApplication::mainWindow->actionLineSmall);
    lineWidthActions.append(UBApplication::mainWindow->actionLineMedium);
    lineWidthActions.append(UBApplication::mainWindow->actionLineLarge);

    UBToolbarButtonGroup *lineWidthChoice =
            new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, lineWidthActions);
    lineWidthChoice->displayText(false);

    connect(lineWidthChoice, SIGNAL(activated(int))
            , UBDrawingController::drawingController(), SLOT(setLineWidthIndex(int)));

    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int))
            , lineWidthChoice, SLOT(setCurrentIndex(int)));

        layout()->addWidget(lineWidthChoice);

    // Setup eraser width choice widget
    QList<QAction *> eraserWidthActions;
    eraserWidthActions.append(UBApplication::mainWindow->actionEraserSmall);
    eraserWidthActions.append(UBApplication::mainWindow->actionEraserMedium);
    eraserWidthActions.append(UBApplication::mainWindow->actionEraserLarge);

    UBToolbarButtonGroup *eraserWidthChoice =
            new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, eraserWidthActions);

    connect(eraserWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setEraserWidthIndex(int)));

    eraserWidthChoice->displayText(false);
    eraserWidthChoice->setCurrentIndex(UBSettings::settings()->eraserWidthIndex());

    layout()->addWidget(eraserWidthChoice);

    addAction(UBApplication::mainWindow->actionEraseDesktopAnnotations);

}
