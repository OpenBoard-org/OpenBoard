/*
 * UNWindowCaptureUtils_win.cpp
 *
 *  Created on: Feb 3, 2009
 *      Author: julienbachmann
 */

#include "UBWindowCapture.h"

#include "UBWindowCaptureDelegate_win.h"
#include "UBDesktopAnnotationController.h"
#include "UBDesktopPalette.h"

#include "board/UBBoardView.h"

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
