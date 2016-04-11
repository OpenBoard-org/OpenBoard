/*
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBGraphicsStroke.h"

#include "UBGraphicsPolygonItem.h"

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/memcheck.h"


UBGraphicsStroke::UBGraphicsStroke()
{
    mAntiScaleRatio = 1./(UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
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

/**
 * @brief Add a point to the curve, interpolating extra points if required
 * @return The points (or point, if none were interpolated) that were added
 */
QList<QPointF> UBGraphicsStroke::addPoint(const QPointF& point, UBInterpolator::InterpolationMethod interpolationMethod)
{
    int n = mDrawnPoints.size();

    if (n == 0) {
        mDrawnPoints << point;
        mAllPoints << point;
        return QList<QPointF>();
    }

    if (interpolationMethod == UBInterpolator::NoInterpolation) {
        QPointF lastPoint = mDrawnPoints.last();
        mDrawnPoints << point;
        mAllPoints << point;
        return QList<QPointF>() << lastPoint << point;
    }

    else if (interpolationMethod == UBInterpolator::Bezier) {
        // This is a bit special, as the curve we are interpolating is not between two drawn points;
        // it is between the midway points of the second-to-last and last point, and last and current point.

        // Don't draw segments smaller than a certain length. This can help with performance
        // (less polygons in a stroke) but mostly with keeping the curve smooth.
        qreal MIN_DISTANCE = 3*mAntiScaleRatio;
        qreal distance = QLineF(mDrawnPoints.last(), point).length();

        if (distance < MIN_DISTANCE) {
            return QList<QPointF>();
        }

        // The first segment is just a straight line to the first midway point
        if (n == 1) {
            QPointF lastPoint = mDrawnPoints[0];
            mDrawnPoints << point;

            return QList<QPointF>() << lastPoint << ((lastPoint + point)/2.0);
        }

        QPointF p0 = mDrawnPoints[mDrawnPoints.size() - 2];
        QPointF p1 = mDrawnPoints[mDrawnPoints.size() - 1];
        QPointF p2 = point;

        UBQuadraticBezier bz;

        QPointF startPoint = (p1+p0)/2.0;
        QPointF endPoint = (p2+p1)/2.0;

        bz.setPoints(startPoint, p1, endPoint);

        QList<QPointF> newPoints = bz.getPoints(10);

        foreach(QPointF p, newPoints) {
            mAllPoints << p;
        }

        mDrawnPoints << point;
        return newPoints;
    }

    return QList<QPointF>();
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
