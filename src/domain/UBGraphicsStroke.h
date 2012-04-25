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

#ifndef UBGRAPHICSSTROKE_H_
#define UBGRAPHICSSTROKE_H_

#include <QtGui>

#include "core/UB.h"


class UBGraphicsPolygonItem;

class UBGraphicsStroke
{
    friend class UBGraphicsPolygonItem;

    public:
        UBGraphicsStroke();
        virtual ~UBGraphicsStroke();

        bool hasPressure();

        QList<UBGraphicsPolygonItem*> polygons() const;

        void remove(UBGraphicsPolygonItem* polygonItem); 

        UBGraphicsStroke *deepCopy();

        bool hasAlpha() const;

        void clear();

    protected:
        void addPolygon(UBGraphicsPolygonItem* pol);

    private:

        QList<UBGraphicsPolygonItem*> mPolygons;

};

#endif /* UBGRAPHICSSTROKE_H_ */
