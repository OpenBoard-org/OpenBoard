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


#ifndef UBIDLETIMER_H_
#define UBIDLETIMER_H_

#include <QObject>
#include <QDateTime>

class QEvent;

class UBIdleTimer : public QObject
{
    Q_OBJECT

    public:

        UBIdleTimer(QObject *parent = 0);
        virtual ~UBIdleTimer();

    protected:

        bool eventFilter(QObject *obj, QEvent *event);
        virtual void timerEvent(QTimerEvent *event);

    private:

        QDateTime mLastInputEventTime;
        bool mCursorIsHidden;
};


#endif /* UBIDLETIMER_H_ */
