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



#ifndef UBCLOCKPALLETTE_H_
#define UBCLOCKPALLETTE_H_

class QHBoxLayout;
class QLabel;

#include "UBFloatingPalette.h"

class UBClockPalette : public UBFloatingPalette
{
    Q_OBJECT;

    public:
        UBClockPalette(QWidget *parent = 0);
        virtual ~UBClockPalette();

    protected:
        int radius();
        void timerEvent(QTimerEvent *event);

        virtual void showEvent ( QShowEvent * event );
        virtual void hideEvent ( QShowEvent * event );


    protected slots:
        void updateTime();

    private:

        QLabel *mTimeLabel;
        int mTimerID;
        QString mTimeFormat;

};

#endif /* UBCLOCKPALLETTE_H_ */
