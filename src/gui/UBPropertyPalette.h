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
#ifndef UBPROPERTYPALETTE_H
#define UBPROPERTYPALETTE_H

#include <QMouseEvent>
#include <QPoint>

#include "UBActionPalette.h"

class UBPropertyPalette : public UBActionPalette
{
    Q_OBJECT
public:
    UBPropertyPalette(QWidget* parent=0, const char* name="propertyPalette");
    UBPropertyPalette(Qt::Orientation orientation, QWidget* parent = 0);
    ~UBPropertyPalette();

private slots:
    void onMouseRelease();
};

#endif // UBPROPERTYPALETTE_H
