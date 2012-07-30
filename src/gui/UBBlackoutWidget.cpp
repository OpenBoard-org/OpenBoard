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

#include <QtGui>

#include "UBBlackoutWidget.h"

#include "core/memcheck.h"

UBBlackoutWidget::UBBlackoutWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
}


void UBBlackoutWidget::mousePressEvent(QMouseEvent *event)
{
        Q_UNUSED(event);
    doActivity();
}


void UBBlackoutWidget::keyPressEvent(QKeyEvent *event)
{
    if (!event->isAccepted())
    {
        if (event->key() == Qt::Key_B)
        {
            doActivity();
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
}


void UBBlackoutWidget::doActivity()
{
    emit activity();
}
