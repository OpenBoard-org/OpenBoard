#ifndef UBINTERPOLATOR_H
#define UBINTERPOLATOR_H

#include <QtGui>

class UBInterpolator
{
    /* Abstract class representing an interpolator */

public:
    enum InterpolationMethod {
        NoInterpolation,
        //SimpleSpline,
        //CatmullRom,
        Bezier
    };

    UBInterpolator();
    virtual ~UBInterpolator();

    virtual void setPoints(QList<QPointF> points) = 0;
    //virtual double y(double x) {}

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
