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




#include "UBGeometryUtils.h"

#include "core/memcheck.h"

const double PI = 4.0 * atan(1.0);
const int UBGeometryUtils::centimeterGraduationHeight = 15;
const int UBGeometryUtils::halfCentimeterGraduationHeight = 10;
const int UBGeometryUtils::millimeterGraduationHeight = 5;
const int UBGeometryUtils::millimetersPerCentimeter = 10;
const int UBGeometryUtils::millimetersPerHalfCentimeter = 5;
const float UBGeometryUtils::inchSize = 25.4f;

UBGeometryUtils::UBGeometryUtils()
{
    // NOOP
}

UBGeometryUtils::~UBGeometryUtils()
{
    // NOOP
}

QPolygonF UBGeometryUtils::lineToPolygon(const QLineF& pLine, const qreal& pWidth)
{
    qreal x1 = pLine.x1();
    qreal y1 = pLine.y1();

    qreal x2 = pLine.x2();
    qreal y2 = pLine.y2();

    qreal alpha = (90.0 - pLine.angle()) * PI / 180.0;
    qreal hypothenuse = pWidth / 2;

    // TODO UB 4.x PERF cache sin/cos table
    qreal opposite = sin(alpha) * hypothenuse;
    qreal adjacent = cos(alpha) * hypothenuse;

    QPointF p1a(x1 - adjacent, y1 - opposite);
    QPointF p1b(x1 + adjacent, y1 + opposite);

    QPointF p2a(x2 - adjacent, y2 - opposite);
    QPointF p2b(x2 + adjacent, y2 + opposite);

    QPainterPath painterPath;

    painterPath.moveTo(p1a);
    painterPath.lineTo(p2a);

    painterPath.arcTo(x2 - hypothenuse, y2 - hypothenuse, pWidth, pWidth, (90.0 + pLine.angle()), -180.0);

    //painterPath.lineTo(p2b);
    painterPath.lineTo(p1b);

    painterPath.arcTo(x1 - hypothenuse, y1 - hypothenuse, pWidth, pWidth, -1 * (90.0 - pLine.angle()), -180.0);

    painterPath.closeSubpath();

    return painterPath.toFillPolygon();
}



QPolygonF UBGeometryUtils::lineToPolygon(const QLineF& pLine, const qreal& pStartWidth, const qreal& pEndWidth)
{
    qreal x1 = pLine.x1();
    qreal y1 = pLine.y1();

    qreal x2 = pLine.x2();
    qreal y2 = pLine.y2();

    qreal alpha = (90.0 - pLine.angle()) * PI / 180.0;
    qreal startHypothenuse = pStartWidth / 2;
    qreal endHypothenuse = pEndWidth / 2;

    // TODO UB 4.x PERF cache sin/cos table
    qreal startOpposite = sin(alpha) * startHypothenuse;
    qreal startAdjacent = cos(alpha) * startHypothenuse;

    qreal endOpposite = sin(alpha) * endHypothenuse;
    qreal endAdjacent = cos(alpha) * endHypothenuse;

    QPointF p1a(x1 - startAdjacent, y1 - startOpposite);
    QPointF p1b(x1 + startAdjacent, y1 + startOpposite);

    QPointF p2a(x2 - endAdjacent, y2 - endOpposite);
    QPointF p2b(x2 + endAdjacent, y2 + endOpposite);

    QPainterPath painterPath;
    painterPath.moveTo(p1a);
    painterPath.lineTo(p2a);

    painterPath.arcTo(x2 - endHypothenuse, y2 - endHypothenuse, pEndWidth, pEndWidth, (90.0 + pLine.angle()), -180.0);

    //painterPath.lineTo(p2b);
    painterPath.lineTo(p1b);

    painterPath.arcTo(x1 - startHypothenuse, y1 - startHypothenuse, pStartWidth, pStartWidth, -1 * (90.0 - pLine.angle()), -180.0);

    painterPath.closeSubpath();

    return painterPath.toFillPolygon();
}

