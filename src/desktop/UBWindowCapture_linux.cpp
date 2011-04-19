/*
 * UNWindowCaptureUtils_win.cpp
 *
 *  Created on: Feb 3, 2009
 *      Author: julienbachmann
 */

#include "UBWindowCapture.h"
#include "UBDesktopAnnotationController.h"


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
    //TODO UB 4.x - Not used for now

    return QDialog::Rejected;
}
