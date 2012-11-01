/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include "UBScreenMirror.h"

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#if defined(Q_WS_MAC)
#include <ApplicationServices/ApplicationServices.h>
#endif

#include "core/memcheck.h"


UBScreenMirror::UBScreenMirror(QWidget* parent)
    : QWidget(parent)
    , mScreenIndex(0)
    , mSourceWidget(0)
    , mTimerID(0)
{
    // NOOP
}


UBScreenMirror::~UBScreenMirror()
{
    // NOOP
}


void UBScreenMirror::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.fillRect(0, 0, width(), height(), QBrush(Qt::black));

    if (!mLastPixmap.isNull())
    {
        int x = (width() - mLastPixmap.width()) / 2;
        int y = (height() - mLastPixmap.height()) / 2;

        painter.drawPixmap(x, y, mLastPixmap);
    }
}


void UBScreenMirror::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    grabPixmap();

    update();
}

void UBScreenMirror::grabPixmap()
{
    if (mSourceWidget)
    {
        QPoint topLeft = mSourceWidget->mapToGlobal(mSourceWidget->geometry().topLeft());
        QPoint bottomRight = mSourceWidget->mapToGlobal(mSourceWidget->geometry().bottomRight());

        mRect.setTopLeft(topLeft);
        mRect.setBottomRight(bottomRight);
    }

    // get image of desktop

    WId windowID = qApp->desktop()->screen(mScreenIndex)->winId();
#if defined(Q_WS_MAC)
    // Available in Mac OS X v10.6 and later.
    CGRect grabRect;
    grabRect.origin.x = mRect.x();
    grabRect.origin.y = mRect.y();
    grabRect.size.width = mRect.width();
    grabRect.size.height = mRect.height();
    CGImageRef windowImage = CGWindowListCreateImage(grabRect
        ,kCGWindowListOptionOnScreenOnly
        ,windowID
        ,kCGWindowImageDefault);

    mLastPixmap = QPixmap::fromMacCGImageRef(windowImage);
#else

    mLastPixmap = QPixmap::grabWindow(windowID, mRect.x(), mRect.y(), mRect.width(), mRect.height());
#endif
    mLastPixmap = mLastPixmap.scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


void UBScreenMirror::setSourceWidget(QWidget *sourceWidget)
{
    mSourceWidget = sourceWidget;

    mScreenIndex = qApp->desktop()->screenNumber(sourceWidget);

    grabPixmap();

    update();
}


void UBScreenMirror::start()
{
    qDebug() << "mirroring START";
    UBApplication::boardController->freezeW3CWidgets(true);
    if (mTimerID == 0)
    {
        int ms = 125;

        bool success;
        int fps = UBSettings::settings()->mirroringRefreshRateInFps->get().toInt(&success);

        if (success && fps > 0)
        {
            ms = 1000 / fps;
        }

        mTimerID = startTimer(ms);
    }
    else
    {
        qDebug() << "UBScreenMirror::start() : Timer alrady running ...";
    }
}


void UBScreenMirror::stop()
{
    qDebug() << "mirroring STOP";
    UBApplication::boardController->freezeW3CWidgets(false);
    if (mTimerID != 0)
    {
        killTimer(mTimerID);
        mTimerID = 0;
    }
}
