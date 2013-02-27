/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBIdleTimer.h"

#include <QApplication>
#include <QInputEvent>
#include <QTimer>
#include <QWidget>

#include "UBApplication.h"
#include "UBApplicationController.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "core/memcheck.h"

UBIdleTimer::UBIdleTimer(QObject *parent)
     : QObject(parent)
     , mCursorIsHidden(false)
{
    startTimer(100);
}

UBIdleTimer::~UBIdleTimer()
{
    // NOOP
}

bool UBIdleTimer::eventFilter(QObject *obj, QEvent *event)
{
    // if the event is an input event (mouse / tablet / keyboard)
    if (dynamic_cast<QInputEvent*>(event))
    {
        mLastInputEventTime = QDateTime::currentDateTime();
        if (mCursorIsHidden)
        {
            QApplication::restoreOverrideCursor();
            mCursorIsHidden = false;
        }
    }

    return QObject::eventFilter(obj, event);
}

void UBIdleTimer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

        bool hasWaitCuror = (UBApplication::overrideCursor() &&
                (UBApplication::overrideCursor()->shape() == Qt::WaitCursor));

    if (!mCursorIsHidden
            && (mLastInputEventTime.secsTo(QDateTime::currentDateTime()) >= 2)
                        && UBApplication::boardController
                        && UBApplication::boardController->controlView()
                        && UBApplication::boardController->controlView()->hasFocus()
                        && !hasWaitCuror)
    {
        QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
        mCursorIsHidden = true;
    }
}
