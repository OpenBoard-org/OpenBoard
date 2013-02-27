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
