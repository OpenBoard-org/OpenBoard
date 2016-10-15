/*
 * Copyright (C) 2015-2016 Département de l'Instruction Publique (DIP-SEM)
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
 * @return The points (or point, if none were interpolated) that were added
 */
QList<QPair<QPointF, qreal> > UBGraphicsStroke::addPoint(const QPointF& point, qreal width, UBInterpolator::InterpolationMethod interpolationMethod)
{
    strokePoint newPoint(point, width);

    int n = mReceivedPoints.size();

    if (n == 0) {
        mReceivedPoints << newPoint;
        mDrawnPoints << newPoint;
        return QList<strokePoint>();
    }

    if (interpolationMethod == UBInterpolator::NoInterpolation) {
        strokePoint lastPoint = mReceivedPoints.last();
        mReceivedPoints << newPoint;
        mDrawnPoints << newPoint;
        return QList<strokePoint>() << lastPoint << newPoint;
    }

    else if (interpolationMethod == UBInterpolator::Bezier) {
        // The curve we are interpolating is not between two drawn points;
        // it is between the midway points of the second-to-last and last point, and last and current point.

        // Don't draw segments smaller than a certain length. This can help with performance
        // (less polygons to draw) but mostly with keeping the curve smooth.
        qreal MIN_DISTANCE = 3*mAntiScaleRatio;
        qreal distance = QLineF(mReceivedPoints.last().first, newPoint.first).length();

        if (distance < MIN_DISTANCE) {
            return QList<strokePoint>() << mDrawnPoints.last();
        }

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

        UBQuadraticBezier bz;

        QPointF startPoint = (p1+p0)/2.0;
        QPointF endPoint = (p2+p1)/2.0;

        bz.setPoints(startPoint, p1, endPoint);

        QList<QPointF> calculated = bz.getPoints(10);
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
    qDebug() << "Simplifying. Before: " << points.size() << " points and " << polygons().size() << " polygons";

    /* Basic simplifying algorithm: consider A, B and C the current point and the two following ones.
     * If the angle between (AB) and (BC) is lower than a certain threshold,
     * the three points are considered to be aligned and the middle one (B) is removed.
     *
     * We then consider the two following points as the new B and C while keeping the same A, and
     * test these three points. As long as they are aligned, B is erased and we start over.
     * If not, the current B becomes the new A, and so on.
     *
     *
     * TODO: more advanced algorithm that could also simplify curved sections of the stroke
     */

    // angle difference in degrees between AB and BC below which the segments are considered colinear
    qreal threshold = UBSettings::settings()->boardSimplifyPenStrokesThresholdAngle->get().toReal();

    QList<strokePoint>::iterator it = points.begin();
    QList<QList<strokePoint>::iterator> toDelete;

    while (it+2 != points.end()) {
        QList<strokePoint>::iterator b_it(it+1);

        while (b_it+1 != points.end()) {
            qreal angle = qFabs(QLineF(it->first, b_it->first).angle() - QLineF(b_it->first, (b_it+1)->first).angle());

            if (angle < threshold)
                b_it = points.erase(b_it);
            else
                break;
        }

        if (b_it+1 == points.end())
            break;
        else
            it = b_it;
    }

    // Next, we iterate over the new points to build the polygons that make up the stroke

    QList<UBGraphicsPolygonItem*> newPolygons;
    QList<strokePoint> newStrokePoints;
    int i(0);

    while (i < points.size()) {
        bool drawCurve = false;

        newStrokePoints << points[i];

        // When a polygon is transparent and it overlaps with itself, it is *sometimes* filled incorrectly.
        // Limiting the size of the polygons, and creating new ones when the angle between consecutive points is above a
        // certain threshold helps mitigate this issue.
        // TODO: fix fill issue

        if (newStrokePoints.size() > 1 && i < points.size() - 1) {
            qreal angle = qFabs(UBGeometryUtils::angle(points[i-1].first, points[i].first, points[i+1].first));
            qDebug() << "Angle: " << angle;
            if (angle > 40 && angle < 320)
                drawCurve = true;
        }

        if (newStrokePoints.size() % 20 == 0)
            drawCurve = true;


        if (drawCurve) {
            UBGraphicsPolygonItem* poly = mScene->polygonToPolygonItem(UBGeometryUtils::curveToPolygon(newStrokePoints, true, true));
            //poly->setColor(QColor(rand()%256, rand()%256, rand()%256, poly->brush().color().alpha()));

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

    qDebug() << "After: " << points.size() << " points and " << newStroke->polygons().size() << " polygons";

    return newStroke;
}
