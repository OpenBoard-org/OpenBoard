
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

