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


#include <QtTest>

#include <QLineF>
#include <QRandomGenerator>
#include <QTransform>

#include <cmath>
#include <limits>

#include "frameworks/UBShapeRecognizer.h"

using Shape = UBShapeRecognizer::Shape;
using Stroke = QList<QPointF>;

Q_DECLARE_METATYPE(Shape)

namespace
{
    // ---------------------------------------------------------------------
    // Stroke generators, simulating what a person draws with a stylus
    // ---------------------------------------------------------------------

    /// densely sample the polyline through the given points
    Stroke densify(const Stroke& corners, qreal spacing = 2.)
    {
        Stroke result;

        for (int i = 0; i + 1 < corners.size(); ++i)
        {
            const QLineF segment(corners.at(i), corners.at(i + 1));
            const int steps = std::max(1, int(std::ceil(segment.length() / spacing)));

            for (int s = 0; s < steps; ++s)
            {
                result << segment.pointAt(qreal(s) / steps);
            }
        }

        result << corners.last();
        return result;
    }

    /// moving average over a window, which rounds corners like a hurried hand does
    Stroke smooth(const Stroke& points, int window)
    {
        if (window <= 1)
        {
            return points;
        }

        Stroke result;

        for (int i = 0; i < points.size(); ++i)
        {
            QPointF sum;
            int count = 0;

            // symmetric window, shrinking at the ends so that the end points stay in place
            const int half = std::min({window / 2, i, int(points.size()) - 1 - i});

            for (int j = i - half; j <= i + half; ++j)
            {
                sum += points.at(j);
                ++count;
            }

            result << sum / count;
        }

        return result;
    }

    /**
     * Add a smooth, low frequency wobble perpendicular to the stroke (hand tremor)
     * plus some high frequency jitter (sensor noise). Amplitudes are absolute.
     */
    Stroke wobble(const Stroke& points, qreal tremor, qreal jitter, quint32 seed)
    {
        QRandomGenerator random(seed);
        const qreal f1 = 1 + random.bounded(3.);
        const qreal f2 = 3 + random.bounded(5.);
        const qreal p1 = random.bounded(2 * M_PI);
        const qreal p2 = random.bounded(2 * M_PI);

        Stroke result;
        const int n = points.size();

        for (int i = 0; i < n; ++i)
        {
            const QPointF previous = points.at(std::max(0, i - 1));
            const QPointF next = points.at(std::min(n - 1, i + 1));
            QPointF normal(-(next - previous).y(), (next - previous).x());
            const qreal len = std::hypot(normal.x(), normal.y());
            normal = len > 0 ? normal / len : QPointF();

            const qreal t = qreal(i) / std::max(1, n - 1);
            const qreal offset = tremor * (0.7 * std::sin(2 * M_PI * f1 * t + p1) + 0.3 * std::sin(2 * M_PI * f2 * t + p2));
            const QPointF noise((random.generateDouble() - 0.5) * 2 * jitter, (random.generateDouble() - 0.5) * 2 * jitter);

            result << points.at(i) + offset * normal + noise;
        }

        return result;
    }

    /// add points around the last one, simulating a pen held still on the board
    Stroke hold(const Stroke& points, int count = 30, qreal jitter = 0.8, quint32 seed = 42)
    {
        QRandomGenerator random(seed);
        Stroke result = points;
        const QPointF last = points.last();

        for (int i = 0; i < count; ++i)
        {
            result << last + QPointF((random.generateDouble() - 0.5) * 2 * jitter, (random.generateDouble() - 0.5) * 2 * jitter);
        }

        return result;
    }

    Stroke line(const QPointF& from, const QPointF& to)
    {
        return densify({from, to});
    }

    /**
     * A closed polygon starting at a fraction of the first edge. A positive closing
     * value overshoots past the start point, a negative value leaves a gap, both
     * as a fraction of the perimeter.
     */
    Stroke polygon(QList<QPointF> vertices, qreal startFraction = 0., qreal closing = 0., bool reverse = false)
    {
        if (reverse)
        {
            std::reverse(vertices.begin(), vertices.end());
        }

        Stroke loop = vertices;
        loop << vertices.first();
        Stroke dense = densify(loop);
        dense.removeLast();

        const int n = dense.size();
        const int start = int(startFraction * n / vertices.size()) % n;
        const int count = n + int(closing * n);

        Stroke result;

        for (int i = 0; i <= count; ++i)
        {
            result << dense.at((start + i) % n);
        }

        return result;
    }

