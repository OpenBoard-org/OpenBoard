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
