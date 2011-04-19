/*
 * UBRoutedMouseEventWebView.h
 *
 *  Created on: 6 juil. 2009
 *      Author: Luc
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
