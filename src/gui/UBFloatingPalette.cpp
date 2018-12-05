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




#include <QtGui>
#include <QPainterPath>

#include "UBFloatingPalette.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBSettings.h"

#include "core/memcheck.h"

UBFloatingPalette::UBFloatingPalette(Qt::Corner position, QWidget *parent)
    : QWidget(parent, parent ? Qt::Widget : Qt::Tool | (Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint))
    , mCustomPosition(false)
    , mIsMoving(false)
    , mCanBeMinimized(false)
    , mMinimizedLocation(eMinimizedLocation_None)
    , mDefaultPosition(position)
{
    setCursor(Qt::ArrowCursor);

    if (parent)
    {
        setAttribute(Qt::WA_NoMousePropagation);
    }
    else
    {
        // standalone window
        // !!!! Should be included into Windows after QT recompilation
#ifndef Q_OS_WIN
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_MacAlwaysShowToolWindow);
#endif
#ifdef Q_OS_OSX
        setAttribute(Qt::WA_MacAlwaysShowToolWindow);
        setAttribute(Qt::WA_MacNoShadow);
#endif
    }

    mBackgroundBrush = QBrush(UBSettings::paletteColor);
    mbGrip = true;
}

void UBFloatingPalette::setGrip(bool newGrip)
{
    mbGrip = newGrip;
    update();
}


void UBFloatingPalette::setBackgroundBrush(const QBrush& brush)
{
    if (mBackgroundBrush != brush)
    {
        mBackgroundBrush = brush;
        update();
    }
}


void UBFloatingPalette::setCustomPosition(bool pFlag)
{
    mCustomPosition = pFlag;

    if (pFlag)
        removeAllAssociatedPalette();

}

int UBFloatingPalette::radius()
{
    return 10;
}


int UBFloatingPalette::border()
{
    return 0;
}


void UBFloatingPalette::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    emit mouseEntered();
}

void UBFloatingPalette::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mIsMoving = true;
        mDragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
    else
    {
        QWidget::mousePressEvent(event);
    }
}

void UBFloatingPalette::mouseMoveEvent(QMouseEvent *event)
{
    if (mIsMoving)
    {
        moveInsideParent(event->globalPos() - mDragPosition);
        event->accept();
        emit moving();
    }
    else
    {
        QWidget::mouseMoveEvent(event);
    }
}

void UBFloatingPalette::mouseReleaseEvent(QMouseEvent *event)
{
    if (mIsMoving)
    {
        mIsMoving = false;
        setCustomPosition(true);
        event->accept();
    }
    else
    {
        QWidget::mouseReleaseEvent(event);
    }
}

int UBFloatingPalette::getParentRightOffset()
{
    return 0;
}

void UBFloatingPalette::moveInsideParent(const QPoint &position)
{
    QWidget *parent = parentWidget();

    if (parent)
    {
        int margin = UBSettings::boardMargin - border();
        qreal newX = qMax(margin, qMin(parent->width() - getParentRightOffset() - width() - margin, position.x()));
        qreal newY = qMax(margin, qMin(parent->height() - height() - margin, position.y()));

        if (!mCustomPosition && !mIsMoving)
        {
            if (mDefaultPosition == Qt::TopLeftCorner || mDefaultPosition == Qt::BottomLeftCorner)
            {
                newX = margin;
            }
            else
            {
                newX = qMax(margin, parent->width() - getParentRightOffset() - width() - margin);
            }
        }
        move(newX, newY);
        minimizePalette(QPoint(newX, newY));
    }
    else
    {
        move(position);
    }
}

void UBFloatingPalette::addAssociatedPalette(UBFloatingPalette* pOtherPalette)
{
    mAssociatedPalette.append(pOtherPalette);
}

void UBFloatingPalette::removeAssociatedPalette(UBFloatingPalette* pOtherPalette)
{
    mAssociatedPalette.removeOne(pOtherPalette);
}

