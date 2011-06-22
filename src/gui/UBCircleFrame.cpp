/*
 * UBCircleFrame.cpp
 *
 *  Created on: Nov 18, 2008
 *      Author: luc
 */

#include "UBCircleFrame.h"
#include "core/memcheck.h"

UBCircleFrame::UBCircleFrame(QWidget* parent)
    : QFrame(parent)
{
    // NOOP
}

UBCircleFrame::~UBCircleFrame()
{
    // NOOP
}


void UBCircleFrame::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(Qt::black));

    qreal diameter = (width() - 2) / maxPenWidth * currentPenWidth;
    diameter = qMax(2.0, diameter);
    qreal x = (width() - diameter) / 2;

    painter.drawEllipse(x, x, diameter, diameter);
}