QPolygonF UBGeometryUtils::lineToPolygon(const QPointF& pStart, const QPointF& pEnd,
        const qreal& pStartWidth, const qreal& pEndWidth)
{

    qreal x1 = pStart.x();
    qreal y1 = pStart.y();

    qreal x2 = pEnd.x();
    qreal y2 = pEnd.y();

    QLineF line(pStart, pEnd);

    qreal alpha = (90.0 - line.angle()) * PI / 180.0;
    qreal hypothenuseStart = pStartWidth / 2;

    qreal hypothenuseEnd = pEndWidth / 2;

    qreal sinAlpha = sin(alpha);
    qreal cosAlpha = cos(alpha);

    // TODO UB 4.x PERF cache sin/cos table
    qreal oppositeStart = sinAlpha * hypothenuseStart;
    qreal adjacentStart = cosAlpha * hypothenuseStart;

    QPointF p1a(x1 - adjacentStart, y1 - oppositeStart);
    QPointF p1b(x1 + adjacentStart, y1 + oppositeStart);

    qreal oppositeEnd = sinAlpha * hypothenuseEnd;
    qreal adjacentEnd = cosAlpha * hypothenuseEnd;

    QPointF p2a(x2 - adjacentEnd, y2 - oppositeEnd);

    QPainterPath painterPath;

    painterPath.moveTo(p1a);
    painterPath.lineTo(p2a);

    painterPath.arcTo(x2 - hypothenuseEnd, y2 - hypothenuseEnd, pEndWidth, pEndWidth, (90.0 + line.angle()), -180.0);

    painterPath.lineTo(p1b);

    painterPath.arcTo(x1 - hypothenuseStart, y1 - hypothenuseStart, pStartWidth, pStartWidth, -1 * (90.0 - line.angle()), -180.0);

    painterPath.closeSubpath();

    return painterPath.toFillPolygon();
}

QPolygonF UBGeometryUtils::arcToPolygon(const QLineF& startRadius, qreal spanAngleInDegrees, qreal width)
{
    qreal startAngleInDegrees = - startRadius.angle();
    if (startAngleInDegrees > 180)
        startAngleInDegrees -= 360;
    else if (startAngleInDegrees < -180)
        startAngleInDegrees += 360;

    qreal radiusLength = startRadius.length();
    qreal angle = 2 * asin(width / (2 * radiusLength)) * 180 / PI;
    bool overlap = qAbs(spanAngleInDegrees) > 360 - angle;
    if (overlap)
        spanAngleInDegrees = spanAngleInDegrees < 0 ? -360 : 360;

    qreal endAngleInDegrees = startAngleInDegrees + spanAngleInDegrees;

    qreal innerRadius = radiusLength - width / 2;
    QRectF innerSquare(
        startRadius.p1().x() - innerRadius,
        startRadius.p1().y() - innerRadius,
        2 * innerRadius,
        2 * innerRadius);
    qreal outerRadius = radiusLength + width / 2;
    QRectF outerSquare(
        startRadius.p1().x() - outerRadius,
        startRadius.p1().y() - outerRadius,
        2 * outerRadius,
        2 * outerRadius);
    QRectF startSquare(
        startRadius.p2().x() - width / 2,
        startRadius.p2().y() - width / 2,
        width,
        width);
    QRectF endSquare(
        startRadius.p1().x() + radiusLength * cos(endAngleInDegrees * PI / 180.0) - width / 2,
        startRadius.p1().y() + radiusLength * sin(endAngleInDegrees * PI / 180.0) - width / 2,
        width,
        width);

    QPainterPath painterPath(
        QPointF(
            startRadius.p1().x() + innerRadius * cos(startAngleInDegrees * PI / 180.0),
            startRadius.p1().y() + innerRadius * sin(startAngleInDegrees * PI / 180.0)));
    startAngleInDegrees = - startAngleInDegrees;
    endAngleInDegrees = - endAngleInDegrees;
    spanAngleInDegrees = - spanAngleInDegrees;

    painterPath.setFillRule(Qt::WindingFill);
    painterPath.arcTo(innerSquare, startAngleInDegrees, spanAngleInDegrees);
    painterPath.arcTo(endSquare, 180.0 + endAngleInDegrees, spanAngleInDegrees > 0 ? -180.0 : 180.0);
    painterPath.arcTo(outerSquare, endAngleInDegrees, - spanAngleInDegrees);
    painterPath.arcTo(startSquare, startAngleInDegrees, spanAngleInDegrees > 0 ? -180.0 : 180.0);
    painterPath.closeSubpath();

    return painterPath.toFillPolygon();
}

