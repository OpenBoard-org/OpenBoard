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
    bool overlap = abs(spanAngleInDegrees) > 360 - angle;
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

    if (overlap)
    {
        painterPath.addEllipse(outerSquare);
        QPainterPath innerPainterPath;
        innerPainterPath.addEllipse(innerSquare);
        painterPath = painterPath.subtracted(innerPainterPath);
    }
    else
    {
        painterPath.arcTo(innerSquare, startAngleInDegrees, spanAngleInDegrees);
        painterPath.arcTo(endSquare, 180.0 + endAngleInDegrees, spanAngleInDegrees > 0 ? -180.0 : 180.0);
        painterPath.arcTo(outerSquare, endAngleInDegrees, - spanAngleInDegrees);
        painterPath.arcTo(startSquare, startAngleInDegrees, spanAngleInDegrees > 0 ? -180.0 : 180.0);
        painterPath.closeSubpath();
    }

    return painterPath.toFillPolygon();
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
