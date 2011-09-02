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

UBLeftPalette::UBLeftPalette(QWidget *parent, const char *name):UBDockPalette(parent)
  , mpPageNavigator(NULL)
{
    setObjectName(name);
    setOrientation(eUBDockOrientation_Left);
    mLastWidth = 300;
    setMaximumWidth(300);
    resize(UBSettings::settings()->navigPaletteWidth->get().toInt(), height());

    mpLayout->setContentsMargins(customMargin(), customMargin(), 2*border() + customMargin(), customMargin());

    mpPageNavigator = new UBPageNavigationWidget(this);
    addTabWidget(mpPageNavigator);
}

UBLeftPalette::~UBLeftPalette()
{
    if(NULL != mpPageNavigator)
    {
        delete mpPageNavigator;
        mpPageNavigator = NULL;
    }
}

UBPageNavigationWidget* UBLeftPalette::pageNavigator()
{
    return mpPageNavigator;
}
