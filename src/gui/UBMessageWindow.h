/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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
