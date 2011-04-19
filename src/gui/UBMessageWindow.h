/*
 * UBMessageWindow.h
 *
 *  Created on: Dec 5, 2008
 *      Author: Luc
 */

#ifndef UBMESSAGEWINDOW_H_
#define UBMESSAGEWINDOW_H_

#include <QtGui>

#include "UBFloatingPalette.h"

class UBSpinningWheel;

class UBMessageWindow  : public UBFloatingPalette
{
    Q_OBJECT;

    public:
        UBMessageWindow(QWidget *parent = 0);
        virtual ~UBMessageWindow();

        void showMessage(const QString& message, bool showSpinningWheel = false);

        void hideMessage();

    protected:
        void timerEvent(QTimerEvent *event);

    private:
        QHBoxLayout *mLayout;
        UBSpinningWheel *mSpinningWheel;
        QLabel *mLabel;
        QBasicTimer  mTimer;

        int mOriginalAlpha;
        int mFadingStep;

        int mTimerID;
};

#endif /* UBMESSAGEWINDOW_H_ */