    Stroke rectangle(const QRectF& rect, qreal rotation = 0., qreal startFraction = 0., qreal closing = 0., bool reverse = false)
    {
        QTransform transform;
        transform.translate(rect.center().x(), rect.center().y());
        transform.rotate(rotation);
        transform.translate(-rect.center().x(), -rect.center().y());

        return polygon({transform.map(rect.topLeft()), transform.map(rect.topRight()),
                        transform.map(rect.bottomRight()), transform.map(rect.bottomLeft())},
                       startFraction, closing, reverse);
    }

    /// an ellipse drawn from startAngle over sweep degrees
    Stroke ellipse(const QPointF& center, qreal rx, qreal ry, qreal rotation = 0., qreal startAngle = 0., qreal sweep = 360.)
    {
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(rotation);

        const qreal circumference = M_PI * (rx + ry);
        const int count = std::max(16, int(circumference * std::abs(sweep) / 360. / 2.));
        Stroke result;

        for (int i = 0; i <= count; ++i)
        {
            const qreal t = qDegreesToRadians(startAngle + sweep * i / count);
            result << transform.map(QPointF(rx * std::cos(t), ry * std::sin(t)));
        }

        return result;
    }

    Stroke transformed(const Stroke& points, const QTransform& transform)
    {
        Stroke result;

        for (const QPointF& point : points)
        {
            result << transform.map(point);
        }

        return result;
    }

    /// a realistic hand drawn version of the stroke, held still at the end
    Stroke handDrawn(const Stroke& points, qreal size, quint32 seed = 1)
    {
        return hold(wobble(smooth(points, 7), 0.012 * size, 0.004 * size, seed));
    }

    // ---------------------------------------------------------------------
    // Helpers for assertions
    // ---------------------------------------------------------------------

    qreal distance(const QPointF& a, const QPointF& b)
    {
        return QLineF(a, b).length();
    }

    /// largest distance from an expected vertex to the closest recognized vertex
    qreal vertexError(const QPolygonF& actual, const QList<QPointF>& expected)
    {
        qreal result = 0;

        for (const QPointF& e : expected)
        {
            qreal best = std::numeric_limits<qreal>::max();

            for (const QPointF& a : actual)
            {
                best = std::min(best, distance(a, e));
            }

            result = std::max(result, best);
        }

        return result;
    }

    QList<QPointF> corners(const QRectF& rect, qreal rotation = 0.)
    {
        QTransform transform;
        transform.translate(rect.center().x(), rect.center().y());
        transform.rotate(rotation);
        transform.translate(-rect.center().x(), -rect.center().y());

        return {transform.map(rect.topLeft()), transform.map(rect.topRight()),
                transform.map(rect.bottomRight()), transform.map(rect.bottomLeft())};
    }

    /// angle of the first edge of a polygon modulo 90°, in ]-45°, 45°]
    qreal edgeAngleMod90(const QPolygonF& polygon)
    {
        const QPointF d = polygon.at(1) - polygon.at(0);
        qreal angle = qRadiansToDegrees(std::atan2(d.y(), d.x()));
        angle = std::remainder(angle, 90.);
        return angle;
    }

    /// difference of two axis angles modulo 180°
    qreal axisAngleDifference(qreal a, qreal b)
    {
        return std::abs(std::remainder(a - b, 180.));
    }

    bool isFinite(const QPolygonF& polygon)
    {
        for (const QPointF& point : polygon)
        {
            if (!std::isfinite(point.x()) || !std::isfinite(point.y()))
            {
                return false;
            }
        }

        return true;
    }
}

class TestUBShapeRecognizer : public QObject
{
    Q_OBJECT

private slots:
    // lines
    void recognizesLines_data();
    void recognizesLines();
    void snapsLinesToAxes_data();
    void snapsLinesToAxes();
    void doesNotSnapDiagonalLines();

    // rectangles and squares
    void recognizesRectangles_data();
    void recognizesRectangles();
    void recognizesSquares();
    void snapsRectanglesToAxes();
    void keepsRotatedRectangles();

    // triangles and quadrilaterals
    void recognizesTriangles_data();
    void recognizesTriangles();
    void recognizesQuadrilaterals_data();
    void recognizesQuadrilaterals();

    // ellipses and circles
    void recognizesCircles_data();
    void recognizesCircles();
    void recognizesEllipses_data();
    void recognizesEllipses();

    // robustness
    void recognizesWithManyHands_data();
    void recognizesWithManyHands();
    void isScaleAndTranslationInvariant_data();
    void isScaleAndTranslationInvariant();
    void rejectsOtherStrokes_data();
    void rejectsOtherStrokes();
    void handlesDegenerateInput_data();
    void handlesDegenerateInput();
    void neverProducesInvalidResults();

    // result helpers
    void outlineIsClosedForClosedShapes();
};

// -------------------------------------------------------------------------
// lines
// -------------------------------------------------------------------------

