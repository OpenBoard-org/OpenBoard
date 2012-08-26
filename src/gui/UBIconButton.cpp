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


#include <QtGui>

#include "UBIconButton.h"

#include "core/memcheck.h"


UBIconButton::UBIconButton(QWidget *parent, const QIcon &icon)
    : QAbstractButton(parent)
    , mToggleable(false)
{
    setIcon(icon);
    setCheckable(true);
    mIconSize = icon.actualSize(QSize(128, 128));
}

void UBIconButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pixmap = icon().pixmap(mIconSize, isChecked() ? QIcon::Selected : QIcon::Normal);
    painter.drawPixmap((width() - pixmap.width()) / 2, 0, pixmap);
}

void UBIconButton::mousePressEvent(QMouseEvent *event)
{
    if (!mToggleable)
        setChecked(true);

    QAbstractButton::mousePressEvent(event);
}

void UBIconButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit doubleClicked();
}

QSize UBIconButton::minimumSizeHint() const
{
    return mIconSize;
}
