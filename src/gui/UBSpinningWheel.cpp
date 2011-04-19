
#include "UBSpinningWheel.h"

#include <QtGui>

UBSpinningWheel::UBSpinningWheel(QWidget *parent)
    : QWidget(parent)
    , mPosition(9)
    , mTimerID(0)
{
    // NOOP
}

UBSpinningWheel::~UBSpinningWheel()
{
    stopAnimation();
}

QSize UBSpinningWheel::sizeHint() const
{
    return QSize(16, 16);
}

void UBSpinningWheel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    qreal side = qMin(width() / 2, height() / 2);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);

    QPen pen;
    pen.setWidthF(side / 6.5);
    pen.setCapStyle(Qt::RoundCap);

    painter.setPen(pen);
    painter.rotate(30 * (mPosition % 12));

    for(int i = 0; i < 12; i++)
    {
        painter.drawLine(QPointF(side / 2, 0), QPointF(0.9 * side, 0));
        painter.rotate(30);
        QColor color = pen.color();
        color.setAlphaF(0.25 + (i / 16.));
        pen.setColor(color);
        painter.setPen(pen);
    }
}

void UBSpinningWheel::startAnimation()
{
    if (mTimerID == 0)
    {
        mTimerID = startTimer(42);
    }
}

void UBSpinningWheel::stopAnimation()
{
    if (mTimerID != 0)
    {
        killTimer(mTimerID);
        mTimerID = 0;
    }
}

void UBSpinningWheel::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    mPosition.ref();
    update();
}




