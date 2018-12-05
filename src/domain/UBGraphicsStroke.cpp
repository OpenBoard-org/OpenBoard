/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
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
#include "domain/UBGraphicsScene.h"

#include "frameworks/UBGeometryUtils.h"


typedef QPair<QPointF, qreal> strokePoint;

UBGraphicsStroke::UBGraphicsStroke(UBGraphicsScene *scene)
    :mScene(scene)
{
    mAntiScaleRatio = 1./(UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
}


UBGraphicsStroke::~UBGraphicsStroke()
{
    foreach(UBGraphicsPolygonItem* poly, mPolygons)
        poly->setStroke(NULL);

    mPolygons.clear();
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
 * @param point The position of the point to add
 * @param width The width of the stroke at that point.
 * @param interpolate If true, a Bézier curve will be drawn rather than a straight line
 * @return A list containing the last point drawn plus the point(s) that were added
 *
 * This method should be called when a new point is given by the input method (mouse, pen or other), and the points that are returned
 * should be used to draw the actual stroke on-screen. This is because if interpolation (bézier curves) are to be used, the points to draw
 * do not correspond to the points that were given by the input method.
 *
 */
QList<QPair<QPointF, qreal> > UBGraphicsStroke::addPoint(const QPointF& point, qreal width, bool interpolate)
{
    strokePoint newPoint(point, width);

    int n = mReceivedPoints.size();

    if (n == 0) {
        mReceivedPoints << newPoint;
        mDrawnPoints << newPoint;
        return QList<strokePoint>() << newPoint;
    }

    if (!interpolate) {
        strokePoint lastPoint = mReceivedPoints.last();
        mReceivedPoints << newPoint;
        mDrawnPoints << newPoint;
        return QList<strokePoint>() << lastPoint << newPoint;
    }

    else {
        // The curve we are interpolating is not between two drawn points;
        // it is between the midway points of the second-to-last and last point, and last and current point.


        // The first segment is just a straight line to the first midway point
        if (n == 1) {
            QPointF lastPoint = mReceivedPoints[0].first;
            qreal lastWidth = mReceivedPoints[0].second;
            strokePoint p(((lastPoint+point)/2.0), (lastWidth+width)/2.0);
            mReceivedPoints << newPoint;
            mDrawnPoints << p;

            return QList<strokePoint>() << mReceivedPoints[0] << p;
        }

        QPointF p0 = mReceivedPoints[mReceivedPoints.size() - 2].first;
        QPointF p1 = mReceivedPoints[mReceivedPoints.size() - 1].first;
        QPointF p2 = point;

        QPointF startPoint = (p1+p0)/2.0;
        QPointF endPoint = (p2+p1)/2.0;

        QList<QPointF> calculated = UBGeometryUtils::quadraticBezier(startPoint, p1, endPoint, 10);
        QList<strokePoint> newPoints;

        qreal startWidth = mDrawnPoints.last().second;

        for (int i(0); i < calculated.size(); ++i) {
            qreal w = startWidth + (qreal(i)/qreal(calculated.size()-1)) * (width - startWidth);
            newPoints << strokePoint(calculated[i], w);
        }

        // avoid adding duplicates
        if (newPoints.first().first == mDrawnPoints.last().first)
            mDrawnPoints.removeLast();

        foreach(strokePoint p, newPoints)
            mDrawnPoints << p;

        mReceivedPoints << strokePoint(point, width);
        return newPoints;
    }

    return QList<strokePoint>();
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
        return false;
    }
    else
        return true;
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
 * @brief Return a simplified version of the stroke, with less points and polygons.
 *
 */
UBGraphicsStroke* UBGraphicsStroke::simplify()
{
    if (mDrawnPoints.size() < 3)
        return NULL;

    UBGraphicsStroke* newStroke = new UBGraphicsStroke();
    newStroke->mDrawnPoints = QList<strokePoint>(mDrawnPoints);

    QList<strokePoint>& points = newStroke->mDrawnPoints;
    //qDebug() << "Simplifying. Before: " << points.size() << " points and " << polygons().size() << " polygons";

    /* Basic simplifying algorithm: consider A, B and C the current point and the two following ones.
     * If the angle between (AB) and (BC) is lower than a certain threshold,
     * the three points are considered to be aligned and the middle one (B) is removed.
     *
     * We then consider the two following points as the new B and C while keeping the same A, and
     * test these three points. As long as they are aligned, B is erased and we start over.
     * If not, the current B becomes the new A, and so on.
     *
     * In case the stroke thickness varies a lot between A and B, then B is not removed even if it
     * should be according to the previous criteria.
     *
     * TODO: more advanced algorithm that could also simplify curved sections of the stroke
     */

    // angle difference in degrees between AB and BC below which the segments are considered colinear
    qreal thresholdAngle = UBSettings::settings()->boardSimplifyPenStrokesThresholdAngle->get().toReal();

    // Relative difference in thickness between two consecutive points (A and B) below which they are considered equal
    qreal thresholdWidthDifference = UBSettings::settings()->boardSimplifyPenStrokesThresholdWidthDifference->get().toReal();

    QList<strokePoint>::iterator it = points.begin();
    QList<QList<strokePoint>::iterator> toDelete;

    while (it+2 != points.end()) {
        // it, b_it and (b_it+1) correspond to A, B and C respectively
        QList<strokePoint>::iterator b_it(it+1);

        while (b_it+1 != points.end()) {
            qreal angle = qFabs(180-(UBGeometryUtils::angle(it->first, b_it->first, (b_it+1)->first)));
            qreal widthRatio = qMax(it->second, b_it->second)/qMin(it->second, b_it->second);

            if (angle < thresholdAngle && widthRatio < thresholdWidthDifference)
                b_it = points.erase(b_it);
            else
                break;
        }

        if (b_it+1 == points.end())
            break;
        else
            it = b_it;
    }

    // Next, we iterate over the new points to build the polygons that make up the stroke.
    // A new polygon is created every time drawCurve is true.

    QList<UBGraphicsPolygonItem*> newPolygons;
    QList<strokePoint> newStrokePoints;
    int i(0);

    while (i < points.size()) {
        bool drawCurve = false;

        newStrokePoints << points[i];

        // Limiting the size of the polygons, and creating new ones when there is a sharp angle between
        // consecutive point helps with two issues:
        // 1. When a polygon is transparent and it overlaps with itself, it is *sometimes* filled incorrectly.
        // 2. This way of simplifying strokes resuls in sharp, rather than rounded, corners when there is a sharp angle
        //    in the stroke

        if (newStrokePoints.size() > 1 && i < points.size() - 1) {
            qreal angle = qFabs(UBGeometryUtils::angle(points[i-1].first, points[i].first, points[i+1].first));
            if (angle < 150) // arbitrary threshold, change if necessary
                drawCurve = true;
        }

        if (hasAlpha() && newStrokePoints.size() % 20 == 0)
            drawCurve = true;

        if (drawCurve) {
            UBGraphicsPolygonItem* poly = mScene->polygonToPolygonItem(UBGeometryUtils::curveToPolygon(newStrokePoints, true, true));
            //poly->setColor(QColor(rand()%256, rand()%256, rand()%256, poly->brush().color().alpha())); // useful for debugging

            // Subtract overlapping polygons if the stroke is translucent
            if (!poly->brush().isOpaque()) {
                foreach(UBGraphicsPolygonItem* prev, newPolygons)
                    poly->subtract(prev);
            }

            newPolygons << poly;
            newStrokePoints.clear();
            --i;
        }

        ++i;
    }

    if (newStrokePoints.size() > 0) {
        UBGraphicsPolygonItem* poly = mScene->polygonToPolygonItem(UBGeometryUtils::curveToPolygon(newStrokePoints, true, true));

        if (!poly->brush().isOpaque()) {
            foreach(UBGraphicsPolygonItem* prev, newPolygons)
                poly->subtract(prev);
        }

        newPolygons << poly;
    }


    newStroke->mPolygons = QList<UBGraphicsPolygonItem*>(newPolygons);

    foreach(UBGraphicsPolygonItem* poly, newStroke->mPolygons) {
        poly->setFillRule(Qt::WindingFill);
        poly->setStroke(newStroke);
    }

    //qDebug() << "After: " << points.size() << " points and " << newStroke->polygons().size() << " polygons";

    return newStroke;
}
