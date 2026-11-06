/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBShapeRecognizer.h"

#include <QLineF>
#include <QTransform>
#include <QtMath>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>

namespace
{
    // All tolerances are relative, so that recognition does not depend on the zoom level.

    /// number of points the stroke is resampled to
    constexpr int resampleCount = 128;

    /// points closer than this fraction of the bounding box diagonal to their predecessor are ignored (jitter)
    constexpr qreal minPointSpacing = 0.015;

    /// points at the ends of the stroke closer than this fraction of the bounding box diagonal to the end point are ignored (dwell)
    constexpr qreal dwellRadius = 0.03;

    /// a line may deviate from the chord by at most this fraction of its length
    constexpr qreal lineMaxDeviation = 0.07;

    /// the path length of a line may exceed its chord length by at most this ratio
    constexpr qreal lineMaxPathRatio = 1.15;

    /// a stroke is closed if it returns to its start point within this fraction of the bounding box diagonal
    constexpr qreal closeTolerance = 0.2;

    /// the closing point is searched in this last fraction of the stroke, to allow for an overshoot
    constexpr qreal closingSearchStart = 0.7;

    /// the smaller principal extent of a closed shape must be at least this fraction of the larger one
    constexpr qreal minAspectRatio = 0.1;

    /// Douglas-Peucker tolerance as a fraction of the bounding box diagonal
    constexpr qreal cornerTolerance = 0.04;

    /// a corner must turn by at least this angle in degrees
    constexpr qreal minTurnAngle = 30.;

    /// edges shorter than this fraction of the perimeter are considered rounded corners
    constexpr qreal minEdgeRatio = 0.08;

    /// two adjacent turns at least this sharp mark a real (short) edge, not a rounded corner
    constexpr qreal sharpTurnAngle = 60.;

    /// mean distance of the stroke to a polygon, as a fraction of the bounding box diagonal
    constexpr qreal polygonMaxError = 0.03;

    /// mean distance of the stroke to an ellipse, as a fraction of the bounding box diagonal
    constexpr qreal ellipseMaxError = 0.03;

    /// maximum deviation from a right angle in degrees for a quadrilateral to become a rectangle
    constexpr qreal rectangleAngleTolerance = 20.;

    /// maximum relative difference of width and height for a rectangle to become a square
    constexpr qreal squareTolerance = 0.12;

    /// maximum relative difference of the radii for an ellipse to become a circle
    constexpr qreal circleTolerance = 0.15;

    struct Line
    {
        QPointF point;
        QPointF direction;
    };

    qreal length(const QPointF& v)
    {
        return std::hypot(v.x(), v.y());
    }

    qreal cross(const QPointF& a, const QPointF& b)
    {
        return a.x() * b.y() - a.y() * b.x();
    }

    qreal pathLength(const QList<QPointF>& points)
    {
        qreal result = 0;

        for (int i = 1; i < points.size(); ++i)
        {
            result += length(points.at(i) - points.at(i - 1));
        }

        return result;
    }

    QList<QPointF> removeInvalidAndDuplicates(const QList<QPointF>& points)
    {
        QList<QPointF> result;

        for (const QPointF& point : points)
        {
            if (!std::isfinite(point.x()) || !std::isfinite(point.y()))
            {
                continue;
            }

            if (result.isEmpty() || length(point - result.last()) > 1e-9)
            {
                result << point;
            }
        }

        return result;
    }

    /// resample the polyline to count points evenly spaced along its length
    QList<QPointF> resample(const QList<QPointF>& points, int count)
    {
        const qreal total = pathLength(points);
        const qreal interval = total / (count - 1);
        QList<QPointF> result{points.first()};

        qreal accumulated = 0;
        QPointF previous = points.first();
        int nextIndex = 1;

        while (nextIndex < points.size() && result.size() < count - 1)
        {
            const QPointF current = points.at(nextIndex);
            const qreal segment = length(current - previous);

            if (accumulated + segment >= interval && segment > 0)
            {
                const qreal fraction = (interval - accumulated) / segment;
                const QPointF point = previous + fraction * (current - previous);
                result << point;
                previous = point;
                accumulated = 0;
            }
            else
            {
                accumulated += segment;
                previous = current;
                ++nextIndex;
            }
        }

        result << points.last();
        return result;
    }

