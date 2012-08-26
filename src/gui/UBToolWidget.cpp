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


#include <QtGui>
#include "UBToolWidget.h"
#include "api/UBWidgetUniboardAPI.h"
#include "api/UBW3CWidgetAPI.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/memcheck.h"


QPixmap* UBToolWidget::sClosePixmap = 0;
QPixmap* UBToolWidget::sUnpinPixmap = 0;


UBToolWidget::UBToolWidget(const QUrl& pUrl, QGraphicsItem *pParent)
    : QGraphicsWidget(pParent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , mShouldMoveWidget(false)
    , mContentMargin(0)
    , mFrameWidth(0)
    , mGraphicsWebView(0)
    , mGraphicsWidgetItem(0)    
{
    int widgetType = UBGraphicsWidgetItem::widgetType(pUrl);
    if (widgetType == UBWidgetType::Apple)
        mGraphicsWidgetItem = new UBGraphicsAppleWidgetItem(pUrl, this);
    else if (widgetType == UBWidgetType::W3C)
        mGraphicsWidgetItem = new UBGraphicsW3CWidgetItem(pUrl, this);
    else
        qDebug() << "UBToolWidget::UBToolWidget: Unknown widget Type";

    initialize();
}

UBToolWidget::UBToolWidget(UBGraphicsWidgetItem *pWidget, QGraphicsItem *pParent)
    : QGraphicsWidget(pParent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , mShouldMoveWidget(false)
    , mContentMargin(0)
    , mFrameWidth(0)
    , mGraphicsWebView(0)
    , mGraphicsWidgetItem(pWidget)
{
    initialize();
    javaScriptWindowObjectCleared();
}

UBToolWidget::~UBToolWidget()
{
    // NOOP
}

void UBToolWidget::initialize()
{
    if (!sClosePixmap)
        sClosePixmap = new QPixmap(":/images/close.svg");

    if(!sUnpinPixmap)
        sUnpinPixmap = new QPixmap(":/images/unpin.svg");
    
    QGraphicsLinearLayout *graphicsLayout = new QGraphicsLinearLayout(Qt::Vertical, this);

    mFrameWidth = UBSettings::settings()->objectFrameWidth;
    mContentMargin = sClosePixmap->width() / 2 + mFrameWidth;
    graphicsLayout->setContentsMargins(mContentMargin, mContentMargin, mContentMargin, mContentMargin);
    setPreferredSize(mGraphicsWidgetItem->preferredWidth() + mContentMargin * 2, mGraphicsWidgetItem->preferredHeight() + mContentMargin * 2);

    mGraphicsWebView = new QGraphicsWebView();
    connect(mGraphicsWebView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
    mGraphicsWebView->load(mGraphicsWidgetItem->mainHtml());
    graphicsLayout->addItem(mGraphicsWebView);

    mGraphicsWebView->setAcceptDrops(false);
    mGraphicsWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    mGraphicsWebView->setAttribute(Qt::WA_OpaquePaintEvent, false);

    QPalette palette = mGraphicsWebView->page()->palette();
    palette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    mGraphicsWebView->page()->setPalette(palette);

    setLayout(graphicsLayout);

    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(javaScriptWindowObjectCleared()));
}

bool UBToolWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
        event->accept();
    return QGraphicsWidget::event(event);
}

void UBToolWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mousePressEvent(event);

    /* did webkit consume the mouse press ? */
    mShouldMoveWidget = !event->isAccepted() && (event->buttons() & Qt::LeftButton);
    mMousePressPos = event->pos();
    event->accept();
    update();
}

void UBToolWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(mShouldMoveWidget && (event->buttons() & Qt::LeftButton)) {
        setPos(pos() - mMousePressPos + event->pos());
        event->accept();
    }
    QGraphicsWidget::mouseMoveEvent(event);
}

void UBToolWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mShouldMoveWidget = false;

    if (event->pos().x() >= 0 && event->pos().x() < sClosePixmap->width() && event->pos().y() >= 0 && event->pos().y() < sClosePixmap->height()) {
        remove();
        event->accept();
    }
    else if (mGraphicsWidgetItem->canBeContent() && event->pos().x() >= mContentMargin && event->pos().x() < mContentMargin + sUnpinPixmap->width() && event->pos().y() >= 0 && event->pos().y() < sUnpinPixmap->height()) {
        UBApplication::boardController->moveToolWidgetToScene(this);
        event->accept();
    }
    else
        QGraphicsWidget::mouseReleaseEvent(event); /* don't propgate to parent, the widget is deleted in UBApplication */

}

void UBToolWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsWidget::paint(painter, option, widget);

    /* painting lightweight toolbar */
    if (isActiveWindow()) {
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(127, 127, 127, 127));

        painter->drawRoundedRect(QRectF(sClosePixmap->width() / 2, sClosePixmap->height() / 2, preferredWidth() - sClosePixmap->width(), mFrameWidth), mFrameWidth / 2, mFrameWidth / 2);
        painter->drawPixmap(0, 0, *sClosePixmap);
        if (mGraphicsWidgetItem->canBeContent())
            painter->drawPixmap(mContentMargin, 0, *sUnpinPixmap);
    }
}

void UBToolWidget::javaScriptWindowObjectCleared()
{
    UBWidgetUniboardAPI *uniboardAPI = new UBWidgetUniboardAPI(UBApplication::boardController->activeScene(), mGraphicsWidgetItem);

    mGraphicsWebView->page()->mainFrame()->addToJavaScriptWindowObject("sankore", uniboardAPI);

    UBGraphicsW3CWidgetItem *graphicsW3cWidgetItem = dynamic_cast<UBGraphicsW3CWidgetItem*>(mGraphicsWidgetItem);
    if (graphicsW3cWidgetItem)
    {
        UBW3CWidgetAPI* widgetAPI = new UBW3CWidgetAPI(graphicsW3cWidgetItem);
        mGraphicsWebView->page()->mainFrame()->addToJavaScriptWindowObject("widget", widgetAPI);
    }
}

UBGraphicsWidgetItem* UBToolWidget::graphicsWidgetItem() const
{
    return mGraphicsWidgetItem;
}

QPointF UBToolWidget::naturalCenter() const
{
    if (mGraphicsWebView)
        return mGraphicsWebView->geometry().center();
    else
        return QPointF(0, 0);
}

void UBToolWidget::centerOn(const QPointF& pos)
{
    QGraphicsWidget::setPos(pos - QPointF(preferredWidth() / 2, preferredHeight() / 2));
}

void UBToolWidget::remove()
{
    mGraphicsWebView->setHtml(QString());
    scene()->removeItem(this);
}

UBGraphicsScene* UBToolWidget::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}

QPointF UBToolWidget::pos() const
{
    return QPointF(QGraphicsItem::pos().x() + mContentMargin * scale(), QGraphicsItem::pos().y() + mContentMargin * scale());
}

void UBToolWidget::setPos(const QPointF &point)
{
    UBToolWidget::setPos(point.x(), point.y());
}

void UBToolWidget::setPos(qreal x, qreal y)
{
    QGraphicsItem::setPos(x - mContentMargin * scale(), y - mContentMargin * scale());
}

int UBToolWidget::type() const
{
    return Type;
}
