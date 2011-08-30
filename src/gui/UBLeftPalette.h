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
#ifndef UBLEFTPALETTE_H
#define UBLEFTPALETTE_H

#include "UBDockPalette.h"
#include "UBPageNavigationWidget.h"

class UBLeftPalette : public UBDockPalette
{
public:
    UBLeftPalette(QWidget* parent=0, const char* name="UBLeftPalette");
    ~UBLeftPalette();

    UBPageNavigationWidget* pageNavigator();

private:
    UBPageNavigationWidget* mpPageNavigator;

};

#endif // UBLEFTPALETTE_H
