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

#ifdef Q_WS_MAC
#include <QtGui/QMacStyle>
#endif

#include "core/memcheck.h"

UBRubberBand::UBRubberBand(Shape s, QWidget * p)
    : QRubberBand(s, p)
{
    QStyle* rubberBandStyle = QRubberBand::style();

#ifdef Q_WS_WIN
    rubberBandStyle = new QWindowsXPStyle();
#elif defined(Q_WS_MAC)
    rubberBandStyle = new QMacStyle();
#endif
    QRubberBand::setStyle(rubberBandStyle);
}

UBRubberBand::~UBRubberBand()
{
    // NOOP
}
