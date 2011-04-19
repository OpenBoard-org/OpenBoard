/*
 * UBWidgetMirror.cpp
 *
 *  Created on: Nov 25, 2008
 *      Author: luc
 */

#include "UBWidgetMirror.h"

UBWidgetMirror::UBWidgetMirror(QWidget* sourceWidget, QWidget* parent)
    : QWidget(parent, 0)
    , mSourceWidget(sourceWidget)
{
    mSourceWidget->installEventFilter(this);
}

UBWidgetMirror::~UBWidgetMirror()
{
    // NOOP
}

bool UBWidgetMirror::eventFilter(QObject *obj, QEvent *event)
{
    bool result = QObject::eventFilter(obj, event);

    if (event->type() == QEvent::Paint && obj == mSourceWidget)
    {
        QPaintEvent *paintEvent = static_cast<QPaintEvent *>(event);
        update(paintEvent->rect());
    }

    return result;
}

void UBWidgetMirror::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(Qt::black);

    mSourceWidget->render(&painter, event->rect().topLeft(), QRegion(event->rect()));
}


void UBWidgetMirror::setSourceWidget(QWidget *sourceWidget)
{
    if (mSourceWidget)
    {
        mSourceWidget->removeEventFilter(this);
    }

    mSourceWidget = sourceWidget;

    mSourceWidget->installEventFilter(this);

    update();
}

