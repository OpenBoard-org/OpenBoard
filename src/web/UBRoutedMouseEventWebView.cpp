/*
 * UBRoutedMouseEventWebView.cpp
 *
 *  Created on: 6 juil. 2009
 *      Author: Luc
 */

#include "UBRoutedMouseEventWebView.h"

#include <QtCore>
#include <QtWebKit>
#include <QtGui>

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

