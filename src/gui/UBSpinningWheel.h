/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

