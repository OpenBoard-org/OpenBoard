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

#include "UBRubberBand.h"

#include <QtGui>
#include <QtGui/QPlastiqueStyle>
#include <QStyleFactory>

#ifdef Q_WS_MAC
#include <QtGui/QMacStyle>
#endif

#include "core/memcheck.h"

UBRubberBand::UBRubberBand(Shape s, QWidget * p)
    : QRubberBand(s, p)
{
    customStyle = NULL;

#ifdef Q_WS_WIN
    customStyle = new QWindowsXPStyle();
#elif defined(Q_WS_MAC)
    customStyle = new QMacStyle();
#elif defined(Q_WS_X11)
//    customStyle = QStyleFactory::create("oxygen");
#endif

    if (customStyle)
        QRubberBand::setStyle(customStyle);

}

UBRubberBand::~UBRubberBand()
{
    if (customStyle)
        delete customStyle;
}
