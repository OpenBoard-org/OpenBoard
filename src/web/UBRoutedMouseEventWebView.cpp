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

#include "UBRoutedMouseEventWebView.h"

#include <QtCore>
#include <QtWebKit>
#include <QtGui>

#include "core/memcheck.h"

UBRoutedMouseEventWebView::UBRoutedMouseEventWebView(QWidget * parent)
    : QWebView(parent)
{
    QWebView::setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
}

UBRoutedMouseEventWebView::~UBRoutedMouseEventWebView()
{
    // NOOP
}


void UBRoutedMouseEventWebView::mouseMoveEvent(QMouseEvent* ev)
{
    QWebPage* p = page();

    if (p)
    {
        p->event(ev);
    }
}

void UBRoutedMouseEventWebView::mousePressEvent(QMouseEvent* ev)
{
    QWebPage* p = page();

    if (p)
    {
        p->event(ev);
    }
}

void UBRoutedMouseEventWebView::mouseDoubleClickEvent(QMouseEvent* ev)
{
    QWebPage* p = page();

    if (p)
    {
        p->event(ev);
    }
}

void UBRoutedMouseEventWebView::mouseReleaseEvent(QMouseEvent* ev)
{
    QWebPage* p = page();

    if (p)
    {
        p->event(ev);
    }
}

void UBRoutedMouseEventWebView::contextMenuEvent(QContextMenuEvent* ev)
{
    QWebPage* p = page();

    if (p)
    {
        p->event(ev);
    }
}


void UBRoutedMouseEventWebView::wheelEvent(QWheelEvent* ev)
{
    QWebPage* p = page();

    if (p)
    {
        p->event(ev);
    }
}
//void UBRoutedMouseEventWebView::dropEvent(QDropEvent *event)
//{
////    QWebView::dropEvent(event);
//    event->accept();
//}