void TestUBShapeRecognizer::recognizesLines_data()
{
    QTest::addColumn<QPointF>("from");
    QTest::addColumn<QPointF>("to");

    QTest::newRow("diagonal down") << QPointF(0, 0) << QPointF(300, 200);
    QTest::newRow("diagonal up") << QPointF(0, 200) << QPointF(300, 0);
    QTest::newRow("right to left") << QPointF(400, 120) << QPointF(50, 300);
    QTest::newRow("short") << QPointF(10, 10) << QPointF(60, 45);
    QTest::newRow("long") << QPointF(0, 0) << QPointF(1500, 900);
    QTest::newRow("negative coordinates") << QPointF(-500, -200) << QPointF(-100, -500);
}

void TestUBShapeRecognizer::recognizesLines()
{
    QFETCH(QPointF, from);
    QFETCH(QPointF, to);

    const qreal length = distance(from, to);
    const auto result = UBShapeRecognizer::recognize(handDrawn(line(from, to), length));

    QCOMPARE(result.shape, Shape::Line);
    QVERIFY(!result.closed);
    QCOMPARE(result.vertices.size(), 2);
    QVERIFY2(distance(result.vertices.at(0), from) < 0.03 * length, "start point moved too far");
    QVERIFY2(distance(result.vertices.at(1), to) < 0.03 * length, "end point moved too far");
}

void TestUBShapeRecognizer::snapsLinesToAxes_data()
{
    QTest::addColumn<qreal>("angle");
    QTest::addColumn<qreal>("expected");

    QTest::newRow("almost horizontal") << 3. << 0.;
    QTest::newRow("almost horizontal, other side") << -6. << 0.;
    QTest::newRow("almost vertical") << 86. << 90.;
    QTest::newRow("almost vertical down") << -93. << -90.;
    QTest::newRow("almost horizontal leftwards") << 177. << 180.;
    QTest::newRow("at tolerance") << UBShapeRecognizer::axisSnapTolerance - 0.5 << 0.;
}

void TestUBShapeRecognizer::snapsLinesToAxes()
{
    QFETCH(qreal, angle);
    QFETCH(qreal, expected);

    const QPointF from(100, 100);
    QLineF reference(from, from + QPointF(300, 0));
    reference.setAngle(-angle); // QLineF angles are counter-clockwise, scene y axis points down

    // draw the line without tremor, so that only the snapping is tested
    const auto result = UBShapeRecognizer::recognize(line(reference.p1(), reference.p2()));

    QCOMPARE(result.shape, Shape::Line);

    const QPointF d = result.vertices.at(1) - result.vertices.at(0);
    const qreal actual = qRadiansToDegrees(std::atan2(d.y(), d.x()));

    QVERIFY2(std::abs(std::remainder(actual - expected, 360.)) < 1e-6,
             qPrintable(QString("angle %1, expected %2").arg(actual).arg(expected)));
    QVERIFY(distance(result.vertices.at(0), from) < 1e-6);
    QVERIFY(std::abs(std::hypot(d.x(), d.y()) - 300) < 1e-6);
}

void TestUBShapeRecognizer::doesNotSnapDiagonalLines()
{
    for (qreal angle : {15., 30., 45., 60., 75.})
    {
        QLineF reference(QPointF(0, 0), QPointF(300, 0));
        reference.setAngle(-angle);

        const auto result = UBShapeRecognizer::recognize(line(reference.p1(), reference.p2()));
        QCOMPARE(result.shape, Shape::Line);

        const QPointF d = result.vertices.at(1) - result.vertices.at(0);
        QVERIFY(std::abs(qRadiansToDegrees(std::atan2(d.y(), d.x())) - angle) < 1e-6);
    }
}

// -------------------------------------------------------------------------
// rectangles and squares
// -------------------------------------------------------------------------

void TestUBShapeRecognizer::recognizesRectangles_data()
{
    QTest::addColumn<Stroke>("stroke");
    QTest::addColumn<QRectF>("rect");

    const QRectF rect(100, 100, 300, 180);
    const qreal size = 350;

    QTest::newRow("start at corner") << handDrawn(rectangle(rect), size) << rect;
    QTest::newRow("start mid edge") << handDrawn(rectangle(rect, 0, 0.5), size) << rect;
    QTest::newRow("start near corner") << handDrawn(rectangle(rect, 0, 0.9), size) << rect;
    QTest::newRow("counter clockwise") << handDrawn(rectangle(rect, 0, 0.3, 0, true), size) << rect;
    QTest::newRow("overshoot") << handDrawn(rectangle(rect, 0, 0.2, 0.08), size) << rect;
    QTest::newRow("large overshoot") << handDrawn(rectangle(rect, 0, 0.5, 0.2), size) << rect;
    QTest::newRow("gap") << handDrawn(rectangle(rect, 0, 0.6, -0.05), size) << rect;
    QTest::newRow("very rounded corners") << hold(wobble(smooth(rectangle(rect), 25), 3, 1, 7)) << rect;
    QTest::newRow("tall") << handDrawn(rectangle(QRectF(0, 0, 80, 400)), 400) << QRectF(0, 0, 80, 400);
    QTest::newRow("thin") << handDrawn(rectangle(QRectF(0, 0, 500, 60)), 500) << QRectF(0, 0, 500, 60);
}

