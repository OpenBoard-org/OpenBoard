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

#include <QtGui>
#include "UBMagnifer.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"


UBMagnifier::UBMagnifier(QWidget *parent, bool isInteractive) 
    : QWidget(parent, parent ? Qt::Widget : Qt::Tool | (Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint))
    , mShouldMoveWidget(false)
    , mShouldResizeWidget(false)
    , inTimer(false)
    , borderPen(Qt::darkGray)
    , gView(0)
    , mView(0)
{
    isCusrsorAlreadyStored = false;
    setMouseTracking(true);

    //--------------------------------------------------//

    QPixmap pix(":/images/cursors/resize.png");
    QTransform tr;
    tr.rotate(45);
    mResizeCursor  = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);

    //--------------------------------------------------//

    params.sizePercentFromScene = 20;
    m_isInteractive = isInteractive;
    sClosePixmap = new QPixmap(":/images/close.svg");
    sIncreasePixmap = new QPixmap(":/images/increase.svg");
    sDecreasePixmap = new QPixmap(":/images/decrease.svg");
    mResizeItem = new QPixmap(":/images/resize.svg");

    if (parent)
    {
        setAttribute(Qt::WA_NoMousePropagation);
    }
    else
    {
        // standalone window
        // !!!! Should be included into Windows after QT recompilation
#ifndef Q_WS_WIN
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_MacAlwaysShowToolWindow);
#endif
#ifdef Q_WS_MAC
        setAttribute(Qt::WA_MacAlwaysShowToolWindow);
        setAttribute(Qt::WA_MacNonActivatingToolWindow);
        setAttribute(Qt::WA_MacNoShadow);
#endif
    }

}

UBMagnifier::~UBMagnifier()
{
    if (timerUpdate != 0)
    {
        killTimer(timerUpdate);
        timerUpdate = 0;
    }

    if(sClosePixmap)
    {
        delete sClosePixmap;
        sClosePixmap = NULL;
    }

    if(sIncreasePixmap)
    {
        delete sIncreasePixmap;
        sIncreasePixmap = NULL;
    }

    if(sDecreasePixmap)
    {
        delete sDecreasePixmap;
        sDecreasePixmap = NULL;
    }

}

void UBMagnifier::setSize(qreal percentFromScene) 
{
    if(gView == NULL || mView == NULL) return;

    // calculate object size
    params.sizePercentFromScene = percentFromScene;
    QSize sceneSize = mView->size();
    qreal size = params.sizePercentFromScene * sceneSize.width() / 100;

    QRect currGeom = geometry();
    if(currGeom.width() == currGeom.height())
    {
        QPoint newPos = mView->mapFromGlobal(updPointMove);
        setGeometry(newPos.x() - size / 2, newPos.y() - size / 2, size, size);
    }
    else
        setGeometry(0, 0, size, size);

    // prepare transparent bit mask
    QImage mask_img(width(), height(), QImage::Format_Mono);
    mask_img.fill(0xff);
    QPainter mask_ptr(&mask_img);
    mask_ptr.setBrush( QBrush( QColor(0, 0, 0) ) );
    mask_ptr.drawEllipse(QPointF(size/2, size/2), size / 2 - sClosePixmap->width(), size / 2 - sClosePixmap->width());
    bmpMask = QBitmap::fromImage(mask_img);

    // prepare general image
    pMap = QPixmap(width(), height());
    pMap.fill(Qt::transparent);
    pMap.setMask(bmpMask);
}

void UBMagnifier::setZoom(qreal zoom) 
{
    params.zoom = zoom;
}

