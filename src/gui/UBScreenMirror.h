/*
 * UBScreenMirror.h
 *
 *  Created on: Nov 25, 2008
 *      Author: luc
 */

#ifndef UBSCREENMIRROR_H_
#define UBSCREENMIRROR_H_

#include <QtGui>

class UBScreenMirror : public QWidget
{
    Q_OBJECT;

    public:
        UBScreenMirror(QWidget* parent = 0);
        virtual ~UBScreenMirror();

        virtual void paintEvent (QPaintEvent * event);
        virtual void timerEvent(QTimerEvent *event);

    public slots:

        void setSourceWidget(QWidget *sourceWidget);

        void setSourceRect(const QRect& pRect)
        {
            mRect = pRect;
            mSourceWidget = 0;
        }

        void start();

        void stop();

    private:

        void grabPixmap();

        int mScreenIndex;

        QWidget* mSourceWidget;

        QRect mRect;

        QPixmap mLastPixmap;

        long mTimerID;

};

#endif /* UBSCREENMIRROR_H_ */
