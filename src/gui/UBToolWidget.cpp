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

#include <QtGui>

#include "UBToolWidget.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "domain/UBAbstractWidget.h"
#include "domain/UBAppleWidget.h"
#include "domain/UBW3CWidget.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsWidgetItem.h"

#include "api/UBWidgetUniboardAPI.h"
#include "api/UBW3CWidgetAPI.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "core/memcheck.h"

QPixmap* UBToolWidget::sClosePixmap = 0;
QPixmap* UBToolWidget::sUnpinPixmap = 0;


UBToolWidget::UBToolWidget(const QUrl& pUrl, QWidget* pParent)
    : QWidget(pParent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , mToolWidget(0)
    , mShouldMoveWidget(false)
{
    int widgetType = UBAbstractWidget::widgetType(pUrl);

    if(widgetType == UBWidgetType::Apple)
    {
        mToolWidget = new UBAppleWidget(pUrl, this);
    }
    else if(widgetType == UBWidgetType::W3C)
    {
        mToolWidget = new UBW3CWidget(pUrl, this);
    }
    else
    {
        qDebug() << "UBToolWidget::UBToolWidget: Unknown widget Type";
        return;
    }

    initialize();
}


UBToolWidget::UBToolWidget(UBAbstractWidget* pWidget, QWidget* pParent)
    : QWidget(pParent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , mToolWidget(pWidget)
    , mShouldMoveWidget(false)
{
    mToolWidget->setParent(this);
    mToolWidget->loadMainHtml();

    initialize();

    javaScriptWindowObjectCleared();
}


UBToolWidget::~UBToolWidget()
{
    // NOOP
}


void UBToolWidget::initialize()
{
    mToolWidget->setAcceptDrops(false);

    if(!sClosePixmap)
    {
        sClosePixmap = new QPixmap(":/images/close.svg");
    }

    if(!sUnpinPixmap)
    {
        sUnpinPixmap = new QPixmap(":/images/unpin.svg");
    }

    mToolWidget->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    mToolWidget->setAttribute(Qt::WA_OpaquePaintEvent, false);

    QPalette palette = mToolWidget->page()->palette();
    palette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    mToolWidget->page()->setPalette(palette);

    connect(mToolWidget->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(javaScriptWindowObjectCleared()));

    setLayout(new QVBoxLayout());

    mFrameWidth = UBSettings::settings()->objectFrameWidth;
    mContentMargin = sClosePixmap->width() / 2 + mFrameWidth;

    layout()->setContentsMargins(mContentMargin, mContentMargin, mContentMargin, mContentMargin);
    layout()->addWidget(mToolWidget);

    setFixedSize(mToolWidget->width() + mContentMargin * 2, mToolWidget->height() + mContentMargin * 2);

    mToolWidget->installEventFilter(this);
}


void UBToolWidget::javaScriptWindowObjectCleared()
{
    UBWidgetUniboardAPI* uniboardAPI = new UBWidgetUniboardAPI(UBApplication::boardController->activeScene());

    mToolWidget->page()->mainFrame()->addToJavaScriptWindowObject("sankore", uniboardAPI);

    UBW3CWidget* w3c = dynamic_cast<UBW3CWidget*>(mToolWidget);
    if (w3c)
    {
        UBW3CWidgetAPI* widgetAPI = new UBW3CWidgetAPI(w3c);
        mToolWidget->page()->mainFrame()->addToJavaScriptWindowObject("widget", widgetAPI);
    }
}


void UBToolWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (isActiveWindow())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(127, 127, 127, 127));

        painter.drawRoundedRect(QRectF(sClosePixmap->width() / 2
            , sClosePixmap->height() / 2
            , width() - sClosePixmap->width()
            , mFrameWidth), mFrameWidth / 2, mFrameWidth / 2);

        painter.drawPixmap(0, 0, *sClosePixmap);

        if (mToolWidget->canBeContent())
            painter.drawPixmap(mContentMargin, 0, *sUnpinPixmap);
    }
}


void UBToolWidget::mousePressEvent ( QMouseEvent * event )
{
    QWidget::mousePressEvent(event);

    // did webkit consume the mouse press ?
    mShouldMoveWidget = !event->isAccepted() && (event->buttons() & Qt::LeftButton);

    mMousePressPos = event->pos();

    event->accept();

    update();
}


void UBToolWidget::mouseMoveEvent ( QMouseEvent * event )
{
    if(mShouldMoveWidget && (event->buttons() & Qt::LeftButton))
    {
        move(pos() - mMousePressPos + event->pos());
        event->accept();
    }

    QWidget::mouseMoveEvent(event);

}


void UBToolWidget::mouseReleaseEvent(QMouseEvent * event)
{
    mShouldMoveWidget = false;

    if (event->pos().x() >= 0 && event->pos().x() < sClosePixmap->width()
        && event->pos().y() >= 0 && event->pos().y() < sClosePixmap->height())
    {
        UBApplication::boardController->removeTool(this);
        event->accept();
    }
    else if (mToolWidget->canBeContent() && event->pos().x() >= mContentMargin && event->pos().x() < mContentMargin + sUnpinPixmap->width()
        && event->pos().y() >= 0 && event->pos().y() < sUnpinPixmap->height())
    {
        UBApplication::boardController->moveToolWidgetToScene(this);
        event->accept();
    }
    else
        QWidget::mouseReleaseEvent(event); // don't propgate to parent, the widget is deleted in UBApplication::boardController->removeTool

}


bool UBToolWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (mShouldMoveWidget && obj == mToolWidget && event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(event);

        if (mouseMoveEvent->buttons() & Qt::LeftButton)
        {
            move(pos() - mMousePressPos + mToolWidget->mapTo(this, mouseMoveEvent->pos()));

            event->accept();
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}


void UBToolWidget::centerOn(const QPoint& pos)
{
    QWidget::move(pos - QPoint(width() / 2, height() / 2));
}


QPoint UBToolWidget::naturalCenter() const
{
    if (mToolWidget)
        return mToolWidget->geometry().center();
    else
        return QPoint(0, 0);
}


UBAbstractWidget* UBToolWidget::webWidget() const
{
    return mToolWidget;
}