void UBMagnifier::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (m_isInteractive)
    {
        painter.setBrush(QColor(127, 127, 127, 127));
        painter.drawRoundedRect(QRectF(size().width() / 2, size().height() / 2, ( size().width() - sClosePixmap->width() ) / 2, ( size().height() - sClosePixmap->width() ) / 2), 15, 15);

        painter.setBrush(QColor(190, 190, 190, 255));
        painter.drawEllipse(QPoint( size().width() / 2, size().height() / 2), ( size().width() - sClosePixmap->width() ) / 2, ( size().height() -  sClosePixmap->height() ) / 2);

        painter.drawPixmap(size().width() - sClosePixmap->width(), size().height() / 2 + sClosePixmap->height() * 1, *sClosePixmap);
        painter.drawPixmap(size().width() - sIncreasePixmap->width(), size().height() / 2 + sIncreasePixmap->height() * 2.5, *sIncreasePixmap);
        painter.drawPixmap(size().width() - sDecreasePixmap->width(), size().height() / 2 + sDecreasePixmap->height() * 3.6, *sDecreasePixmap);

        painter.drawPixmap(size().width() - mResizeItem->width() - 20, size().height() - mResizeItem->height() - 20, *mResizeItem);
    }
    else
    {
        painter.setBrush(QColor(127, 127, 127, 127));
        painter.drawEllipse(QPoint( size().width() / 2, size().height() / 2), ( size().width() - sClosePixmap->width() ) / 2, ( size().height() -  sClosePixmap->height() ) / 2);
    }

    painter.drawPixmap(0, 0, pMap);
}

void UBMagnifier::mousePressEvent ( QMouseEvent * event )
{
    if(m_isInteractive)
    {

        QWidget::mousePressEvent(event);

        if (event->pos().x() >= size().width() - mResizeItem->width() - 20 && 
            event->pos().x() < size().width() - 20 && 
            event->pos().y() >= size().height() - mResizeItem->height() - 20 && 
            event->pos().y() < size().height() - - 20)
        {
            mShouldResizeWidget = true;
        }
        else
        {
            mShouldMoveWidget = !event->isAccepted() && (event->buttons() & Qt::LeftButton);
        }

        mMousePressPos = event->pos();
        mMousePressDelta = (qreal)updPointGrab.x() + (qreal)size().width() / 2 - (qreal)event->globalPos().x();

        event->accept();

        update();
    }
    else
        event->ignore();
}

void UBMagnifier::mouseMoveEvent ( QMouseEvent * event )
{
    if(m_isInteractive)
    {
        if(mShouldMoveWidget && (event->buttons() & Qt::LeftButton))
        {
            move(pos() - mMousePressPos + event->pos());
            event->accept();

            QWidget::mouseMoveEvent(event);
            emit magnifierMoved_Signal(QPoint(this->pos().x() + size().width() / 2, this->pos().y() + size().height() / 2 ));
            return;
        }
    
        if(mShouldResizeWidget && (event->buttons() & Qt::LeftButton))
        {

            QPoint currGlobalPos = event->globalPos();
            qreal cvW = mView->width();

            qreal newXSize = ( currGlobalPos.x() + mMousePressDelta - updPointGrab.x() ) * 2;
            qreal newPercentSize = newXSize * 100 / cvW;

            emit magnifierResized_Signal(newPercentSize);

            event->ignore();
            return;
        }

        if (event->pos().x() >= size().width() - mResizeItem->width() - 20 && 
            event->pos().x() < size().width() - 20 && 
            event->pos().y() >= size().height() - mResizeItem->height() - 20 && 
            event->pos().y() < size().height() - - 20 &&
            isCusrsorAlreadyStored == false
           )
        {
            mOldCursor = cursor();
            isCusrsorAlreadyStored = true;
            setCursor(mResizeCursor);
        }

    }
    else
        event->ignore();
}


