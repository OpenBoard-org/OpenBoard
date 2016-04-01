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

/**
 * @brief Add a point to the curve, interpolating extra points if required
 * @return The points (or point, if none were interpolated) that were added
 */
QList<QPointF> UBGraphicsStroke::addPoint(const QPointF& point, UBInterpolator::InterpolationMethod interpolationMethod)
{
    int n = mDrawnPoints.size();

    /*
    if (n > 0) {
        qreal MIN_DISTANCE = 3;
        QPointF lastPoint = mDrawnPoints.last();
        qreal distance = QLineF(lastPoint, point).length();

        //qDebug() << "distance: " << distance;

        if (distance < MIN_DISTANCE)
            return QList<QPointF>();
    }
    */

    if (interpolationMethod == UBInterpolator::NoInterpolation || n == 0) {
        mDrawnPoints << point;
        mAllPoints << point;
        return QList<QPointF>() << point;
    }

    else if (interpolationMethod == UBInterpolator::Bezier) {
        // This is a bit special, as the curve we are interpolating is not between two drawn points;
        // it is between the midway points of the second-to-last and last point, and last and current point.
        qreal MIN_DISTANCE = 3;
        QPointF lastPoint = mDrawnPoints.last();
        qreal distance = QLineF(lastPoint, point).length();

        //qDebug() << "distance: " << distance;

        if (distance < MIN_DISTANCE)
            return QList<QPointF>();
        if (n == 1) {
            // We start with a straight line to the first midway point
            QPointF lastPoint = mDrawnPoints[0];
            mDrawnPoints << point;

            return QList<QPointF>() << ((lastPoint + point)/2.0);
        }

        QPointF p0 = mDrawnPoints[mDrawnPoints.size() - 2];
        QPointF p1 = mDrawnPoints[mDrawnPoints.size() - 1];
        QPointF p2 = point;

        UBQuadraticBezier bz;

        QPointF startPoint = (p1+p0)/2.0;
        QPointF endPoint = (p2+p1)/2.0;

        bz.setPoints(startPoint, p1, endPoint);

        QList<QPointF> newPoints = bz.getPoints(7);

        foreach(QPointF p, newPoints) {
            mAllPoints << p;
        }

        mDrawnPoints << point;
        return newPoints;


    }

    else {

        qreal MIN_INTERPOLATION_DISTANCE = 0;

        QPointF lastPoint;
        if (!mDrawnPoints.isEmpty())
            lastPoint = mDrawnPoints.last();

        QList<QPointF> newPoints;

        // Interpolation
        if (n > 3 && QLineF(lastPoint, point).length() > MIN_INTERPOLATION_DISTANCE) {

            /*
            UBSimpleSpline sp;
            sp.setPoints(mDrawnPoints[n-2], mDrawnPoints[n-1], point);
            */

            // todo: better way of avoiding problems with constant x's. in the meantime this'll do.
            qreal x0 = mDrawnPoints[n-3].x();
            qreal x1 = mDrawnPoints[n-2].x();
            qreal x2 = mDrawnPoints[n-1].x();
            qreal x3 = point.x();
            if (!(x0 == x1 || x0 == x2 || x0 == x3 || x1 == x2 || x1 == x3 || x2 == x3)) {

                UBCatmullRomSpline sp;
                sp.setPoints(QList<QPointF>() << mDrawnPoints[n-3] << mDrawnPoints[n-2] << mDrawnPoints[n-1] << point);

                // get an extra 3 values in between the current point and last one
                int n_points = 3; // number of points to interpolate
                double interval = (point.x() - lastPoint.x())/double(n_points+1);

                qDebug() << "Interpolating between: " << lastPoint << " and " << point;

                for (int i(1); i <= n_points; ++i) {
                    double x = lastPoint.x() + i*interval;
                    QPointF newPoint(x, sp.y(x));
                    qDebug() << newPoint;

                    newPoints << newPoint;
                    mAllPoints << newPoint;
                    //qDebug() << "Got new point: " << newPoint;
                }
            }

        }

        newPoints << point;
        mAllPoints << point;
        mDrawnPoints << point;

        return newPoints;
    }
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


/**
 * @brief Smoothe the curve, by interpolating extra points where needed.
 *
 * @return A new stroke based on the current one.
 */
UBGraphicsStroke* UBGraphicsStroke::smoothe()
{
    // Catmull-Rom spline interpolation
    UBCatmullRomSpline sp;

    UBGraphicsStroke * smoothStroke = new UBGraphicsStroke();

    smoothStroke->mAllPoints << mAllPoints[0];

    for (int i(0); i < mAllPoints.size() - 3; ++i) {
        QPointF p1, p2;

        p1 = mAllPoints[i+1];
        p2 = mAllPoints[i+2];

        qreal x0 = mAllPoints[i].x();
        qreal x1 = p1.x();
        qreal x2 = p2.x();
        qreal x3 = mAllPoints[i+3].x();

        if (!(x0 == x1 || x0 == x2 || x0 == x3 || x1 == x2 || x1 == x3 || x2 == x3)) {
            sp.setPoints(QList<QPointF>() << mAllPoints[i] << mAllPoints[i+1] << mAllPoints[i+2] << mAllPoints[i+3]);

            smoothStroke->mAllPoints << mAllPoints[i+1];
            int n_points = 3; // number of points to interpolate
            double interval = (p2.x() - p1.x())/double(n_points+1);

            for (int i(1); i <= n_points; ++i) {
                double x = p1.x() + i*interval;
                QPointF newPoint(x, sp.y(x));

                smoothStroke->mAllPoints << newPoint;
            }
        }

    }
    smoothStroke->mAllPoints << mAllPoints[mAllPoints.size() - 2] << mAllPoints[mAllPoints.size() - 1];


    return smoothStroke;
}
