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

    mLastWidth = UBSettings::settings()->leftLibPaletteWidth->get().toInt();
    mCollapseWidth = 150;

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
    UBSettings::settings()->leftLibPaletteWidth->set(width());
    UBDockPalette::resizeEvent(event);
}
