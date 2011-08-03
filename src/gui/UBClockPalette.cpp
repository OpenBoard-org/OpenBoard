/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include <QtGui>


#include "UBClockPalette.h"
#include "core/memcheck.h"

UBClockPalette::UBClockPalette(QWidget *parent)
    : UBFloatingPalette(Qt::TopRightCorner, parent)
    , mTimeLabel(0)
{
    setLayout(new QHBoxLayout());
    mTimeLabel = new QLabel(parent);
    mTimeLabel->setStyleSheet(QString("QLabel {color: white; background-color: transparent; font-family: Arial; font-weight: bold; font-size: 20px}"));

    layout()->setContentsMargins(radius() + 15, 4, radius() + 15, 4);
    layout()->addWidget(mTimeLabel);

    mTimeFormat = QLocale::system().timeFormat(QLocale::ShortFormat);

    //strip seconds
    mTimeFormat = mTimeFormat.remove(":ss");
    mTimeFormat = mTimeFormat.remove(":s");

}


UBClockPalette::~UBClockPalette()
{
    // NOOP
}


int UBClockPalette::radius()
{
    return 10;
}


void UBClockPalette::updateTime()
{
    if (mTimeLabel)
    {
        mTimeLabel->setText(QLocale::system().toString (QTime::currentTime(), mTimeFormat));
    }
    adjustSizeAndPosition();
}


void UBClockPalette::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    updateTime();
}


void UBClockPalette::showEvent ( QShowEvent * event )
{
    Q_UNUSED(event);
    updateTime();
    mTimerID = startTimer(1000);
}


void UBClockPalette::hideEvent ( QShowEvent * event )
{
    Q_UNUSED(event);
    killTimer(mTimerID);
}

