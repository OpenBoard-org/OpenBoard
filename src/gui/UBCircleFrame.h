/*
 * UBCircleFrame.h
 *
 *  Created on: Nov 18, 2008
 *      Author: luc
 */

#ifndef UBCIRCLEFRAME_H_
#define UBCIRCLEFRAME_H_

#include <QtGui>

class UBCircleFrame : public QFrame
{
    public:
        UBCircleFrame(QWidget* parent);
        virtual ~UBCircleFrame();

        qreal currentPenWidth;
        qreal maxPenWidth;

    protected:

        virtual void paintEvent (QPaintEvent * event);
};

#endif /* UBCIRCLEFRAME_H_ */