void TestUBShapeRecognizer::recognizesRectangles()
{
    QFETCH(Stroke, stroke);
    QFETCH(QRectF, rect);

    const auto result = UBShapeRecognizer::recognize(stroke);
    const qreal diagonal = std::hypot(rect.width(), rect.height());

    QCOMPARE(result.shape, Shape::Rectangle);
    QVERIFY(result.closed);
    QCOMPARE(result.vertices.size(), 4);
    QVERIFY(!result.isSquare());

    const qreal error = vertexError(result.vertices, corners(rect));
    QVERIFY2(error < 0.04 * diagonal, qPrintable(QString("corner error %1").arg(error)));

    // perfectly axis aligned
    for (int i = 0; i < 4; ++i)
    {
        const QPointF d = result.vertices.at((i + 1) % 4) - result.vertices.at(i);
        QVERIFY(std::abs(d.x()) < 1e-6 || std::abs(d.y()) < 1e-6);
    }
}

void TestUBShapeRecognizer::recognizesSquares()
{
    // slightly uneven sides become a square
    const QRectF rect(0, 0, 200, 212);
    const auto result = UBShapeRecognizer::recognize(handDrawn(rectangle(rect, 0, 0.4), 290));

    QCOMPARE(result.shape, Shape::Rectangle);
    QVERIFY(result.isSquare());

    const qreal side = distance(result.vertices.at(0), result.vertices.at(1));
    QVERIFY(std::abs(side - 206) < 10);

    // clearly different sides do not
    const auto notSquare = UBShapeRecognizer::recognize(handDrawn(rectangle(QRectF(0, 0, 200, 260), 0, 0.4), 330));
    QCOMPARE(notSquare.shape, Shape::Rectangle);
    QVERIFY(!notSquare.isSquare());
}

void TestUBShapeRecognizer::snapsRectanglesToAxes()
{
    const QRectF rect(50, 50, 260, 160);

    for (qreal rotation : {-6., -3., 2., 5.})
    {
        const auto result = UBShapeRecognizer::recognize(handDrawn(rectangle(rect, rotation, 0.3), 300));

        QCOMPARE(result.shape, Shape::Rectangle);
        QVERIFY2(std::abs(edgeAngleMod90(result.vertices)) < 1e-6,
                 qPrintable(QString("rotation %1 not snapped").arg(rotation)));
    }
}

void TestUBShapeRecognizer::keepsRotatedRectangles()
{
    const QRectF rect(50, 50, 260, 160);

    for (qreal rotation : {15., 30., -25., 40.})
    {
        const auto result = UBShapeRecognizer::recognize(handDrawn(rectangle(rect, rotation, 0.3), 300));

        QCOMPARE(result.shape, Shape::Rectangle);

        const qreal difference = std::abs(std::remainder(edgeAngleMod90(result.vertices) - rotation, 90.));
        QVERIFY2(difference < 3., qPrintable(QString("rotation %1, difference %2").arg(rotation).arg(difference)));
        QVERIFY(vertexError(result.vertices, corners(rect, rotation)) < 0.04 * 300);
    }
}

// -------------------------------------------------------------------------
// triangles and quadrilaterals
// -------------------------------------------------------------------------

void TestUBShapeRecognizer::recognizesTriangles_data()
{
    QTest::addColumn<QList<QPointF>>("vertices");
    QTest::addColumn<qreal>("startFraction");
    QTest::addColumn<qreal>("closing");

    const QList<QPointF> equilateral{{200, 50}, {373.2, 350}, {26.8, 350}};
    const QList<QPointF> right{{50, 50}, {50, 350}, {450, 350}};
    const QList<QPointF> obtuse{{0, 300}, {400, 300}, {120, 200}};
    const QList<QPointF> isosceles{{0, 0}, {300, 0}, {150, 450}};

    QTest::newRow("equilateral") << equilateral << 0. << 0.;
    QTest::newRow("equilateral, start mid edge") << equilateral << 1.5 << 0.;
    QTest::newRow("equilateral, overshoot") << equilateral << 0.2 << 0.1;
    QTest::newRow("right") << right << 0. << 0.;
    QTest::newRow("right, gap") << right << 2.5 << -0.04;
    QTest::newRow("obtuse") << obtuse << 0.7 << 0.;
    QTest::newRow("isosceles") << isosceles << 0. << 0.05;
}

