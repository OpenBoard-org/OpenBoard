/*
 * UBMousePressFilter.h
 *
 *  Created on: Jan 12, 2009
 *      Author: luc
 */

#ifndef UBMOUSEPRESSFILTER_H_
#define UBMOUSEPRESSFILTER_H_

#include <QtGui>

class UBMousePressFilter : public QObject
{
    Q_OBJECT;

    public:
        UBMousePressFilter(QObject* parent = 0);
        virtual ~UBMousePressFilter();

    protected:
        bool eventFilter(QObject *obj, QEvent *event);

    protected slots:
        void mouseDownElapsed();

    private:

        QObject* mObject;
        QMouseEvent* mPendingEvent;
};

#endif /* UBMOUSEPRESSFILTER_H_ */
