/*
 * UBWidgetMirror.h
 *
 *  Created on: Nov 25, 2008
 *      Author: luc
 */

#ifndef UBWIDGETMIRROR_H_
#define UBWIDGETMIRROR_H_

#include <QtGui>

class UBWidgetMirror : public QWidget
{

    Q_OBJECT;

    public:
        UBWidgetMirror(QWidget* sourceWidget, QWidget* parent = 0);
        virtual ~UBWidgetMirror();

    public slots:
        void setSourceWidget(QWidget *sourceWidget);

    protected:
        bool eventFilter(QObject *obj, QEvent *event);
        virtual void paintEvent ( QPaintEvent * event );

    private:
        QWidget* mSourceWidget;

};

#endif /* UBWIDGETMIRROR_H_ */