void TestUBShapeRecognizer::recognizesTriangles()
{
    QFETCH(QList<QPointF>, vertices);
    QFETCH(qreal, startFraction);
    QFETCH(qreal, closing);

    QPolygonF expected;

    for (const QPointF& vertex : vertices)
    {
        expected << vertex;
    }

    const qreal size = expected.boundingRect().width();
    const auto result = UBShapeRecognizer::recognize(handDrawn(polygon(vertices, startFraction, closing), size));

    QCOMPARE(result.shape, Shape::Triangle);
    QVERIFY(result.closed);
    QCOMPARE(result.vertices.size(), 3);

    const qreal error = vertexError(result.vertices, vertices);
    QVERIFY2(error < 0.05 * size, qPrintable(QString("vertex error %1").arg(error)));
}

void TestUBShapeRecognizer::recognizesQuadrilaterals_data()
{
    QTest::addColumn<QList<QPointF>>("vertices");

    QTest::newRow("parallelogram") << QList<QPointF>{{0, 0}, {300, 0}, {400, 200}, {100, 200}};
    QTest::newRow("trapezoid") << QList<QPointF>{{100, 0}, {300, 0}, {400, 250}, {0, 250}};
    QTest::newRow("rhombus") << QList<QPointF>{{200, 0}, {400, 150}, {200, 300}, {0, 150}};
    QTest::newRow("kite") << QList<QPointF>{{200, 0}, {320, 120}, {200, 400}, {80, 120}};
}

void TestUBShapeRecognizer::recognizesQuadrilaterals()
{
    QFETCH(QList<QPointF>, vertices);

    const auto result = UBShapeRecognizer::recognize(handDrawn(polygon(vertices, 0.5), 400));

    QCOMPARE(result.shape, Shape::Quadrilateral);
    QCOMPARE(result.vertices.size(), 4);

    const qreal error = vertexError(result.vertices, vertices);
    QVERIFY2(error < 0.05 * 400, qPrintable(QString("vertex error %1").arg(error)));
}

// -------------------------------------------------------------------------
// ellipses and circles
// -------------------------------------------------------------------------

void TestUBShapeRecognizer::recognizesCircles_data()
{
    QTest::addColumn<Stroke>("stroke");
    QTest::addColumn<QPointF>("center");
    QTest::addColumn<qreal>("radius");

    const QPointF c(300, 250);

    QTest::newRow("clockwise") << handDrawn(ellipse(c, 150, 150), 300) << c << 150.;
    QTest::newRow("counter clockwise") << handDrawn(ellipse(c, 150, 150, 0, 90, -360), 300) << c << 150.;
    QTest::newRow("start at top") << handDrawn(ellipse(c, 150, 150, 0, -90), 300) << c << 150.;
    QTest::newRow("overshoot") << handDrawn(ellipse(c, 150, 150, 0, 30, 400), 300) << c << 150.;
    QTest::newRow("gap") << handDrawn(ellipse(c, 150, 150, 0, 30, 335), 300) << c << 150.;
    QTest::newRow("drawn twice") << handDrawn(ellipse(c, 150, 150, 0, 0, 720), 300) << c << 150.;
    QTest::newRow("slightly oval") << handDrawn(ellipse(c, 150, 136, 20), 300) << c << 143.;
    QTest::newRow("small") << handDrawn(ellipse(c, 25, 25), 50) << c << 25.;
}

void TestUBShapeRecognizer::recognizesCircles()
{
    QFETCH(Stroke, stroke);
    QFETCH(QPointF, center);
    QFETCH(qreal, radius);

    const auto result = UBShapeRecognizer::recognize(stroke);

    QCOMPARE(result.shape, Shape::Ellipse);
    QVERIFY(result.isCircle());
    QVERIFY(result.closed);
    QCOMPARE(result.vertices.size(), UBShapeRecognizer::ellipseSegments);
    QVERIFY2(distance(result.center, center) < 0.06 * radius, qPrintable(QString("center error %1").arg(distance(result.center, center))));
    QVERIFY2(std::abs(result.radiusX - radius) < 0.06 * radius, qPrintable(QString("radius %1").arg(result.radiusX)));

    for (const QPointF& point : result.vertices)
    {
        QVERIFY(std::abs(distance(point, result.center) - result.radiusX) < 1e-6);
    }
}