    qreal distanceToSegment(const QPointF& point, const QPointF& start, const QPointF& end)
    {
        const QPointF segment = end - start;
        const qreal squaredLength = QPointF::dotProduct(segment, segment);

        if (squaredLength <= 0)
        {
            return length(point - start);
        }

        // position of the orthogonal projection of the point on the segment, from 0 (start) to 1 (end)
        const qreal projection = std::clamp(QPointF::dotProduct(point - start, segment) / squaredLength, 0., 1.);
        return length(point - (start + projection * segment));
    }

    qreal distanceToPolygon(const QPointF& point, const QPolygonF& polygon)
    {
        qreal result = std::numeric_limits<qreal>::max();

        for (int i = 0; i < polygon.size(); ++i)
        {
            result = std::min(result, distanceToSegment(point, polygon.at(i), polygon.at((i + 1) % polygon.size())));
        }

        return result;
    }

    /// remove points clustering around the end point, as when the pen is held still
    QList<QPointF> trimDwell(QList<QPointF> points, qreal radius)
    {
        const QPointF end = points.last();

        while (points.size() > 2 && length(points.at(points.size() - 2) - end) < radius)
        {
            points.removeAt(points.size() - 2);
        }

        return points;
    }

    /// whether the segments ab and cd cross each other
    bool segmentsIntersect(const QPointF& a, const QPointF& b, const QPointF& c, const QPointF& d)
    {
        QPointF intersection;
        return QLineF(a, b).intersects(QLineF(c, d), &intersection) == QLineF::BoundedIntersection;
    }

    /// turning angle in degrees at b when going from a over b to c, 0 if straight, 180 if reversing
    qreal turnAngle(const QPointF& a, const QPointF& b, const QPointF& c)
    {
        const qreal angle = QLineF(a, b).angleTo(QLineF(b, c));
        return angle > 180. ? 360. - angle : angle;
    }

    void douglasPeucker(const QList<QPointF>& points, int first, int last, qreal epsilon, QList<int>& keep)
    {
        qreal maxDistance = 0;
        int index = -1;

        for (int i = first + 1; i < last; ++i)
        {
            const qreal distance = distanceToSegment(points.at(i), points.at(first), points.at(last));

            if (distance > maxDistance)
            {
                maxDistance = distance;
                index = i;
            }
        }

        if (index >= 0 && maxDistance > epsilon)
        {
            douglasPeucker(points, first, index, epsilon, keep);
            keep << index;
            douglasPeucker(points, index, last, epsilon, keep);
        }
    }

    QPointF centroid(const QList<QPointF>& points)
    {
        QPointF sum;

        for (const QPointF& point : points)
        {
            sum += point;
        }

        return sum / points.size();
    }

    /// principal axis angle in radians and variances along the major and minor axes
    struct Principal
    {
        QPointF center;
        qreal angle{0};
        qreal majorVariance{0};
        qreal minorVariance{0};
    };

    Principal principalAxes(const QList<QPointF>& points)
    {
        Principal result;
        result.center = centroid(points);

        // covariance matrix [[sxx, sxy], [sxy, syy]]
        qreal sxx = 0;
        qreal syy = 0;
        qreal sxy = 0;

        for (const QPointF& point : points)
        {
            const QPointF offset = point - result.center;
            sxx += offset.x() * offset.x();
            syy += offset.y() * offset.y();
            sxy += offset.x() * offset.y();
        }

        sxx /= points.size();
        syy /= points.size();
        sxy /= points.size();

        result.angle = 0.5 * std::atan2(2 * sxy, sxx - syy);

        const qreal mean = (sxx + syy) / 2;
        const qreal delta = std::sqrt(std::max(0., (sxx - syy) * (sxx - syy) / 4 + sxy * sxy));
        result.majorVariance = mean + delta;
        result.minorVariance = std::max(0., mean - delta);

        return result;
    }

