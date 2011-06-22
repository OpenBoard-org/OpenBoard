
#include <QtGui>

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "gui/UBMainWindow.h"

#include "UBWebToolsPalette.h"
#include "UBResources.h"
#include "UBIconButton.h"

#include "core/memcheck.h"

UBWebToolsPalette::UBWebToolsPalette(QWidget *parent,bool tutorialMode=false)
    : UBActionPalette(Qt::TopRightCorner, parent)
{
    QList<QAction*> actions;
    if (tutorialMode){
        actions << UBApplication::mainWindow->actionBoard;
    }
    else{
        actions << UBApplication::mainWindow->actionWebTrapFlash;
        //actions << UBApplication::mainWindow->actionWebTrap;

        actions << UBApplication::mainWindow->actionWebCustomCapture;
        actions << UBApplication::mainWindow->actionWebWindowCapture;
        actions << UBApplication::mainWindow->actionWebOEmbed;

        //actions << UBApplication::mainWindow->actionEduMedia;

        actions << UBApplication::mainWindow->actionWebShowHideOnDisplay;

        if (UBPlatformUtils::hasVirtualKeyboard())
        {
            actions << UBApplication::mainWindow->actionVirtualKeyboard;
        }
    }
    setActions(actions);
    setButtonIconSize(QSize(42, 42));
    adjustSizeAndPosition();
}


UBWebToolsPalette::~UBWebToolsPalette()
{
    // NOOP
}



