#ifndef UBINTERPOLATOR_H
#define UBINTERPOLATOR_H

#include <QtGui>

#include "spline.h"
#include "interpolation.h"

class UBInterpolator
{
    /* Abstract class representing an interpolator */

public:
    enum InterpolationMethod {
        NoInterpolation,
        SimpleSpline,
        CatmullRom,
        Bezier
    };

    UBInterpolator();
    virtual ~UBInterpolator();

    virtual void setPoints(QList<QPointF> points) = 0;
    virtual double y(double x) {}

};


class UBSimpleSpline : public UBInterpolator
{
    /* A basic cubic spline interpolator, that requires only three
     * points to interpolate between the second and third one.
     * To do so, the curvature at p2 is set to 0, so the resulting
     * curve is not very smooth.
     * However, it is better than linear interpolation and requires no
     * "future" points, so it can be used seamlessly during drawing.
    */

public:
    UBSimpleSpline();
    virtual ~UBSimpleSpline() {}

    virtual void setPoints(QList<QPointF> points);
    void setPoints(QPointF p0, QPointF p1, QPointF p2);

    virtual double y(double x);

private:
    long double m_a,
           m_b,
           m_c,
           m_d;
};


class UBCatmullRomSpline : public UBInterpolator
{
    /* Catmull-Rom spline, using AlgLib as backend
     *
     * This requires four points to interpolate between the middle two.
    */

public:
    UBCatmullRomSpline();
    virtual ~UBCatmullRomSpline();

    virtual void setPoints(QList<QPointF> points);
    virtual double y(double x);

private:
    alglib::spline1dinterpolant * mInterpolant;

};


class UBQuadraticBezier : public UBInterpolator
{

public:
    UBQuadraticBezier();
    virtual ~UBQuadraticBezier();

    virtual void setPoints(QList<QPointF> points);
    void setPoints(QPointF start, QPointF control, QPointF end);

    //virtual double y(double x);

    QList<QPointF> getPoints(int n);

private:

    QPainterPath* mPath;
};

#endif // UBINTERPOLATOR_H