    Line fitLine(const QList<QPointF>& points)
    {
        const Principal principal = principalAxes(points);
        return {principal.center, QPointF(std::cos(principal.angle), std::sin(principal.angle))};
    }

    std::optional<QPointF> intersect(const Line& first, const Line& second)
    {
        const qreal denominator = cross(first.direction, second.direction);

        if (std::abs(denominator) < 1e-6)
        {
            return std::nullopt; // parallel lines
        }

        const qreal distanceAlongFirst = cross(second.point - first.point, second.direction) / denominator;
        return first.point + distanceAlongFirst * first.direction;
    }

    /// snap an angle in degrees to the closest multiple of 90° if it is within tolerance
    qreal snapToAxis(qreal angle)
    {
        const qreal snapped = std::round(angle / 90.) * 90.;
        return std::abs(angle - snapped) <= UBShapeRecognizer::axisSnapTolerance ? snapped : angle;
    }

    /// points from index from to index to (inclusive), wrapping around the end of the closed loop
    QList<QPointF> loopSection(const QList<QPointF>& loop, int from, int to)
    {
        QList<QPointF> result;
        const int n = loop.size();
        const int count = ((to - from + n) % n) + 1;

        for (int i = 0; i < count; ++i)
        {
            result << loop.at((from + i) % n);
        }

        return result;
    }

    UBShapeRecognizer::Result recognizeLine(const QList<QPointF>& points, qreal strokeLength)
    {
        const QPointF start = points.first();
        const QPointF end = points.last();
        const qreal chord = length(end - start);

        if (chord <= 0 || strokeLength > lineMaxPathRatio * chord)
        {
            return {};
        }

        for (const QPointF& point : points)
        {
            if (distanceToSegment(point, start, end) > lineMaxDeviation * chord)
            {
                return {};
            }
        }

        QLineF line(start, end);
        line.setAngle(snapToAxis(line.angle()));

        UBShapeRecognizer::Result result;
        result.shape = UBShapeRecognizer::Shape::Line;
        result.vertices << line.p1() << line.p2();
        return result;
    }

    /**
     * Find the corners of a closed loop. Returns the indices of the corners in the loop.
     */
    QList<int> findCorners(const QList<QPointF>& loop, qreal diagonal)
    {
        QList<QPointF> closedPath = loop;
        closedPath << loop.first();

        QList<int> corners{0};
        douglasPeucker(closedPath, 0, closedPath.size() - 1, cornerTolerance * diagonal, corners);

        // remove straight vertices and collapse short edges which are rounded corners
        bool changed = true;

        while (changed && corners.size() > 2)
        {
            changed = false;
            const int n = corners.size();

            auto vertex = [&](int i) { return loop.at(corners.at((i + n) % n)); };
            auto turn = [&](int i) { return turnAngle(vertex(i - 1), vertex(i), vertex(i + 1)); };

            int straightest = 0;

            for (int i = 1; i < n; ++i)
            {
                if (turn(i) < turn(straightest))
                {
                    straightest = i;
                }
            }

            if (turn(straightest) < minTurnAngle)
            {
                corners.removeAt(straightest);
                changed = true;
                continue;
            }

            qreal perimeter = 0;
            int shortest = 0;

            for (int i = 0; i < n; ++i)
            {
                const qreal edge = length(vertex(i + 1) - vertex(i));
                perimeter += edge;

                if (edge < length(vertex(shortest + 1) - vertex(shortest)))
                {
                    shortest = i;
                }
            }

            const qreal shortestLength = length(vertex(shortest + 1) - vertex(shortest));
            const qreal turnStart = turn(shortest);
            const qreal turnEnd = turn(shortest + 1);

            if (shortestLength < minEdgeRatio * perimeter && (turnStart < sharpTurnAngle || turnEnd < sharpTurnAngle))
            {
                corners.removeAt(turnStart < turnEnd ? shortest : (shortest + 1) % n);
                changed = true;
            }
        }

        return corners;
    }

