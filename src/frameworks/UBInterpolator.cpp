#include "UBInterpolator.h"

UBInterpolator::UBInterpolator()
{
}

UBInterpolator::~UBInterpolator()
{
}

UBQuadraticBezier::UBQuadraticBezier()
{
    mPath = 0;
}

UBQuadraticBezier::~UBQuadraticBezier()
{
    if (mPath)
        delete mPath;
}

void UBQuadraticBezier::setPoints(QList<QPointF> points)
{
    setPoints(points[0], points[1], points[2]);

}

void UBQuadraticBezier::setPoints(QPointF start, QPointF control, QPointF end)
{
    mPath = new QPainterPath(start);
    mPath->quadTo(control, end);
}

/**
 * @brief Return n points along the curve, including start and end points (thus n should be larger than or equal to 2).
 *
 * The higher n, the more accurate the resulting curve will be.
 */
QList<QPointF> UBQuadraticBezier::getPoints(int n)
{
    QList<QPointF> points;

    if (n <= 1)
        return points;

    for (int i(0); i <= n; ++i) {
        qreal percent = qreal(i)/qreal(n);

        points << mPath->pointAtPercent(percent);
    }

    return points;
}
