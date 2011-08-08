/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