    /**
     * Fit a straight line to each side of the loop and intersect adjacent lines
     * to obtain precise corners, which are not affected by rounded drawing.
     */
    QPolygonF refineCorners(const QList<QPointF>& loop, const QList<int>& corners, qreal diagonal, const QPointF& center)
    {
        const int n = corners.size();
        QList<Line> sides;

        for (int i = 0; i < n; ++i)
        {
            QList<QPointF> section = loopSection(loop, corners.at(i), corners.at((i + 1) % n));

            // ignore the ends of each side, where corners may be rounded
            const int trim = section.size() / 5;

            if (section.size() - 2 * trim >= 2)
            {
                section = section.mid(trim, section.size() - 2 * trim);
            }

            sides << fitLine(section);
        }

        QPolygonF result;

        for (int i = 0; i < n; ++i)
        {
            const std::optional<QPointF> corner = intersect(sides.at((i + n - 1) % n), sides.at(i));

            if (corner && length(*corner - center) <= diagonal)
            {
                result << *corner;
            }
            else
            {
                result << loop.at(corners.at(i));
            }
        }

        return result;
    }

    /// Make a rectangle or square from a quadrilateral if its angles are nearly right angles.
    std::optional<QPolygonF> regularizeRectangle(const QPolygonF& quad)
    {
        qreal sumCos = 0;
        qreal sumSin = 0;

        for (int i = 0; i < 4; ++i)
        {
            const QPointF previous = quad.at((i + 3) % 4);
            const QPointF corner = quad.at(i);
            const QPointF next = quad.at((i + 1) % 4);

            const qreal interior = 180. - turnAngle(previous, corner, next);

            if (std::abs(interior - 90.) > rectangleAngleTolerance)
            {
                return std::nullopt;
            }

            // average edge direction modulo 90°, weighted by edge length
            const QPointF edge = next - corner;
            const qreal angle = 4 * std::atan2(edge.y(), edge.x());
            sumCos += length(edge) * std::cos(angle);
            sumSin += length(edge) * std::sin(angle);
        }

        const qreal rotation = snapToAxis(qRadiansToDegrees(std::atan2(sumSin, sumCos) / 4));

        QPointF center;

        for (const QPointF& point : quad)
        {
            center += point;
        }

        center /= 4;

        QTransform toLocal;
        toLocal.rotate(-rotation);
        toLocal.translate(-center.x(), -center.y());

        QList<qreal> us;
        QList<qreal> vs;

        for (const QPointF& point : quad)
        {
            const QPointF local = toLocal.map(point);
            us << local.x();
            vs << local.y();
        }

        std::sort(us.begin(), us.end());
        std::sort(vs.begin(), vs.end());

        qreal left = (us.at(0) + us.at(1)) / 2;
        qreal right = (us.at(2) + us.at(3)) / 2;
        qreal top = (vs.at(0) + vs.at(1)) / 2;
        qreal bottom = (vs.at(2) + vs.at(3)) / 2;

        const qreal width = right - left;
        const qreal height = bottom - top;

        if (std::abs(width - height) <= squareTolerance * std::max(width, height))
        {
            const qreal side = (width + height) / 2;
            const QPointF middle((left + right) / 2, (top + bottom) / 2);
            left = middle.x() - side / 2;
            right = middle.x() + side / 2;
            top = middle.y() - side / 2;
            bottom = middle.y() + side / 2;
        }

        const QTransform toScene = toLocal.inverted();
        QPolygonF result;
        result << toScene.map(QPointF(left, top)) << toScene.map(QPointF(right, top))
               << toScene.map(QPointF(right, bottom)) << toScene.map(QPointF(left, bottom));
        return result;
    }

    struct EllipseFit
    {
        QPointF center;
        qreal radiusX{0};
        qreal radiusY{0};
        qreal angle{0};
        qreal error{std::numeric_limits<qreal>::max()};
    };