void UBMagnifier::mouseReleaseEvent(QMouseEvent * event)
{
    if(m_isInteractive)
    {
        mShouldMoveWidget = false;
        mShouldResizeWidget = false;

        if (event->pos().x() >= size().width() - sClosePixmap->width() && 
            event->pos().x() < size().width()&& 
            event->pos().y() >= size().height() / 2 + sClosePixmap->height() * 1 && 
            event->pos().y() < size().height() / 2 + sClosePixmap->height() * 2)
        {
            event->accept();
            emit magnifierClose_Signal();
        }
        else
        if (event->pos().x() >= size().width() - sIncreasePixmap->width() && 
            event->pos().x() < size().width()&& 
            event->pos().y() >= size().height() / 2 + sIncreasePixmap->height() * 2.5 && 
            event->pos().y() < size().height() / 2 + sIncreasePixmap->height() * 3.5)
        {
            event->accept();
            emit magnifierZoomIn_Signal();
        }
        else
        if (event->pos().x() >= size().width() - sDecreasePixmap->width() && 
            event->pos().x() < size().width()&& 
            event->pos().y() >= size().height() / 2 + sDecreasePixmap->height() * 3.6 && 
            event->pos().y() < size().height() / 2 + sDecreasePixmap->height() * 4.6)
        {
            event->accept();
            emit magnifierZoomOut_Signal();
        }
        else
            QWidget::mouseReleaseEvent(event); // don't propgate to parent, the widget is deleted in UBApplication::boardController->removeTool
    }
    else
        event->ignore();

}

void UBMagnifier::timerEvent(QTimerEvent *e)
{
    if(inTimer) return;
    if (e->timerId() == timerUpdate)
    {
        inTimer = true;
        if(!(updPointGrab.isNull())) 
            grabPoint(updPointGrab);

        if(isCusrsorAlreadyStored)
        {
            QPoint globalCursorPos = QCursor::pos();
            QPoint cursorPos = mapFromGlobal(globalCursorPos);
            if (cursorPos.x() < size().width() - mResizeItem->width() - 20 || 
                cursorPos.x() > size().width() - 20 ||
                cursorPos.y() < size().height() - mResizeItem->height() - 20 ||
                cursorPos.y() > size().height() - - 20
                )
            {
                isCusrsorAlreadyStored = false;
                setCursor(mOldCursor);
            }

        }

        inTimer = false;
    }
}

void UBMagnifier::grabPoint()
{
    QPointF itemPos = gView->mapFromGlobal(updPointGrab);

    qreal zWidth = size().width() / params.zoom;
    qreal zWidthHalf = zWidth / 2;
    qreal zHeight = size().height() / params.zoom;
    qreal zHeightHalf = zHeight / 2;

    int x = itemPos.x() - zWidthHalf;
    int y = itemPos.y() - zHeightHalf;

    QPixmap newPixMap(QSize(zWidth,zHeight));
    ((QWidget*)gView)->render(&newPixMap, QPoint(0, 0), QRegion(x, y, zWidth, zHeight));
    UBApplication::boardController->activeScene()->update();

    pMap.fill(Qt::transparent);
    pMap = newPixMap.scaled(QSize(width(), height()));
    pMap.setMask(bmpMask);

    update();
}

void UBMagnifier::grabPoint(const QPoint &pGrab)
{
    updPointGrab = pGrab;
    QPointF itemPos = gView->mapFromGlobal(pGrab);

    qreal zWidth = size().width() / params.zoom;
    qreal zWidthHalf = zWidth / 2;
    qreal zHeight = size().height() / params.zoom;
    qreal zHeightHalf = zHeight / 2;

    int x = itemPos.x() - zWidthHalf;
    int y = itemPos.y() - zHeightHalf;

    QPixmap newPixMap(QSize(zWidth,zHeight));
    ((QWidget*)gView)->render(&newPixMap, QPoint(0, 0), QRegion(x, y, zWidth, zHeight));
    UBApplication::boardController->activeScene()->update();

    pMap.fill(Qt::transparent);
    pMap = newPixMap.scaled(QSize(width(), height()));
    pMap.setMask(bmpMask);

    update();
}

// from global
void UBMagnifier::grabNMove(const QPoint &pGrab, const QPoint &pMove, bool needGrab, bool needMove)
{
    updPointGrab = pGrab;
    updPointMove = pMove;

    if(needGrab)
        grabPoint(pGrab);

    if(needMove)
    {
        QPoint movePos = mView->mapFromGlobal(pMove);
        move(movePos.x() - width()/2, movePos.y() - height()/2);
        //    move(itemPos.x(), itemPos.y());
    }
}

void UBMagnifier::setGrabView(QWidget *view)
{
    if (timerUpdate != 0)
        killTimer(timerUpdate);
    gView = view;
    timerUpdate = startTimer(200);
}

