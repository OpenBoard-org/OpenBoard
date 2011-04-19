
#include "UBStylusPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

UBStylusPalette::UBStylusPalette(QWidget *parent, Qt::Orientation orient)
    : UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mLastSelectedId(-1)
{
    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionPen;
    actions << UBApplication::mainWindow->actionEraser;
    actions << UBApplication::mainWindow->actionMarker;
    actions << UBApplication::mainWindow->actionSelector;

    actions << UBApplication::mainWindow->actionHand;
    actions << UBApplication::mainWindow->actionZoomIn;
    actions << UBApplication::mainWindow->actionZoomOut;

    actions << UBApplication::mainWindow->actionPointer;
    actions << UBApplication::mainWindow->actionLine;
    actions << UBApplication::mainWindow->actionText;
    actions << UBApplication::mainWindow->actionCapture;

    if(UBPlatformUtils::hasVirtualKeyboard())
        actions << UBApplication::mainWindow->actionVirtualKeyboard;

    setActions(actions);
    setButtonIconSize(QSize(42, 42));

    if(!UBPlatformUtils::hasVirtualKeyboard())
    {
            groupActions();
    }
    else
    {
            // VirtualKeyboard action is not in group
            // So, groupping all buttons, except last
            mButtonGroup = new QButtonGroup(this);
            for(int i=0; i < mButtons.size()-1; i++)
            {
                    mButtonGroup->addButton(mButtons[i], i);
            }
        connect(mButtonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(buttonGroupClicked(int)));
    }

    adjustSizeAndPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(doubleClicked()), this, SLOT(stylusToolDoubleClicked()));
    }

}


UBStylusPalette::~UBStylusPalette()
{
    // NOOP
}

void UBStylusPalette::stylusToolDoubleClicked()
{
    emit stylusToolDoubleClicked(mButtonGroup->checkedId());
}


