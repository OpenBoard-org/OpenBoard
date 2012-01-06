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

#ifndef UBROUTEDMOUSEEVENTWEBVIEW_H_
#define UBROUTEDMOUSEEVENTWEBVIEW_H_

#include <QtWebKit>

/*
 * This class changes the routing of mouse events.
 * Qt 4.5.2 does not report back the "accepted" flag set
 * by WebKit, as we want to know if a mouse event has been
 * consumed by webKit, we override them without keeping the
 * accepted flag status
 */

class UBRoutedMouseEventWebView : public QWebView
{
    public:
        UBRoutedMouseEventWebView(QWidget * parent = 0 );
        virtual ~UBRoutedMouseEventWebView();

    protected:
        virtual void mouseMoveEvent(QMouseEvent* ev);
        virtual void mousePressEvent(QMouseEvent* ev);
        virtual void mouseDoubleClickEvent(QMouseEvent* ev);
        virtual void mouseReleaseEvent(QMouseEvent* ev);
        virtual void contextMenuEvent(QContextMenuEvent* ev);
        virtual void wheelEvent(QWheelEvent* ev);
};

#endif /* UBROUTEDMOUSEEVENTWEBVIEW_H_ */