/**
 * @brief Build and return a polygon from a list of points (at least 2), and start and end widths.
 *
 * The resulting polygon will pass by all points in the curve; its thickness is calculated at each point
 * of the curve (linearly interpolated between start and end widths) and the segments are joined by
 * (approximately) curved joints.
 *
 * Like with lineToPolygon, the ends are semi-circular.
 */
QPolygonF UBGeometryUtils::curveToPolygon(const QList<QPointF>& points, qreal startWidth, qreal endWidth)
{
    int n_points = points.size();

    if (n_points < 2)
        return QPolygonF();

    if (n_points == 2)
        return lineToPolygon(points[0], points[1], startWidth, endWidth);

    QList<QPair<QPointF, qreal> > pointsAndWidths;
    for (int i(0); i < n_points; ++i) {
        qreal width = startWidth + (qreal(i)/qreal(n_points-1)) * (endWidth - startWidth);

        pointsAndWidths << QPair<QPointF, qreal>(points[i], width);
    }

    return curveToPolygon(pointsAndWidths, true, true);
}

/**
 * @brief Build and return a polygon from a list of points and thicknesses (at least 2)
 *
 * The resulting polygon will pass by all points in the curve; the segments are joined by
 * (approximately) curved joints. The ends of the polygon can be terminated by arcs by passing
 * `true` as the `roundStart` and/or `roundEnd` parameters.
 *
 */
QPolygonF UBGeometryUtils::curveToPolygon(const QList<QPair<QPointF, qreal> >& points, bool roundStart, bool roundEnd)
{
    int n_points = points.size();

    if (n_points == 0)
        return QPolygonF();
    if (n_points == 1)
        return lineToPolygon(points.first().first, points.first().first, points.first().second, points.first().second);

    qreal startWidth = points.first().second;
    qreal endWidth = points.last().second;

    /* The vertices (x's) are calculated based on the stroke's width and angle, and the position of the
       supplied points (o's):

          x----------x--------x

          o          o        o

          x----------x -------x

       The vertices above and below each 'o' point are temporarily stored together, 
       as a pair of points.
     */

    typedef QPair<QPointF, QPointF> pointPair;
    QList<pointPair> newPoints;


    QLineF firstSegment = QLineF(points[0].first, points[1].first);
    QLineF normal = firstSegment.normalVector();
    normal.setLength(startWidth/2.0);
    newPoints << pointPair(normal.p2(), points[0].first - QPointF(normal.dx(), normal.dy()));

    /*
    Calculating the vertices (d1 and d2, below) is a little less trivial for the
    next points: their positions depend on the angle between one segment and the next.

                      d1
         ------------x
                      \
         .a      b .   \
                        \
         --------x       \
               d2 \       \
                   \   .c  \

    Here, points a, b and c are supplied in the `points` list.

    N.B: The drawing isn't quite accurate; we don't do a miter joint but a kind
    of rounded-off joint (the distance between b and d1 is half the width of the stroke)
    */

    for (int i(1); i < n_points-1; ++i) {
        //qreal width = startWidth + (qreal(i)/qreal(n_points-1)) * (endWidth - startWidth);

        QLineF normal = (QLineF(points[i-1].first, points[i+1].first)).normalVector();
        normal.setLength(points[i].second/2.0);
        QPointF d1 = points[i].first + QPointF(normal.dx(), normal.dy());
        QPointF d2 = points[i].first - QPointF(normal.dx(), normal.dy());

        newPoints << pointPair(d1, d2);
    }

    // The last point is similar to the first
    QLineF lastSegment = QLineF(points[n_points-2].first, points[n_points-1].first);
    normal = lastSegment.normalVector();
    normal.setLength(endWidth/2.0);

    QPointF d1 = points.last().first + QPointF(normal.dx(), normal.dy());
    QPointF d2 = points.last().first - QPointF(normal.dx(), normal.dy());

    newPoints << pointPair(d1, d2);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.moveTo(newPoints[0].first);

    for (int i(1); i < n_points; ++i) {
        path.lineTo(newPoints[i].first);
    }

    if (roundEnd)
        path.arcTo(points.last().first.x() - endWidth/2.0, points.last().first.y() - endWidth/2.0, endWidth, endWidth, (90.0 + lastSegment.angle()), -180.0);
    else
        path.lineTo(newPoints.last().second);

    for (int i(n_points-1); i >= 0; --i) {
        path.lineTo(newPoints[i].second);
    }

    if (roundStart)
        path.arcTo(points[0].first.x() - startWidth/2.0, points[0].first.y() - startWidth/2.0, startWidth, startWidth, (firstSegment.angle() - 90.0), -180.0);
    else
        path.lineTo(newPoints[0].first);


    //path.closeSubpath();

    return path.toFillPolygon();
}