QSize UBFloatingPalette::preferredSize()
{
    QSize palettePreferredSize = sizeHint();

    if (palettePreferredSize.width() == 0)
    {
        palettePreferredSize = childrenRect().size();
    }

    return palettePreferredSize;
}

void UBFloatingPalette::adjustSizeAndPosition(bool pUp)
{
    QSize newPreferredSize = preferredSize();

    foreach (UBFloatingPalette* palette, mAssociatedPalette)
    {
        QSize palettePreferredSize = palette->preferredSize();
        newPreferredSize.setWidth(newPreferredSize.expandedTo(palettePreferredSize).width());
    }
    QSize previousSize = size();
    int biggerHeight = preferredSize().height() - previousSize.height();
    if ((pUp && (biggerHeight > 0))
        || (!pUp && (biggerHeight < 0)))
    {
        move(pos().x(), pos().y() - biggerHeight);
    }

    if (newPreferredSize != size())
    {
        resize(newPreferredSize);
        moveInsideParent(pos());
        foreach(UBFloatingPalette* palette, mAssociatedPalette)
        {
            palette->move(pos().x(), palette->pos().y());
            palette->resize(newPreferredSize.width(), palette->size().height());
        }
    }
}

void UBFloatingPalette::removeAllAssociatedPalette()
{
    foreach (UBFloatingPalette* palette, mAssociatedPalette)
    {
        palette->removeAssociatedPalette(this);
        removeAssociatedPalette(palette);
    }
}

void UBFloatingPalette::showEvent(QShowEvent *)
{
    moveInsideParent(pos());
}

void UBFloatingPalette::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(mBackgroundBrush);

    if(mbGrip)
    {
        painter.setBrush(QBrush(QColor(170, 170 ,170)));
        QPainterPath borderPath;
        borderPath.addRoundedRect(0, 0, width(), height(), radius(), radius());
        borderPath.addRoundedRect(border(), border(), width() - 2 * border(), height() - 2 * border(), radius(), radius());
        painter.drawPath(borderPath);
        painter.setBrush(mBackgroundBrush);
        painter.drawRoundedRect(border(), border(), width() - 2 * border(), height() - 2 * border(), radius(), radius());
    }
    else
    {
        painter.drawRoundedRect(border(), border(), width() - 2 * border(), height() - 2 * border(), radius(), radius());
    }
}

int UBFloatingPalette::gripSize()
{
    return 5;
}

void UBFloatingPalette::minimizePalette(const QPoint& pos)
{
    if(!mCanBeMinimized)
    {
        //  If this floating palette cannot be minimized, we exit this method.
    return;
    }

    if(mMinimizedLocation == eMinimizedLocation_None)
    {
    //  Verify if we have to minimize this palette
    if(pos.x() == 5)
    {
        mMinimizedLocation = eMinimizedLocation_Left;
    }
//    else if(pos.y() == 5)
//    {
//        mMinimizedLocation = eMinimizedLocation_Top;
//    }
    else if(pos.x() == parentWidget()->width() - getParentRightOffset() - width() - 5)
    {
        mMinimizedLocation = eMinimizedLocation_Right;
    }
//    else if(pos.y() == parentSize.height() - height() - 5)
//    {
//        mMinimizedLocation = eMinimizedLocation_Bottom;
//    }

    //  Minimize the Palette
    if(mMinimizedLocation != eMinimizedLocation_None)
    {
        emit minimizeStart(mMinimizedLocation);
    }
    }
    else
    {
    //  Restore the palette
    if(pos.x() > 5 &&
       pos.y() > 5 &&
       pos.x() < parentWidget()->width() - getParentRightOffset()  - width() - 5 &&
       pos.y() < parentWidget()->size().height() - height() - 5)
    {
        mMinimizedLocation = eMinimizedLocation_None;
        emit maximizeStart();
    }
    }
}

void UBFloatingPalette::setMinimizePermission(bool permission)
{
    mCanBeMinimized = permission;
}
