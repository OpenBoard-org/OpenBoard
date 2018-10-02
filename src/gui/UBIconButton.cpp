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