QPointF UBGeometryUtils::pointConstrainedInRect(QPointF point, QRectF rect)
{
    return QPointF(qMax(rect.x(), qMin(rect.x() + rect.width(), point.x())), qMax(rect.y(), qMin(rect.y() + rect.height(), point.y())));
}


QPoint UBGeometryUtils::pointConstrainedInRect(QPoint point, QRect rect)
{
    return QPoint(qMax(rect.x(), qMin(rect.x() + rect.width(), point.x())), qMax(rect.y(), qMin(rect.y() + rect.height(), point.y())));
}


QRectF UBGeometryUtils::lineToInnerRect(const QLineF& pLine, const qreal& pWidth)
{
    qreal centerX = (pLine.x1() + pLine.x2()) / 2;
    qreal centerY = (pLine.y1() + pLine.y2()) / 2;

    // Please put a fucking comment here
    qreal side = sqrt((pWidth * pWidth) / 2);
    qreal halfSide = side / 2;

    return QRectF(centerX - halfSide, centerY - halfSide, side, side);
}


void UBGeometryUtils::crashPointList(QVector<QPointF> &points)
{
   // QVector<QPointF> result(points);
    int position = 1;

    while(position < points.size())
    {
        if (points.at(position) == points.at(position - 1))
        {
            points.remove(position);
        }
        else
        {
            ++position;
        }
    }
}

/**
 * @brief Return the angle in degrees between three points
 */
qreal UBGeometryUtils::angle(const QPointF& p1, const QPointF& p2, const QPointF& p3)
{
    // Angle between three points, using the law of cosines:
    // The angle at B equals arccos((a^2-b^2+c^2)/(2*a*c)), where a, b and c are the sides of the triangle
    // opposite A, B and C, respectively

    qreal a, b, c, beta;
    a = qSqrt(qPow(p2.x() - p3.x(), 2) + qPow(p2.y() - p3.y(), 2));
    b = qSqrt(qPow(p1.x() - p3.x(), 2) + qPow(p1.y() - p3.y(), 2));
    c = qSqrt(qPow(p1.x() - p2.x(), 2) + qPow(p1.y() - p2.y(), 2));

    if (a == 0 || c == 0)
        beta = 3.14159;
    else
        beta = qAcos(std::max(-1.0, std::min(1.0, (a*a - b*b + c*c)/(2*a*c))));


    return 180.* beta/3.14159;
}



/**
 * @brief Calculate a quadratic Bézier curve and return it in the form of a list of points
 * @param p0 The start point of the curve
 * @param p1 The control point of the curve
 * @param p2 The end point of the curve
 * @param nPoints The number of points by which to approximate the curve, i.e. the length of the returned list
 * @return A list of points that can be used to draw the curve.
 */
QList<QPointF> UBGeometryUtils::quadraticBezier(const QPointF& p0, const QPointF& p1, const QPointF& p2, unsigned int nPoints)
{
    QPainterPath path(p0);
    path.quadTo(p1, p2);

    QList<QPointF> points;

    if (nPoints <= 1)
        return points;

    for (unsigned int i(0); i <= nPoints; ++i) {
        qreal percent = qreal(i)/qreal(nPoints);
        points << path.pointAtPercent(percent);
    }

    return points;
}
