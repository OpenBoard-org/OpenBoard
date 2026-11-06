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


#pragma once

#include <QList>
#include <QPointF>
#include <QPolygonF>

/**
 * @brief Recognize simple geometric shapes in a freehand stroke.
 *
 * The recognizer is purely geometric and has no dependency on the scene or
 * on any global state, so that it can be unit tested in isolation.
 *
 * It recognizes:
 * - straight lines (open strokes),
 * - triangles and quadrilaterals (closed strokes with 3 or 4 corners);
 *   nearly rectangular quadrilaterals become perfect rectangles or squares,
 * - ellipses (closed, round strokes); nearly round ellipses become circles.
 *
 * Lines and rectangles which are almost horizontal or vertical are snapped
 * to the axes, as are the axes of ellipses.
 *
 * Anything else (handwriting, scribbles, open curves, ...) is rejected.
 */
class UBShapeRecognizer
{
public:
    enum class Shape
    {
        None,
        Line,
        Triangle,
        Quadrilateral,
        Rectangle,
        Ellipse
    };

    struct Result
    {
        Shape shape{Shape::None};

        /**
         * Vertices of the shape. For a line, the two end points. For triangles,
         * quadrilaterals and rectangles, the corners (not repeating the first one).
         * For an ellipse, points sampled along the outline (not repeating the first one).
         */
        QPolygonF vertices;

        /// true for every shape except a line
        bool closed{false};

        // Ellipse parameters, only valid if shape == Ellipse.
        QPointF center;
        qreal radiusX{0};
        qreal radiusY{0};
        /// rotation of the X axis of the ellipse in degrees (clockwise in scene coordinates)
        qreal angle{0};

        bool isValid() const { return shape != Shape::None; }
        bool isSquare() const;
        bool isCircle() const;

        /**
         * Outline of the shape as a polyline. For closed shapes, the first point
         * is repeated at the end.
         */
        QList<QPointF> outline() const;
    };

    /// tolerance in degrees for snapping lines, rectangles and ellipses to the axes
    static constexpr qreal axisSnapTolerance = 8.;

    /// number of points used to sample the outline of an ellipse
    static constexpr int ellipseSegments = 72;

    static Result recognize(const QList<QPointF>& points);

private:
    UBShapeRecognizer() = delete;
};
