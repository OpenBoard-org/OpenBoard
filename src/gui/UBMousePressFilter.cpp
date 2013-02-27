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



#include "UBMousePressFilter.h"

#include "core/memcheck.h"

UBMousePressFilter::UBMousePressFilter(QObject* parent)
    : QObject(parent)
    , mPendingEvent(0)
{
    // NOOP
}


UBMousePressFilter::~UBMousePressFilter()
{
    // NOOP
}


bool UBMousePressFilter::eventFilter(QObject *obj, QEvent *event)
{
    const bool isMousePress = event->type() == QEvent::MouseButtonPress;
    const bool isMouseRelease = event->type() == QEvent::MouseButtonRelease;
    const bool isMouseMove = event->type() == QEvent::MouseMove;

    if (isMousePress)
    {
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent)
        {
            if (mPendingEvent)
            {
                delete mPendingEvent;
            }

            mPendingEvent = new QMouseEvent(QEvent::MouseButtonDblClick,
                mouseEvent->pos(), mouseEvent->globalPos(),
                mouseEvent->button(), mouseEvent->buttons(),
                mouseEvent->modifiers());

            mObject = obj;

            QTimer::singleShot(1000, this, SLOT(mouseDownElapsed()));
        }
    }
    else if (isMouseRelease || isMouseMove)
    {
        if (mPendingEvent)
        {
            QTabletEvent * tabletEvent = static_cast<QTabletEvent *>(event);
            QPoint point = tabletEvent->globalPos() - mPendingEvent->globalPos();
            if (isMouseRelease || point.manhattanLength() > QApplication::startDragDistance())
            {
                delete mPendingEvent;
                mPendingEvent = 0;
                mObject = 0;
            }
        }
    }

    return false;
}


void UBMousePressFilter::mouseDownElapsed()
{
    if (mPendingEvent && mObject)
    {
        QApplication::postEvent(mObject, mPendingEvent);
        mPendingEvent = 0;
        mObject = 0;
    }
}
