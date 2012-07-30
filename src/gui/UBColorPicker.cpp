/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


