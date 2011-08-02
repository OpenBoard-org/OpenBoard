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

#ifndef UBICONBUTTON_H_
#define UBICONBUTTON_H_

#include <QAbstractButton>

class UBIconButton : public QAbstractButton
{
    Q_OBJECT;

    public:
        UBIconButton(QWidget *parent = 0, const QIcon &iconOn = QIcon());

        void setToggleable(bool pToggleable) {mToggleable = pToggleable;}

    protected:
        virtual void paintEvent(QPaintEvent *);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseDoubleClickEvent(QMouseEvent *event);
        QSize minimumSizeHint () const;

    private:
        QSize mIconSize;
        bool mToggleable;

    signals:
        void doubleClicked();
};

#endif /* UBICONBUTTON_H_ */