    /// solve the linear system given as augmented matrix [A | b] by Gaussian elimination with partial pivoting
    template<int N>
    std::optional<std::array<qreal, N>> solve(std::array<std::array<qreal, N + 1>, N> augmented)
    {
        for (int column = 0; column < N; ++column)
        {
            int pivot = column;

            for (int row = column + 1; row < N; ++row)
            {
                if (std::abs(augmented[row][column]) > std::abs(augmented[pivot][column]))
                {
                    pivot = row;
                }
            }

            if (std::abs(augmented[pivot][column]) < 1e-12)
            {
                return std::nullopt;
            }

            std::swap(augmented[column], augmented[pivot]);

            for (int row = column + 1; row < N; ++row)
            {
                const qreal factor = augmented[row][column] / augmented[column][column];

                for (int k = column; k <= N; ++k)
                {
                    augmented[row][k] -= factor * augmented[column][k];
                }
            }
        }

        std::array<qreal, N> solution{};

        for (int row = N - 1; row >= 0; --row)
        {
            qreal sum = augmented[row][N];

            for (int k = row + 1; k < N; ++k)
            {
                sum -= augmented[row][k] * solution[k];
            }

            solution[row] = sum / augmented[row][row];
        }

        return solution;
    }

    /**
     * Fit a conic a*x^2 + b*x*y + c*y^2 + d*x + e*y = 1 by least squares and
     * return it if it is an ellipse. Center and orientation are free, so that
     * strokes which are not completely closed are fitted correctly.
     */
    std::optional<EllipseFit> fitEllipse(const QList<QPointF>& loop)
    {
        // normalize coordinates for numerical stability
        const QPointF origin = centroid(loop);
        qreal scale = 0;

        for (const QPointF& point : loop)
        {
            scale += QPointF::dotProduct(point - origin, point - origin);
        }

        scale = std::sqrt(scale / loop.size());

        if (scale <= 0)
        {
            return std::nullopt;
        }

        // normal equations of the least squares problem, as an augmented 5x6 matrix
        std::array<std::array<qreal, 6>, 5> normal{};

        for (const QPointF& point : loop)
        {
            const QPointF normalized = (point - origin) / scale;
            const qreal x = normalized.x();
            const qreal y = normalized.y();
            const std::array<qreal, 5> row{x * x, x * y, y * y, x, y};

            for (int i = 0; i < 5; ++i)
            {
                for (int j = 0; j < 5; ++j)
                {
                    normal[i][j] += row[i] * row[j];
                }

                normal[i][5] += row[i];
            }
        }

        const std::optional<std::array<qreal, 5>> conic = solve<5>(normal);

        if (!conic)
        {
            return std::nullopt;
        }

        // coefficients of a*x^2 + b*x*y + c*y^2 + d*x + e*y = 1
        const auto [a, b, c, d, e] = *conic;

        // the center is where the gradient of the conic vanishes
        const qreal determinant = 4 * a * c - b * b;

        if (determinant <= 0)
        {
            return std::nullopt; // not an ellipse
        }

        const qreal centerX = (b * e - 2 * c * d) / determinant;
        const qreal centerY = (b * d - 2 * a * e) / determinant;

        // in coordinates relative to the center: a*x^2 + b*x*y + c*y^2 = level
        const qreal level = 1 - (d * centerX + e * centerY) / 2;

        if (level <= 0)
        {
            return std::nullopt;
        }

        // eigen decomposition of the quadratic form [[formXX, formXY], [formXY, formYY]]
        const qreal formXX = a / level;
        const qreal formXY = b / 2 / level;
        const qreal formYY = c / level;
        const qreal mean = (formXX + formYY) / 2;
        const qreal delta = std::sqrt((formXX - formYY) * (formXX - formYY) / 4 + formXY * formXY);
        const qreal majorEigenvalue = mean - delta; // the smaller eigenvalue belongs to the major axis
        const qreal minorEigenvalue = mean + delta;

        if (majorEigenvalue <= 0)
        {
            return std::nullopt;
        }

        const qreal majorAxisAngle = 0.5 * std::atan2(2 * formXY, formXX - formYY) + M_PI_2;

        EllipseFit fit;
        fit.center = origin + scale * QPointF(centerX, centerY);
        fit.radiusX = scale / std::sqrt(majorEigenvalue);
        fit.radiusY = scale / std::sqrt(minorEigenvalue);
        fit.angle = qRadiansToDegrees(majorAxisAngle);

        // mean distance of the stroke to the ellipse, measured along the ray from the center
        const qreal cosAngle = std::cos(majorAxisAngle);
        const qreal sinAngle = std::sin(majorAxisAngle);
        qreal error = 0;

        for (const QPointF& point : loop)
        {
            const QPointF offset = point - fit.center;

            // position in the frame of the ellipse, scaled so that the ellipse is the unit circle
            const qreal alongMajor = (offset.x() * cosAngle + offset.y() * sinAngle) / fit.radiusX;
            const qreal alongMinor = (-offset.x() * sinAngle + offset.y() * cosAngle) / fit.radiusY;
            const qreal normalizedRadius = std::hypot(alongMajor, alongMinor);
            const qreal distanceFromCenter = length(offset);

            error += normalizedRadius > 0 ? distanceFromCenter * std::abs(1 - 1 / normalizedRadius) : fit.radiusX;
        }

        fit.error = error / loop.size();
        return fit;
    }

