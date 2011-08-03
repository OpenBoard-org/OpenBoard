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

#ifndef UBGEOMETRYUTILS_H_
#define UBGEOMETRYUTILS_H_

#include <QtGui>

class UBGeometryUtils
{
    private:
        UBGeometryUtils();
        virtual ~UBGeometryUtils();

    public:
        static QPolygonF lineToPolygon(const QLineF& pLine, const qreal& pWidth);
        static QRectF lineToInnerRect(const QLineF& pLine, const qreal& pWidth);

        static QPolygonF arcToPolygon(const QLineF& startRadius, qreal spanAngle, qreal width);

        static QPolygonF lineToPolygon(const QPointF& pStart, const QPointF& pEnd,
                const qreal& pStartWidth, const qreal& pEndWidth);

        static QPointF pointConstrainedInRect(QPointF point, QRectF rect);
        static QPoint pointConstrainedInRect(QPoint point, QRect rect);

        static QVector<QPointF> crashPointList(const QVector<QPointF> points);
};

#endif /* UBGEOMETRYUTILS_H_ */
