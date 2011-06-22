/*
 * UBGraphicsStroke.cpp
 *
 *  Created on: 28 sept. 2009
 *      Author: Luc
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
    mPolygons << pol;
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