    QPolygonF sampleEllipse(const QPointF& center, qreal radiusX, qreal radiusY, qreal angle)
    {
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(angle);

        QPolygonF result;

        for (int i = 0; i < UBShapeRecognizer::ellipseSegments; ++i)
        {
            const qreal parameter = 2 * M_PI * i / UBShapeRecognizer::ellipseSegments;
            result << transform.map(QPointF(radiusX * std::cos(parameter), radiusY * std::sin(parameter)));
        }

        return result;
    }

    UBShapeRecognizer::Result recognizeClosedShape(const QList<QPointF>& points, qreal diagonal)
    {
        // find the point where the stroke comes back to its start, ignoring any overshoot
        const int n = points.size();
        int closingIndex = -1;
        qreal closingDistance = std::numeric_limits<qreal>::max();

        for (int i = int(n * closingSearchStart); i < n; ++i)
        {
            const qreal distance = length(points.at(i) - points.first());

            if (distance < closingDistance)
            {
                closingDistance = distance;
                closingIndex = i;
            }
        }

        if (closingIndex < 0 || closingDistance > closeTolerance * diagonal)
        {
            return {};
        }

        QList<QPointF> loop = points.mid(0, closingIndex + 1);

        if (length(loop.last() - loop.first()) < 1e-9)
        {
            loop.removeLast();
        }

        if (loop.size() < 8)
        {
            return {};
        }

        const Principal principal = principalAxes(loop);

        if (principal.majorVariance <= 0 || std::sqrt(principal.minorVariance / principal.majorVariance) < minAspectRatio)
        {
            return {};
        }

        // start the loop at the point farthest from the center, which is a corner for polygons
        int farthest = 0;

        for (int i = 1; i < loop.size(); ++i)
        {
            if (length(loop.at(i) - principal.center) > length(loop.at(farthest) - principal.center))
            {
                farthest = i;
            }
        }

        std::rotate(loop.begin(), loop.begin() + farthest, loop.end());

        // polygon candidate
        std::optional<QPolygonF> polygon;
        qreal polygonError = std::numeric_limits<qreal>::max();
        const QList<int> corners = findCorners(loop, diagonal);

        if (corners.size() >= 3)
        {
            polygon = refineCorners(loop, corners, diagonal, principal.center);
            polygonError = 0;

            for (const QPointF& point : loop)
            {
                polygonError += distanceToPolygon(point, *polygon);
            }

            polygonError /= loop.size();
        }

        // ellipse candidate
        const std::optional<EllipseFit> ellipse = fitEllipse(loop);
        const qreal ellipseError = ellipse ? ellipse->error : std::numeric_limits<qreal>::max();

        UBShapeRecognizer::Result result;
        result.closed = true;

        if (polygon && polygonError <= polygonMaxError * diagonal && polygonError <= ellipseError)
        {
            if (polygon->size() == 3)
            {
                result.shape = UBShapeRecognizer::Shape::Triangle;
                result.vertices = *polygon;
            }
            else if (polygon->size() == 4)
            {
                if (segmentsIntersect(polygon->at(0), polygon->at(1), polygon->at(2), polygon->at(3))
                    || segmentsIntersect(polygon->at(1), polygon->at(2), polygon->at(3), polygon->at(0)))
                {
                    return {}; // self-intersecting "bow tie"
                }

                if (const std::optional<QPolygonF> rectangle = regularizeRectangle(*polygon))
                {
                    result.shape = UBShapeRecognizer::Shape::Rectangle;
                    result.vertices = *rectangle;
                }
                else
                {
                    result.shape = UBShapeRecognizer::Shape::Quadrilateral;
                    result.vertices = *polygon;
                }
            }
            else
            {
                // polygons with more corners are not supported, and must not become ellipses
                return {};
            }

            return result;
        }

        if (ellipse && ellipseError <= ellipseMaxError * diagonal)
        {
            result.shape = UBShapeRecognizer::Shape::Ellipse;
            result.center = ellipse->center;

            const qreal major = std::max(ellipse->radiusX, ellipse->radiusY);
            const qreal minor = std::min(ellipse->radiusX, ellipse->radiusY);

            if (major - minor <= circleTolerance * major)
            {
                const qreal radius = (major + minor) / 2;
                result.radiusX = radius;
                result.radiusY = radius;
                result.angle = 0;
            }
            else
            {
                // normalize the angle to ]-90°, 90°] and snap the axes
                qreal angle = std::remainder(ellipse->angle, 180.);
                angle = angle <= -90. ? angle + 180. : angle;
                result.angle = snapToAxis(angle);
                result.radiusX = ellipse->radiusX;
                result.radiusY = ellipse->radiusY;
            }

            result.vertices = sampleEllipse(result.center, result.radiusX, result.radiusY, result.angle);
            return result;
        }

        return {};
    }
}

