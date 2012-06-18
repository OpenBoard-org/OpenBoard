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
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#include "UBRightPalette.h"

#include "core/memcheck.h"

/**
 * \brief The constructor
 */
UBRightPalette::UBRightPalette(QWidget *parent, const char *name):
    UBDockPalette(eUBDockPaletteType_RIGHT, parent)
{
    setObjectName(name);
    setOrientation(eUBDockOrientation_Right);

    mLastWidth = UBSettings::settings()->rightLibPaletteWidth->get().toInt();
    mCollapseWidth = 150;

    resize(mLastWidth, parentWidget()->height());
}

/**
 * \brief The destructor
 */
UBRightPalette::~UBRightPalette()
{
}

/**
 * \brief Handle the mouse move event
 * @event as the mouse move event
 */
void UBRightPalette::mouseMoveEvent(QMouseEvent *event)
{
    if(mCanResize)
    {
        UBDockPalette::mouseMoveEvent(event);
    }
}

/**
 * \brief Handle the resize event
 * @param event as the resize event
 */
void UBRightPalette::resizeEvent(QResizeEvent *event)
{
    UBDockPalette::resizeEvent(event);
    UBSettings::settings()->rightLibPaletteWidth->set(width());
    emit resized();
}

/**
 * \brief Update the maximum width
 */
void UBRightPalette::updateMaxWidth()
{
    setMaximumWidth((int)(parentWidget()->width() * 0.45));
    setMaximumHeight(parentWidget()->height());
    setMinimumHeight(parentWidget()->height());
}
