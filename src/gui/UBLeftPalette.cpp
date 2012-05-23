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
#include "UBLeftPalette.h"
#include "core/UBSettings.h"

#include "core/memcheck.h"

/**
 * \brief The constructor
 */
UBLeftPalette::UBLeftPalette(QWidget *parent, const char *name):
    UBDockPalette(eUBDockPaletteType_LEFT, parent)
{
    setObjectName(name);
    setOrientation(eUBDockOrientation_Left);
    mCollapseWidth = 150;

    bool isCollapsed = false;
    if(mCurrentMode == eUBDockPaletteWidget_BOARD){
    	mLastWidth = UBSettings::settings()->leftLibPaletteBoardModeWidth->get().toInt();
    	isCollapsed = UBSettings::settings()->leftLibPaletteBoardModeIsCollapsed->get().toBool();
    }
    else{
    	mLastWidth = UBSettings::settings()->leftLibPaletteDesktopModeWidth->get().toInt();
    	isCollapsed = UBSettings::settings()->leftLibPaletteDesktopModeIsCollapsed->get().toBool();
    }

    if(isCollapsed)
    	resize(0,parentWidget()->height());
    else
    	resize(mLastWidth, parentWidget()->height());
}

/**
 * \brief The destructor
 */
UBLeftPalette::~UBLeftPalette()
{

}

/**
 * \brief Update the maximum width
 */
void UBLeftPalette::updateMaxWidth()
{
    setMaximumWidth(270);
}

/**
 * \brief Handle the resize event
 * @param event as the resize event
 */
void UBLeftPalette::resizeEvent(QResizeEvent *event)
{
	int newWidth = width();
	if(mCurrentMode == eUBDockPaletteWidget_BOARD){
		if(newWidth > mCollapseWidth)
			UBSettings::settings()->leftLibPaletteBoardModeWidth->set(newWidth);
		UBSettings::settings()->leftLibPaletteBoardModeIsCollapsed->set(newWidth == 0);
	}
	else{
		if(newWidth > mCollapseWidth)
			UBSettings::settings()->leftLibPaletteDesktopModeWidth->set(newWidth);
		UBSettings::settings()->leftLibPaletteDesktopModeIsCollapsed->set(newWidth == 0);
	}
    UBDockPalette::resizeEvent(event);
}


bool UBLeftPalette::switchMode(eUBDockPaletteWidgetMode mode)
{
	int newModeWidth;
	if(mode == eUBDockPaletteWidget_BOARD){
		mLastWidth = UBSettings::settings()->leftLibPaletteBoardModeWidth->get().toInt();
		newModeWidth = mLastWidth;
		if(UBSettings::settings()->leftLibPaletteBoardModeIsCollapsed->get().toBool())
			newModeWidth = 0;
	}
	else{
		mLastWidth = UBSettings::settings()->leftLibPaletteDesktopModeWidth->get().toInt();
		newModeWidth = mLastWidth;
		if(UBSettings::settings()->leftLibPaletteDesktopModeIsCollapsed->get().toBool())
			newModeWidth = 0;
	}
	resize(newModeWidth,height());
	return UBDockPalette::switchMode(mode);
}
