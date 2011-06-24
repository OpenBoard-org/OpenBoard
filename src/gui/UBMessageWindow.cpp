/*
 * UBMessageWindow.cpp
 *
 *  Created on: Dec 5, 2008
 *      Author: Luc
 */

#include "UBMessageWindow.h"

#include "UBSpinningWheel.h"

#include "core/memcheck.h"

UBMessageWindow::UBMessageWindow(QWidget *parent)
    : UBFloatingPalette(Qt::BottomLeftCorner, parent)
    , mTimerID(-1)
{
    mLayout = new QHBoxLayout(this);
    mSpinningWheel = new UBSpinningWheel(parent);
    mLabel = new QLabel(parent);
    mLabel->setStyleSheet(QString("QLabel { color: white; background-color: transparent; border: none; font-family: Arial; font-size: 14px }"));

    mOriginalAlpha = mBackgroundBrush.color().alpha();

    mLayout->setContentsMargins(radius() + 15, 4, radius() + 15, 4);

#ifdef Q_WS_MAC
    mLayout->setContentsMargins(radius() + 15, 8, radius() + 15, 10);
#endif

    mLayout->addWidget(mSpinningWheel);
    mLayout->addWidget(mLabel);
}

UBMessageWindow::~UBMessageWindow()
{
    // NOOP
}

void UBMessageWindow::showMessage(const QString& message, bool showSpinningWheel)
{
    mTimer.stop();

    mLabel->setText(message);

    QColor fadedColor = mBackgroundBrush.color();
    fadedColor.setAlpha(mOriginalAlpha);
    mBackgroundBrush = QBrush(fadedColor);
    mFadingStep = 25;

    if (showSpinningWheel)
    {
        mSpinningWheel->show();
        mSpinningWheel->startAnimation();
    }
    else
    {
        mSpinningWheel->hide();
        mSpinningWheel->stopAnimation();
        mTimer.start(200, this);
    }
    adjustSizeAndPosition();

    show();
    // showMessage may have been called from the GUI thread, so make sure the message window is drawn right now
    repaint();
    // I mean it, *right now*, also on Mac
    qApp->flush();
}

void UBMessageWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    mFadingStep--;

    if (mFadingStep < 1)
    {
        hide();
        mTimer.stop();
        mSpinningWheel->stopAnimation();
    }
    else if (mFadingStep < 10)
    {
        QColor fadedColor = mBackgroundBrush.color();
        fadedColor.setAlpha(mOriginalAlpha / 10 * mFadingStep);
        mBackgroundBrush = QBrush(fadedColor);

        update();
    }
}

void UBMessageWindow::hideMessage()
{
    mFadingStep = 0;
    hide();
    qApp->flush();
}
