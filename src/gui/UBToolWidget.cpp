/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include <QtGui>
#include <QWebView>
#include <QWebFrame>

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


UBToolWidget::UBToolWidget(const QUrl& pUrl, QWidget *pParent)
    : QWidget(pParent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , mWebView(0)
    , mToolWidget(0)
    , mShouldMoveWidget(false)
    , mContentMargin(0)
    , mFrameWidth(0)

{
    int widgetType = UBGraphicsWidgetItem::widgetType(pUrl);
    if (widgetType == UBWidgetType::Apple)
        mToolWidget = new UBGraphicsAppleWidgetItem(pUrl);
    else if (widgetType == UBWidgetType::W3C)
        mToolWidget = new UBGraphicsW3CWidgetItem(pUrl);
    else
        qDebug() << "UBToolWidget::UBToolWidget: Unknown widget Type";

    initialize();
}

UBToolWidget::UBToolWidget(UBGraphicsWidgetItem *pWidget, QWidget *pParent)
    : QWidget(pParent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , mWebView(0)
    , mToolWidget(pWidget)
    , mShouldMoveWidget(false)
    , mContentMargin(0)
    , mFrameWidth(0)

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

    UBGraphicsScene *wscene = dynamic_cast<UBGraphicsScene *>(mToolWidget->scene());
    if (wscene)
    {
        wscene->removeItemFromDeletion(mToolWidget);
        wscene->removeItem(mToolWidget);
    }


    mWebView = new QWebView(this);

    QPalette palette = mWebView->page()->palette();
    palette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    mWebView->page()->setPalette(palette);


    mWebView->installEventFilter(this);

    mFrameWidth = UBSettings::settings()->objectFrameWidth;
    mContentMargin = sClosePixmap->width() / 2 + mFrameWidth;
    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(mContentMargin, mContentMargin, mContentMargin, mContentMargin);
    layout()->addWidget(mWebView);

    setFixedSize(mToolWidget->boundingRect().width() + mContentMargin * 2, mToolWidget->boundingRect().height() + mContentMargin * 2);

    connect(mWebView->page()->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared,
            this, &UBToolWidget::javaScriptWindowObjectCleared);
    mWebView->load(mToolWidget->mainHtml());


    mWebView->setAcceptDrops(false);
    mWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    mWebView->setAttribute(Qt::WA_OpaquePaintEvent, false);


    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(javaScriptWindowObjectCleared()));
}


bool UBToolWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (mShouldMoveWidget && obj == mWebView && event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(event);

        if (mouseMoveEvent->buttons() & Qt::LeftButton)
        {
            move(pos() - mMousePressPos + mWebView->mapTo(this, mouseMoveEvent->pos()));

            event->accept();
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void UBToolWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    /* did webkit consume the mouse press ? */
    mShouldMoveWidget = !event->isAccepted() && (event->buttons() & Qt::LeftButton);
    mMousePressPos = event->pos();
    event->accept();
    update();
}

void UBToolWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(mShouldMoveWidget && (event->buttons() & Qt::LeftButton)) {
        move(pos() - mMousePressPos + event->pos());
        event->accept();
    }
    QWidget::mouseMoveEvent(event);
}

void UBToolWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mShouldMoveWidget = false;

    if (event->pos().x() >= 0 && event->pos().x() < sClosePixmap->width() && event->pos().y() >= 0 && event->pos().y() < sClosePixmap->height()) {
        UBApplication::boardController->removeTool(this);
        event->accept();
    }
    else if (mToolWidget->canBeContent() && event->pos().x() >= mContentMargin && event->pos().x() < mContentMargin + sUnpinPixmap->width() && event->pos().y() >= 0 && event->pos().y() < sUnpinPixmap->height()) {
        UBApplication::boardController->moveToolWidgetToScene(this);
        event->accept();
    }
    else
        QWidget::mouseReleaseEvent(event); /* don't propgate to parent, the widget is deleted in UBApplication */

}

void UBToolWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    //if (isActiveWindow())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(127, 127, 127, 127));

        painter.drawRoundedRect(QRectF(sClosePixmap->width() / 2
                                     , sClosePixmap->height() / 2
                                     , width() - sClosePixmap->width()
                                     , mFrameWidth)
                                     , mFrameWidth / 2
                                     , mFrameWidth / 2);

        painter.drawPixmap(0, 0, *sClosePixmap);

        if (mToolWidget->canBeContent())
            painter.drawPixmap(mContentMargin, 0, *sUnpinPixmap);
    }
}

void UBToolWidget::javaScriptWindowObjectCleared()
{
    UBWidgetUniboardAPI *uniboardAPI = new UBWidgetUniboardAPI(UBApplication::boardController->activeScene(), mToolWidget);

    mWebView->page()->mainFrame()->addToJavaScriptWindowObject("sankore", uniboardAPI);

    UBGraphicsW3CWidgetItem *graphicsW3cWidgetItem = dynamic_cast<UBGraphicsW3CWidgetItem*>(mToolWidget);
    if (graphicsW3cWidgetItem)
    {
        UBW3CWidgetAPI* widgetAPI = new UBW3CWidgetAPI(graphicsW3cWidgetItem);
        mWebView->page()->mainFrame()->addToJavaScriptWindowObject("widget", widgetAPI);
    }
}

UBGraphicsWidgetItem* UBToolWidget::toolWidget() const
{
    return mToolWidget;
}

QPoint UBToolWidget::naturalCenter() const
{
    if (mWebView)
        return mWebView->geometry().center();
    else
        return QPoint(0, 0);
}

void UBToolWidget::remove()
{
    mToolWidget = NULL;
    hide();
    deleteLater();
}

void UBToolWidget::centerOn(const QPoint& pos)
{
    QWidget::move(pos - QPoint(width() / 2, height() / 2));
}
