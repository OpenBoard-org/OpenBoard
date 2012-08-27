/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBSPINNINGWHEEL_H_
#define UBSPINNINGWHEEL_H_

#include <QWidget>

class UBSpinningWheel : public QWidget
{
    Q_OBJECT;

    public:
        UBSpinningWheel(QWidget *parent = 0);
        virtual ~UBSpinningWheel();
        virtual void paintEvent(QPaintEvent *event);

    public slots:
        void startAnimation();
        void stopAnimation();

    protected:
        virtual QSize sizeHint() const;
        virtual void timerEvent(QTimerEvent *event);

    private:
        QAtomicInt mPosition;
        int mTimerID;
};

#endif /* UBSPINNINGWHEEL_H_ */

