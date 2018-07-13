#include "UBDrawingPolygonPalette.h"

#include "core/UBApplication.h"
#include "UBMainWindow.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"

#include <QFocusEvent>

#include "board/UBBoardPaletteManager.h"
#include "gui/UBStylusPalette.h"


UBDrawingPolygonPalette::UBDrawingPolygonPalette(Qt::Orientation orient, QWidget *parent)
    :UBAbstractSubPalette(orient, parent)
{
    hide();

    QList<QAction*> actions;
    actions<<UBApplication::mainWindow->actionPolygon;
    actions<<UBApplication::mainWindow->actionSmartLine;
    actions<<UBApplication::mainWindow->actionSmartPen;

    setActions(actions);
    groupActions();

    //css
    layout()->setSpacing(0);
    mButtons.at(0)->setStyleSheet(styleSheetLeftGroupedButton);
    mButtons.at(1)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(2)->setStyleSheet(styleSheetRightGroupedButton);

    adjustSizeAndPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
}

UBActionPaletteButton * UBDrawingPolygonPalette::addActionButton(QAction * action)
{
    UBActionPaletteButton * button = new UBActionPaletteButton(action, this);
    button->setToolButtonStyle(mToolButtonStyle);
    mActions.append(action);
    layout()->addWidget(button);
    return button;
}

UBDrawingPolygonPalette::~UBDrawingPolygonPalette()
{

}

void UBDrawingPolygonPalette::buttonClicked()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>(sender());
    if (button)
    {
        QAction * action = button->defaultAction();

        if (action)
        {
            triggerAction(action);

            // Change the Action shown in the DrawingPalette :
            foreach (QAction* a, actionPaletteButtonParent()->actions()) {
                actionPaletteButtonParent()->removeAction(a); // Remove all older actions, in order to let only one action associated to the button.
            }
            actionPaletteButtonParent()->setDefaultAction(action); // Associate the new Action to the Button.
        }
    }

    hide();
}



void UBDrawingPolygonPalette::triggerAction(QAction *action)
{
    if (action)
    {        
        if (action == UBApplication::mainWindow->actionPolygon){
            UBApplication::boardController->shapeFactory().createPolygon(true);            
        }
        else if (action == UBApplication::mainWindow->actionSmartLine){
            UBApplication::boardController->shapeFactory().createLine(true);
        }
        else if (action == UBApplication::mainWindow->actionSmartPen){
            UBApplication::boardController->shapeFactory().createPen(true);
        }
    }
}
