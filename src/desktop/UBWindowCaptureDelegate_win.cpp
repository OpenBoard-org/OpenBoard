/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBWindowCaptureDelegate_win.h"
#include <windows.h>
#include <QDialog>

#include "core/memcheck.h"

UBWindowCaptureDelegate::UBWindowCaptureDelegate(QObject *parent)
    :QObject(parent)
        , mIsCapturing(false)
        , mCancel(false)
        , mLastPoint(0, 0)
{
    // NOOP
}


UBWindowCaptureDelegate::~UBWindowCaptureDelegate(void)
{
    // NOOP
}


const QPixmap UBWindowCaptureDelegate::getCapturedWindow()
{
    return mCapturedPixmap;
}


int UBWindowCaptureDelegate::execute()
{
    mIsCapturing = true;
    mCancel = false;
    qApp->installEventFilter(this);

    while (mIsCapturing && !mCancel)
    {
        QPoint curPos = QCursor::pos();
        if (mLastPoint != curPos)
        {
            processPos(curPos);
            mLastPoint = curPos;
        }
        drawSelectionRect();
        qApp->processEvents();
    }

    if (!mCancel)
    {
        BringWindowToTop(mCurrentWindow);
        RedrawWindow(mCurrentWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        QWaitCondition sleep;
        QMutex mutex;
        mutex.lock();
        sleep.wait(&mutex, 200);
        mutex.unlock();
        mCapturedPixmap = QPixmap::grabWindow((WId)mCurrentWindow);
        return QDialog::Accepted;
    }
    else
    {
        RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        return QDialog::Rejected;
    }
}


void UBWindowCaptureDelegate::processPos(QPoint pPoint)
{
    POINT wPoint;

    wPoint.x = pPoint.x();
    wPoint.y = pPoint.y();
    HWND desktop = GetDesktopWindow();
    HWND newWindow = ChildWindowFromPointEx(desktop, wPoint, CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);

    if (mCurrentWindow != newWindow && newWindow != desktop)
    {
        RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        mCurrentWindow = newWindow;
    }
}


void UBWindowCaptureDelegate::drawSelectionRect()
{
    RECT windowRect;
    if (GetWindowRect(mCurrentWindow, &windowRect))
    {
        HDC hDC;
        HPEN hOldPen;
        HBRUSH hOldBrush;

        hDC = GetDC(NULL);

        hOldPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 5, RGB(255, 0, 0)));
        hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

        Rectangle(hDC, windowRect.left + 1, windowRect.top + 1, windowRect.right - 1, windowRect.bottom - 1);

        SelectObject(hDC, hOldBrush);
        DeleteObject(SelectObject(hDC, hOldPen));

        ReleaseDC(NULL, hDC);
    }
}


bool UBWindowCaptureDelegate::eventFilter(QObject *target, QEvent *event)
{
    Q_UNUSED(target);

    // once the application is deactivated, it means that the user clicked somewhere else and we shall
    // take the capture
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::WindowDeactivate)
    {
        mIsCapturing = false;
    }
    else if (event->type() == QEvent::KeyPress)
    {
        mCancel = true;
    }
    return true;
}

