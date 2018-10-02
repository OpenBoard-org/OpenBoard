/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBSCREENMIRROR_H_
#define UBSCREENMIRROR_H_

#include <QtGui>
#include <QWidget>

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
