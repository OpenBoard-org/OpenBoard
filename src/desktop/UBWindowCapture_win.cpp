/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include "UBWindowCapture.h"

#include "UBWindowCaptureDelegate_win.h"
#include "UBDesktopAnnotationController.h"
#include "UBDesktopPalette.h"

#include "board/UBBoardView.h"

#include "core/memcheck.h"

UBWindowCapture::UBWindowCapture(UBDesktopAnnotationController *parent)
        : QObject(parent)
        , mParent(parent)
{
    // NOOP
}


UBWindowCapture::~UBWindowCapture()
{
    // NOOP
}


const QPixmap UBWindowCapture::getCapturedWindow()
{
    return mWindowPixmap;
}


int UBWindowCapture::execute()
{
    mParent->desktopPalette()->grabMouse();
    mParent->desktopPalette()->grabKeyboard();

    UBWindowCaptureDelegate windowCaptureEventHandler;
    int result = windowCaptureEventHandler.execute();

    mParent->desktopPalette()->releaseMouse();
    mParent->desktopPalette()->releaseKeyboard();

    mWindowPixmap = windowCaptureEventHandler.getCapturedWindow();

    return result;
}
