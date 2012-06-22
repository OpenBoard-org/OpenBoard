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

#ifndef UBRUBBERBAND_H_
#define UBRUBBERBAND_H_

#include <QRubberBand>

class UBRubberBand : public QRubberBand
{
    Q_OBJECT;

    public:
        UBRubberBand(Shape s, QWidget * p = 0);
        virtual ~UBRubberBand();

    private:
        enum enm_resizingMode
        {
            None,
            Top,
            TopLeft,
            TopRight,
            Bottom,
            BottomLeft,
            BottomRight,
            Left,
            Right
        };

        enm_resizingMode determineResizingMode(QPoint pos);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);

    private:
        QStyle* customStyle;
        enm_resizingMode mResizingMode;
        int mResizingBorderHeight;
        bool mMouseIsPressed;
        QPoint mLastPressedPoint;
        QPoint mLastMousePos;
};

#endif /* UBRUBBERBAND_H_ */
