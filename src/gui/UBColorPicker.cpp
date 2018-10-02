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




#include "UBColorPicker.h"

#include <QtGui>

#include "core/memcheck.h"

UBColorPicker::UBColorPicker(QWidget* parent)
    : QFrame(parent)
    , mSelectedColorIndex(0)
{
    // NOOP
}

UBColorPicker::UBColorPicker(QWidget* parent, const QList<QColor>& colors, int pSelectedColorIndex)
    : QFrame(parent)
    , mColors(colors)
    , mSelectedColorIndex(pSelectedColorIndex)
{
    // NOOP
}


UBColorPicker::~UBColorPicker()
{
    // NOOP
}


void UBColorPicker::paintEvent ( QPaintEvent * event )
{
    Q_UNUSED(event);

    QPainter painter(this);

    if (mSelectedColorIndex < mColors.size())
    {
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setBrush(mColors.at(mSelectedColorIndex));

        painter.drawRect(0, 0, width(), height());
    }

}

void UBColorPicker::mousePressEvent ( QMouseEvent * event )
{
    if (event->buttons() & Qt::LeftButton)
    {
        mSelectedColorIndex++;

        if (mSelectedColorIndex >= mColors.size())
            mSelectedColorIndex = 0;

        repaint();

        emit colorSelected(mColors.at(mSelectedColorIndex));

    }
}


