/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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

    mOriginalAlpha = 255;

    mLayout->setContentsMargins(radius() + 15, 4, radius() + 15, 4);

#ifdef Q_OS_OSX
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
        mTimer.start(50, this);
    }
    adjustSizeAndPosition();

    show();
    // showMessage may have been called from the GUI thread, so make sure the message window is drawn right now
    repaint();
    // on Linux, calling sendPostedEvents() with no filters creates an issue where "longpress events" slots are applied before it should (e.g, add a page between page 1
    // and 2 in a 500+ pages document => the newPage dialog appears like if you did a long click).
    //qApp->sendPostedEvents();

    // Moreover, on OSX, calling sendPostedEvents() doesn't solve the initial issue, where the messages are not displayed
    // todo : find a better solution for Mac, maybe by testing something like the following line (+ X11ExcludeTimers or #ifdef Q_OS_OSX)
    //QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::X11ExcludeTimers);

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
    // on Linux, calling sendPostedEvents() with no filters creates an issue where "longpress events" slots are applied before it should (e.g, add a page between page 1
    // and 2 in a 500+ pages document => the newPage dialog appears like if you did a long click).
    // Moreover, on OSX, calling sendPostedEvents() doesn't solve the initial issue, where the messages are not displayed
    // todo : find a better solution for Mac, maybe by testing something like the following line (+ X11ExcludeTimers or #ifdef Q_OS_OSX)
    //QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::X11ExcludeTimers);
}
