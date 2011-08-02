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
