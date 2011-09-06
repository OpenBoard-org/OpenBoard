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
    mCollapseWidth = 180;
/*
    resize(UBSettings::settings()->libPaletteWidth->get().toInt(), parentWidget()->height());
    mpLayout->setContentsMargins(2*border() + customMargin(), customMargin(), customMargin(), customMargin());
*/

    resize(UBSettings::settings()->navigPaletteWidth->get().toInt(), parentWidget()->height());
    mpLayout->setContentsMargins(customMargin(), customMargin(), 2*border() + customMargin(), customMargin());

    // Add the widgets here
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

void UBLeftPalette::updateMaxWidth()
{
    setMaximumWidth(300);
}

void UBLeftPalette::resizeEvent(QResizeEvent *event)
{
    UBDockPalette::resizeEvent(event);
    if(NULL != mpPageNavigator)
    {
        mpPageNavigator->setMinimumHeight(height() - 2*border());
    }
    UBSettings::settings()->navigPaletteWidth->set(width());
}

UBPageNavigationWidget* UBLeftPalette::pageNavigator()
{
    return mpPageNavigator;
}
