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

#include "UBGraphicsStroke.h"

#include "UBGraphicsPolygonItem.h"

#include "core/memcheck.h"

UBGraphicsStroke::UBGraphicsStroke()
{
    // NOOP
}


UBGraphicsStroke::~UBGraphicsStroke()
{
    // NOOP
}


void UBGraphicsStroke::addPolygon(UBGraphicsPolygonItem* pol)
{
    remove(pol);
    mPolygons << pol;
}

void UBGraphicsStroke::remove(UBGraphicsPolygonItem* polygonItem)
{
    int n = mPolygons.indexOf(polygonItem);
    if (n>=0)
        mPolygons.removeAt(n);
}

QList<UBGraphicsPolygonItem*> UBGraphicsStroke::polygons() const
{
    return mPolygons;
}


bool UBGraphicsStroke::hasPressure()
{
    if (mPolygons.count() > 2)
    {
        qreal nominalWidth = mPolygons.at(0)->originalWidth();

        foreach(UBGraphicsPolygonItem* pol, mPolygons)
        {
            if (!pol->isNominalLine() || pol->originalWidth() != nominalWidth)
                return true;
        }
    }

    return false;
}


UBGraphicsStroke* UBGraphicsStroke::deepCopy()
{
    UBGraphicsStroke* clone = new UBGraphicsStroke();

    return clone;
}

bool UBGraphicsStroke::hasAlpha() const
{
    if (mPolygons.length() > 0 && mPolygons.at(0)->color().alphaF() != 1.0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void UBGraphicsStroke::clear()
{
    if(!mPolygons.empty()){
        mPolygons.clear();
    }
}

