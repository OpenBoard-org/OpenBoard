/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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




/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "WBChaseWidget.h"

#include <QtGui>
#include <QApplication>

#include "core/memcheck.h"

WBChaseWidget::WBChaseWidget(QWidget *parent, QPixmap pixmap, bool pixmapEnabled)
    : QWidget(parent)
    , mSegment(0)
    , mDelay(100)
    , mStep(40)
    , mTimerID(-1)
    , mAnimated(false)
    , mPixmap(pixmap)
    , mPixmapEnabled(pixmapEnabled)
{
    // NOOP
}


void WBChaseWidget::setAnimated(bool value)
{
    if (mAnimated == value)
        return;
    mAnimated = value;
    if (mTimerID != -1) 
    {
        killTimer(mTimerID);
        mTimerID = -1;
    }
    if (mAnimated) 
    {
        mSegment = 0;
        mTimerID = startTimer(mDelay);
    }
    update();
}


void WBChaseWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    if (mPixmapEnabled && !mPixmap.isNull()) 
    {
        p.drawPixmap(0, 0, mPixmap);
        return;
    }

    const int extent = qMin(width() - 8, height() - 8);
    const int displ = extent / 4;
    const int ext = extent / 4 - 1;

    p.setRenderHint(QPainter::Antialiasing, true);

    if(mAnimated)
        p.setPen(Qt::gray);
    else
        p.setPen(QPen(palette().dark().color()));

    p.translate(width() / 2, height() / 2); // center

    for (int segment = 0; segment < segmentCount(); ++segment) 
    {
        p.rotate(QApplication::isRightToLeft() ? mStep : -mStep);
        if(mAnimated)
            p.setBrush(colorForSegment(segment));
        else
            p.setBrush(palette().background());
        p.drawEllipse(QRect(displ, -ext / 2, ext, ext));
    }
}


QSize WBChaseWidget::sizeHint() const
{
    return QSize(32, 32);
}


void WBChaseWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mTimerID) 
    {
        ++mSegment;
        update();
    }
    QWidget::timerEvent(event);
}


QColor WBChaseWidget::colorForSegment(int seg) const
{
    int index = ((seg + mSegment) % segmentCount());
    int comp = qMax(0, 255 - (index * (255 / segmentCount())));
    return QColor(comp, comp, comp, 255);
}


int WBChaseWidget::segmentCount() const
{
    return 360 / mStep;
}


void WBChaseWidget::setPixmapEnabled(bool enable)
{
    mPixmapEnabled = enable;
}