void TestUBShapeRecognizer::recognizesEllipses_data()
{
    QTest::addColumn<qreal>("rx");
    QTest::addColumn<qreal>("ry");
    QTest::addColumn<qreal>("rotation");
    QTest::addColumn<qreal>("expectedAngle");

    QTest::newRow("wide") << 200. << 100. << 0. << 0.;
    QTest::newRow("tall") << 80. << 180. << 0. << 0.;
    QTest::newRow("wide, slightly tilted") << 200. << 110. << 5. << 0.;
    QTest::newRow("tall, slightly tilted") << 200. << 110. << 94. << 90.;
    QTest::newRow("rotated 30") << 200. << 100. << 30. << 30.;
    QTest::newRow("rotated -50") << 220. << 90. << -50. << -50.;
}

void TestUBShapeRecognizer::recognizesEllipses()
{
    QFETCH(qreal, rx);
    QFETCH(qreal, ry);
    QFETCH(qreal, rotation);
    QFETCH(qreal, expectedAngle);

    const QPointF center(400, 300);
    const auto result = UBShapeRecognizer::recognize(handDrawn(ellipse(center, rx, ry, rotation, 45), 2 * std::max(rx, ry)));

    QCOMPARE(result.shape, Shape::Ellipse);
    QVERIFY(!result.isCircle());
    QVERIFY(distance(result.center, center) < 0.05 * std::max(rx, ry));

    // compare major/minor radii and the direction of the major axis
    const qreal major = std::max(result.radiusX, result.radiusY);
    const qreal minor = std::min(result.radiusX, result.radiusY);
    QVERIFY2(std::abs(major - std::max(rx, ry)) < 0.07 * std::max(rx, ry), qPrintable(QString("major %1").arg(major)));
    QVERIFY2(std::abs(minor - std::min(rx, ry)) < 0.07 * std::max(rx, ry), qPrintable(QString("minor %1").arg(minor)));

    const qreal majorAngle = result.radiusX >= result.radiusY ? result.angle : result.angle + 90;
    const qreal expectedMajorAngle = rx >= ry ? expectedAngle : expectedAngle + 90;
    const qreal difference = axisAngleDifference(majorAngle, expectedMajorAngle);

    if (std::abs(std::remainder(expectedAngle, 90.)) < 1e-9)
    {
        QVERIFY2(difference < 1e-6, qPrintable(QString("angle %1 not snapped").arg(result.angle)));
    }
    else
    {
        QVERIFY2(difference < 3., qPrintable(QString("angle %1, expected %2").arg(majorAngle).arg(expectedMajorAngle)));
    }
}

// -------------------------------------------------------------------------
// robustness
// -------------------------------------------------------------------------

void TestUBShapeRecognizer::recognizesWithManyHands_data()
{
    QTest::addColumn<int>("kind");
    QTest::addColumn<Shape>("expected");

    QTest::newRow("line") << 0 << Shape::Line;
    QTest::newRow("rectangle") << 1 << Shape::Rectangle;
    QTest::newRow("triangle") << 2 << Shape::Triangle;
    QTest::newRow("circle") << 3 << Shape::Ellipse;
    QTest::newRow("ellipse") << 4 << Shape::Ellipse;
}

void TestUBShapeRecognizer::recognizesWithManyHands()
{
    // the same shape drawn by many different (simulated) hands must always be recognized
    QFETCH(int, kind);
    QFETCH(Shape, expected);

    for (quint32 seed = 1; seed <= 100; ++seed)
    {
        QRandomGenerator random(seed * 7919);
        const qreal start = random.bounded(1.);
        const qreal closing = random.bounded(0.12) - 0.04;
        const qreal angle = random.bounded(360.);
        Stroke base;

        switch (kind)
        {
        case 0:
        {
            QLineF reference(QPointF(0, 0), QPointF(200 + random.bounded(400.), 0));
            reference.setAngle(angle);
            base = line(reference.p1(), reference.p2());
            break;
        }
        case 1:
            base = rectangle(QRectF(0, 0, 200 + random.bounded(200.), 120 + random.bounded(150.)), 0, start * 4, closing);
            break;
        case 2:
            base = polygon({{0, 0}, {300, 20}, {120 + random.bounded(60.), 260}}, start * 3, closing);
            break;
        case 3:
            base = ellipse(QPointF(), 150, 150, 0, angle, 360 + closing * 360);
            break;
        case 4:
            base = ellipse(QPointF(), 220, 110, angle, 0, 360 + closing * 360);
            break;
        }

        const auto result = UBShapeRecognizer::recognize(handDrawn(base, 300, seed));
        QVERIFY2(result.shape == expected, qPrintable(QString("failed for seed %1, got %2").arg(seed).arg(int(result.shape))));
    }
}