bool UBShapeRecognizer::Result::isSquare() const
{
    if (shape != Shape::Rectangle || vertices.size() != 4)
    {
        return false;
    }

    const qreal width = length(vertices.at(1) - vertices.at(0));
    const qreal height = length(vertices.at(2) - vertices.at(1));
    return std::abs(width - height) <= 1e-6 * std::max(width, height);
}

bool UBShapeRecognizer::Result::isCircle() const
{
    return shape == Shape::Ellipse && qFuzzyCompare(radiusX, radiusY);
}

QList<QPointF> UBShapeRecognizer::Result::outline() const
{
    QList<QPointF> result;

    for (const QPointF& vertex : vertices)
    {
        result << vertex;
    }

    if (closed && !result.isEmpty())
    {
        result << result.first();
    }

    return result;
}

UBShapeRecognizer::Result UBShapeRecognizer::recognize(const QList<QPointF>& points)
{
    const QList<QPointF> cleaned = removeInvalidAndDuplicates(points);

    if (cleaned.size() < 2)
    {
        return {};
    }

    QPolygonF polygon;

    for (const QPointF& point : cleaned)
    {
        polygon << point;
    }

    const QRectF bounds = polygon.boundingRect();
    const qreal diagonal = std::hypot(bounds.width(), bounds.height());

    if (diagonal <= 0)
    {
        return {};
    }

    // ignore jitter, especially while the pen is held still at the start or end of the stroke
    QList<QPointF> trimmed = trimDwell(cleaned, dwellRadius * diagonal);
    std::reverse(trimmed.begin(), trimmed.end());
    trimmed = trimDwell(trimmed, dwellRadius * diagonal);
    std::reverse(trimmed.begin(), trimmed.end());

    QList<QPointF> filtered{trimmed.first()};

    for (const QPointF& point : trimmed)
    {
        if (length(point - filtered.last()) >= minPointSpacing * diagonal)
        {
            filtered << point;
        }
    }

    // keep the exact end point
    if (filtered.size() > 1)
    {
        filtered.last() = trimmed.last();
    }
    else
    {
        filtered << trimmed.last();
    }

    if (filtered.size() < 2)
    {
        return {};
    }

    const QList<QPointF> resampled = resample(filtered, resampleCount);

    Result line = recognizeLine(resampled, pathLength(resampled));

    if (line.isValid())
    {
        return line;
    }

    return recognizeClosedShape(resampled, diagonal);
}
