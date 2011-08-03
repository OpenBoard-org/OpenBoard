/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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



