/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui>

#include "UBPageNumberPalette.h"

#include "core/memcheck.h"

UBPageNumberPalette::UBPageNumberPalette(QWidget *parent)
    : UBFloatingPalette(Qt::BottomRightCorner, parent)
{
    mLayout = new QHBoxLayout(this);
    mButton = new QPushButton(parent);
    mButton->setStyleSheet(QString("QPushButton { color: white; background-color: transparent; border: none; font-family: Arial; font-weight: bold; font-size: 20px }"));
    mButton->setFocusPolicy(Qt::NoFocus);
    connect(mButton, SIGNAL(clicked(bool)), this, SLOT(buttonClicked(bool)));

    mLayout->setContentsMargins(radius() + 15, 4, radius() + 15, 4);

    mLayout->addWidget(mButton);

    setPageNumber(0, 0);
}

UBPageNumberPalette::~UBPageNumberPalette()
{
    delete mButton;
    delete mLayout;
}

void UBPageNumberPalette::buttonClicked(bool checked)
{
    Q_UNUSED(checked);

    emit clicked();
}

int UBPageNumberPalette::radius()
{
    return 10;
}

void UBPageNumberPalette::setPageNumber(int current, int total)
{
    mButton->setText(QString("%1 / %2").arg(current).arg(total));
    adjustSizeAndPosition();
}
