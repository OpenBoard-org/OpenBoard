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

#ifndef UBDESKTOPSERVICES_H_
#define UBDESKTOPSERVICES_H_

#include <QtCore/qglobal.h>
#include <QDesktopServices>

// TODO Qt 4.6 #error Delete this UBDesktopServices class,
// it was used to work around a bug in Qt 4.4 that was fixed in Qt 4.5 and another that should be fixed
// in 4.5.1

class UBDesktopServices : public QDesktopServices
{
    public:
        static QString storageLocation(StandardLocation type);
};

#endif /* UBDESKTOPSERVICES_H_ */