void TestUBShapeRecognizer::isScaleAndTranslationInvariant_data()
{
    QTest::addColumn<Stroke>("stroke");
    QTest::addColumn<Shape>("expected");

    QTest::newRow("line") << handDrawn(line({0, 0}, {300, 120}), 300) << Shape::Line;
    QTest::newRow("rectangle") << handDrawn(rectangle(QRectF(0, 0, 300, 200), 0, 0.5), 300) << Shape::Rectangle;
    QTest::newRow("triangle") << handDrawn(polygon({{0, 0}, {300, 0}, {150, 250}}), 300) << Shape::Triangle;
    QTest::newRow("circle") << handDrawn(ellipse({}, 150, 150), 300) << Shape::Ellipse;
}

void TestUBShapeRecognizer::isScaleAndTranslationInvariant()
{
    QFETCH(Stroke, stroke);
    QFETCH(Shape, expected);

    for (qreal scale : {0.01, 0.1, 1., 10., 1000.})
    {
        for (const QPointF& offset : {QPointF(0, 0), QPointF(-5000, 3000), QPointF(1e5, 1e5)})
        {
            QTransform transform;
            transform.translate(offset.x(), offset.y());
            transform.scale(scale, scale);

            const auto result = UBShapeRecognizer::recognize(transformed(stroke, transform));
            QVERIFY2(result.shape == expected, qPrintable(QString("scale %1, offset %2/%3").arg(scale).arg(offset.x()).arg(offset.y())));
        }
    }
}

void TestUBShapeRecognizer::rejectsOtherStrokes_data()
{
    QTest::addColumn<Stroke>("stroke");

    // open curves
    QTest::newRow("arc 90") << handDrawn(ellipse({}, 200, 200, 0, 0, 90), 300);
    QTest::newRow("half circle") << handDrawn(ellipse({}, 200, 200, 0, 0, 180), 400);
    QTest::newRow("three quarter circle") << handDrawn(ellipse({}, 200, 200, 0, 0, 260), 400);

    Stroke wave;
    for (int i = 0; i <= 400; ++i)
    {
        wave << QPointF(i, 40 * std::sin(i / 30.));
    }
    QTest::newRow("sine wave") << handDrawn(wave, 400);

    // polylines which are not closed
    QTest::newRow("L shape") << handDrawn(densify({{0, 0}, {0, 300}, {200, 300}}), 300);
    QTest::newRow("V shape") << handDrawn(densify({{0, 0}, {150, 300}, {300, 0}}), 300);
    QTest::newRow("zigzag") << handDrawn(densify({{0, 0}, {50, 100}, {100, 0}, {150, 100}, {200, 0}, {250, 100}}), 250);
    QTest::newRow("U shape") << handDrawn(densify({{0, 0}, {0, 300}, {200, 300}, {200, 0}}), 300);
    QTest::newRow("open rectangle") << handDrawn(rectangle(QRectF(0, 0, 300, 200), 0, 0, -0.3), 300);

    // handwriting and scribbles
    Stroke spiral;
    for (int i = 0; i <= 720; ++i)
    {
        const qreal t = qDegreesToRadians(qreal(i));
        spiral << QPointF((20 + i / 4.) * std::cos(t), (20 + i / 4.) * std::sin(t));
    }
    QTest::newRow("spiral") << handDrawn(spiral, 400);

    Stroke cursive;
    for (int i = 0; i <= 600; ++i)
    {
        const qreal t = i / 30.;
        cursive << QPointF(i / 2. + 15 * std::cos(t), 30 * std::sin(t));
    }
    QTest::newRow("cursive loops") << handDrawn(cursive, 300);

    Stroke figureEight;
    for (int i = 0; i <= 360; ++i)
    {
        const qreal t = qDegreesToRadians(qreal(i));
        figureEight << QPointF(150 * std::sin(t), 150 * std::sin(t) * std::cos(t));
    }
    QTest::newRow("figure eight") << handDrawn(figureEight, 300);

    Stroke star;
    for (int i = 0; i <= 5; ++i)
    {
        const qreal t = qDegreesToRadians(-90. + i * 144.);
        star << QPointF(150 * std::cos(t), 150 * std::sin(t));
    }
    QTest::newRow("star") << handDrawn(densify(star), 300);

    QTest::newRow("pentagon") << handDrawn(polygon({{150, 0}, {293, 104}, {238, 271}, {62, 271}, {7, 104}}, 0.5), 300);
    QTest::newRow("hexagon") << handDrawn(polygon({{100, 0}, {200, 0}, {250, 87}, {200, 173}, {100, 173}, {50, 87}}, 0.5), 200);
    QTest::newRow("bow tie") << handDrawn(densify({{0, 0}, {300, 200}, {300, 0}, {0, 200}, {0, 0}}), 300);
    QTest::newRow("back and forth") << handDrawn(densify({{0, 0}, {300, 0}, {0, 3}}), 300);
    QTest::newRow("heart") << handDrawn(densify({{150, 80}, {200, 0}, {300, 20}, {280, 120}, {150, 260}, {20, 120}, {0, 20}, {100, 0}, {150, 80}}), 300);

    QRandomGenerator random(1234);
    Stroke scribble{{0, 0}};
    for (int i = 0; i < 60; ++i)
    {
        scribble << scribble.last() + QPointF(random.bounded(80.) - 40, random.bounded(80.) - 40);
    }
    QTest::newRow("random scribble") << densify(scribble);
}

