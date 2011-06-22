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
