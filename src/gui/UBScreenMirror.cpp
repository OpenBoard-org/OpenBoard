/*
 * UBScreenMirror.cpp
 *
 *  Created on: Nov 25, 2008
 *      Author: luc
 */

#include "UBScreenMirror.h"

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"

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

    mLastPixmap = QPixmap::grabWindow(qApp->desktop()->screen(mScreenIndex)->winId(),
        mRect.x(), mRect.y(), mRect.width(), mRect.height());

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
    if (mTimerID != 0)
    {
        killTimer(mTimerID);
        mTimerID = 0;
    }
}
