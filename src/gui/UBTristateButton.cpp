/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#include "UBTristateButton.h"

#include <QStyleOptionButton>
#include <QStylePainter>


UBTristateButton::UBTristateButton(QWidget* parent)
    : QToolButton(parent)
{
    connect(this, &QToolButton::clicked, this, &UBTristateButton::onClicked);
}

void UBTristateButton::setPixmap(Qt::CheckState checkState, QPixmap pixmap)
{
    switch (checkState)
    {
    case Qt::Unchecked:
        mPixmapUnchecked = pixmap;
        break;

    case Qt::PartiallyChecked:
        mPixmapPartiallyChecked = pixmap;
        break;

    case Qt::Checked:
        mPixmapChecked = pixmap;
        break;
    }
}

Qt::CheckState UBTristateButton::checkState() const
{
    return mCheckState;
}

void UBTristateButton::setCheckState(Qt::CheckState checkState)
{
    mCheckState = checkState;
    setChecked(checkState == Qt::Checked);

    update();
}

void UBTristateButton::onClicked(bool checked)
{
    setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void UBTristateButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // Choose the right pixmap
    QPixmap pix;

    switch (checkState())
    {
    case Qt::Unchecked:
        pix = mPixmapUnchecked;
        break;

    case Qt::PartiallyChecked:
        pix = mPixmapPartiallyChecked;
        break;

    case Qt::Checked:
        pix = mPixmapChecked;
        break;
    }


    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    // Paint the icon
    QStylePainter painter{this};
    opt.icon = QIcon(pix);
    painter.drawComplexControl(QStyle::CC_ToolButton, opt);
}
