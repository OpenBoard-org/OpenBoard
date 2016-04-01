#include "UBInterpolator.h"

UBInterpolator::UBInterpolator()
{
}

UBInterpolator::~UBInterpolator()
{
}


UBSimpleSpline::UBSimpleSpline()
{

}

void UBSimpleSpline::setPoints(QList<QPointF> points)
{
    setPoints(points[0], points[1], points[2]);
}

void UBSimpleSpline::setPoints(QPointF p0, QPointF p1, QPointF p2)
{
    /*
    p0 -= p0;
    p1 -= p0;
    p2 -= p0;
    */
    long double x0, x1, x2, y0, y1, y2;
    x0 = p0.x();
    x1 = p1.x();
    x2 = p2.x();
    y0 = p0.y();
    y1 = p1.y();
    y2 = p2.y();

    long double k1 = (y2-y0)/(x2-x0);

    m_a = (y1-y2-k1*(x1-x2))/(pow(x1,3) - pow(x2,3) - 3*x2*(pow(x1,2)-pow(x2,2)) -
            3*(pow(x1,2) - 2*x1*x2)*(x1-x2));

    m_b = -3*m_a*x2;
    m_c = k1 - 3*m_a*pow(x1,2) - 2*m_b*x1;
    m_d = y1 - m_a*pow(x1,3) - m_b*pow(x1,2) - m_c*x1;
}

double UBSimpleSpline::y(double x)
{
    return m_a*pow(x, 3) + m_b*pow(x, 2) + m_c*x + m_d;
}

UBCatmullRomSpline::UBCatmullRomSpline()
{
    mInterpolant = 0;
}

UBCatmullRomSpline::~UBCatmullRomSpline()
{
    if (mInterpolant)
        delete mInterpolant;
}

void UBCatmullRomSpline::setPoints(QList<QPointF> points)
{
    // todo : basis change to avoid crashing when several X's are equal
    mInterpolant = new alglib::spline1dinterpolant();

    // alglib arrays are defined as strings
    QString x = "[";
    QString y = "[";

    foreach(QPointF point, points) {
        x += (QString::number(point.x()) + QString(","));
        y += (QString::number(point.y()) + QString(","));
    }

    x.chop(1);
    y.chop(1);

    x+="]";
    y+="]";

    alglib::real_1d_array xArray = x.toLatin1().data();
    alglib::real_1d_array yArray = y.toLatin1().data();

    alglib::spline1dbuildcatmullrom(xArray, yArray, *mInterpolant);


}

double UBCatmullRomSpline::y(double x)
{
    return alglib::spline1dcalc(*mInterpolant, x);
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
 * The higher n, the more accurate the result
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