void TestUBShapeRecognizer::rejectsOtherStrokes()
{
    QFETCH(Stroke, stroke);

    const auto result = UBShapeRecognizer::recognize(stroke);
    QVERIFY2(result.shape == Shape::None, qPrintable(QString("recognized as %1").arg(int(result.shape))));
    QVERIFY(!result.isValid());
    QVERIFY(result.vertices.isEmpty());
}

void TestUBShapeRecognizer::handlesDegenerateInput_data()
{
    QTest::addColumn<Stroke>("stroke");

    const qreal nan = std::numeric_limits<qreal>::quiet_NaN();
    const qreal inf = std::numeric_limits<qreal>::infinity();

    QTest::newRow("empty") << Stroke();
    QTest::newRow("single point") << Stroke{{10, 10}};
    QTest::newRow("same point repeated") << Stroke{{10, 10}, {10, 10}, {10, 10}, {10, 10}};
    QTest::newRow("pen held still") << hold(Stroke{{10, 10}}, 200, 0.);
    QTest::newRow("nan") << Stroke{{nan, nan}, {nan, 0}};
    QTest::newRow("nan and point") << Stroke{{nan, nan}, {5, 5}};
    QTest::newRow("infinity") << Stroke{{0, 0}, {inf, 0}, {0, inf}};
}

void TestUBShapeRecognizer::handlesDegenerateInput()
{
    QFETCH(Stroke, stroke);

    const auto result = UBShapeRecognizer::recognize(stroke);
    QCOMPARE(result.shape, Shape::None);
}

void TestUBShapeRecognizer::neverProducesInvalidResults()
{
    // fuzzing with random strokes: no crash, and results always satisfy their invariants
    QRandomGenerator random(20261106);

    for (int i = 0; i < 3000; ++i)
    {
        Stroke stroke;
        const int count = random.bounded(1, 200);
        const qreal step = random.bounded(50.) + 0.001;
        QPointF point(random.bounded(1000.), random.bounded(1000.));

        for (int j = 0; j < count; ++j)
        {
            stroke << point;

            if (random.bounded(4) == 0)
            {
                continue; // duplicates
            }

            point += QPointF(random.bounded(2 * step) - step, random.bounded(2 * step) - step);
        }

        // sometimes close the stroke
        if (random.bounded(2) == 0 && !stroke.isEmpty())
        {
            stroke << stroke.first();
        }

        const auto result = UBShapeRecognizer::recognize(stroke);
        QVERIFY(isFinite(result.vertices));

        switch (result.shape)
        {
        case Shape::None:
            QVERIFY(result.vertices.isEmpty());
            break;
        case Shape::Line:
            QCOMPARE(result.vertices.size(), 2);
            QVERIFY(!result.closed);
            break;
        case Shape::Triangle:
            QCOMPARE(result.vertices.size(), 3);
            QVERIFY(result.closed);
            break;
        case Shape::Quadrilateral:
        case Shape::Rectangle:
            QCOMPARE(result.vertices.size(), 4);
            QVERIFY(result.closed);
            break;
        case Shape::Ellipse:
            QCOMPARE(result.vertices.size(), UBShapeRecognizer::ellipseSegments);
            QVERIFY(result.closed);
            QVERIFY(result.radiusX > 0 && result.radiusY > 0);
            break;
        }
    }
}

// -------------------------------------------------------------------------
// result helpers
// -------------------------------------------------------------------------

void TestUBShapeRecognizer::outlineIsClosedForClosedShapes()
{
    const auto lineResult = UBShapeRecognizer::recognize(line({0, 0}, {100, 50}));
    QCOMPARE(lineResult.outline().size(), 2);

    const auto rectResult = UBShapeRecognizer::recognize(handDrawn(rectangle(QRectF(0, 0, 300, 200)), 300));
    const QList<QPointF> outline = rectResult.outline();
    QCOMPARE(outline.size(), 5);
    QCOMPARE(outline.first(), outline.last());

    const auto circleResult = UBShapeRecognizer::recognize(handDrawn(ellipse({}, 100, 100), 200));
    QCOMPARE(circleResult.outline().size(), UBShapeRecognizer::ellipseSegments + 1);

    QVERIFY(UBShapeRecognizer::Result().outline().isEmpty());
}

QTEST_APPLESS_MAIN(TestUBShapeRecognizer)

#include "tst_UBShapeRecognizer.moc"
