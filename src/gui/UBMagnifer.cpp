/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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
#include "UBMagnifer.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBBoardView.h"

#include "core/memcheck.h"


UBMagnifier::UBMagnifier(QWidget *parent, bool isInteractive)
    : QWidget(parent, parent ? Qt::Widget : Qt::Tool | (Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint))
    , mShouldMoveWidget(false)
    , mShouldResizeWidget(false)
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
    sChangeModePixmap = new QPixmap();

    qDebug() << "sClosePixmap" << sClosePixmap->size() << '\n'
             << "sIncreasePixmap" << sIncreasePixmap->size() << '\n'
             << "sDecreasePixmap" << sDecreasePixmap->size() << '\n'
             << "mResizeItem" << mResizeItem->size() << '\n';


    setDrawingMode(UBSettings::settings()->magnifierDrawingMode->get().toInt());

    if (parent)
    {
        setAttribute(Qt::WA_NoMousePropagation);
    }
    else
    {
        // standalone window
        // !!!! Should be included into Windows after QT recompilation
#ifndef Q_OS_WIN
//        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_MacAlwaysShowToolWindow);
#endif
#ifdef Q_OS_OSX
        setAttribute(Qt::WA_MacAlwaysShowToolWindow);
        setAttribute(Qt::WA_MacNoShadow);
#endif
    }

    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(slot_refresh()));
}

UBMagnifier::~UBMagnifier()
{
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

    if (sChangeModePixmap)
    {
        delete sChangeModePixmap;
        sChangeModePixmap = NULL;
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
    if (circular == mDrawingMode)
    {
        if(currGeom.width() == currGeom.height())
        {
            QPoint newPos = mView->mapFromGlobal(updPointMove);
            setGeometry(newPos.x() - size / 2, newPos.y() - size / 2, size, size);
        }
        else
            setGeometry(0, 0, size, size);
    }
    else if (rectangular == mDrawingMode)
    {
        QPoint newPos = mView->mapFromGlobal(updPointMove);
        setGeometry(newPos.x() - size / 2, newPos.y() - size / 2 / 3, size, size/3);
    }

    calculateButtonsPositions();
    createMask();

}

void UBMagnifier::createMask()
{
    if(gView == NULL || mView == NULL) return;

    // calculate object size
    QSize sceneSize = mView->size();
    qreal isize = params.sizePercentFromScene * sceneSize.width() / 100;

    QImage mask_img(width(), height(), QImage::Format_Mono);
    mask_img.fill(0xff);
    QPainter mask_ptr(&mask_img);
    mask_ptr.setBrush( QBrush( QColor(0, 0, 0) ) );

    if (circular == mDrawingMode)
        mask_ptr.drawEllipse(QPointF(isize/2, isize/2), isize / 2 - sClosePixmap->width(), isize / 2 - sClosePixmap->width());
    else if (rectangular == mDrawingMode)
        mask_ptr.drawRoundedRect(QRect(sClosePixmap->width(), sClosePixmap->width(), size().width() - 2*sClosePixmap->width(), size().height() - 2*sClosePixmap->width()), sClosePixmap->width()/2, sClosePixmap->width()/2);

    bmpMask = QBitmap::fromImage(mask_img);

    pMap = QPixmap(width(), height());
    pMap.fill(Qt::transparent);
    pMap.setMask(bmpMask);
}

void UBMagnifier::setZoom(qreal zoom)
{
    params.zoom = zoom;
}


void UBMagnifier::calculateButtonsPositions()
{
    qDebug() << "current widget size is " << size();

    m_iButtonInterval = 5;
    mResizeItemButtonRect = QRect(size().width() - 1.5*mResizeItem->width() - m_iButtonInterval, size().height() - 1.5*mResizeItem->height() - m_iButtonInterval, mResizeItem->width(), mResizeItem->height());
    sClosePixmapButtonRect = QRect(mResizeItemButtonRect.x() - sChangeModePixmap->width() - 3*m_iButtonInterval, size().height() - sChangeModePixmap->height(), sChangeModePixmap->width(), sChangeModePixmap->height());
    sChangeModePixmapButtonRect = QRect(sClosePixmapButtonRect.x() - sChangeModePixmap->width() - m_iButtonInterval, size().height() - sChangeModePixmap->height(), sChangeModePixmap->width(), sChangeModePixmap->height());
    sDecreasePixmapButtonRect = QRect(sChangeModePixmapButtonRect.x() - sChangeModePixmap->width() - m_iButtonInterval, size().height() - sDecreasePixmap->height(), sDecreasePixmap->width(), sDecreasePixmap->height());
    sIncreasePixmapButtonRect = QRect(sDecreasePixmapButtonRect.x() - sChangeModePixmap->width() - m_iButtonInterval, size().height() - sIncreasePixmap->height(), sIncreasePixmap->width(), sIncreasePixmap->height());

    qDebug() << "mResizeItemButtonRect" << mResizeItemButtonRect << '\n'
             << "sClosePixmapButtonRect" << sClosePixmapButtonRect << '\n'
             << "sChangeModePixmapButtonRect" << sChangeModePixmapButtonRect << '\n'
             << "sDecreasePixmapButtonRect" << sDecreasePixmapButtonRect << '\n'
             << "sIncreasePixmapButtonRect" << sIncreasePixmapButtonRect << '\n';
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
    }

    painter.setBrush(QColor(190, 190, 190, 255));
    if (circular == mDrawingMode)
    {
        painter.drawEllipse(QPoint(size().width() / 2, size().height() / 2), ( size().width() - sClosePixmap->width() ) / 2, ( size().height() -  sClosePixmap->height() ) / 2);
    }
    else if (rectangular == mDrawingMode)
    {
        QRect r = QRect(sClosePixmap->width()/2, sClosePixmap->width()/2, size().width()- sClosePixmap->width(), size().height() - sClosePixmap->width());
        painter.drawRoundedRect(r, sClosePixmap->width()/2, sClosePixmap->width()/2);
    }

    painter.drawPixmap(0, 0, pMap);

    if (m_isInteractive)
    {
        painter.setBrush(QColor(190, 190, 190, 255));
        painter.drawPixmap(sClosePixmapButtonRect.topLeft(), *sClosePixmap);
        painter.drawPixmap(sIncreasePixmapButtonRect.topLeft(), *sIncreasePixmap);
        painter.drawPixmap(sDecreasePixmapButtonRect.topLeft(), *sDecreasePixmap);
        painter.drawPixmap(sChangeModePixmapButtonRect.topLeft(), *sChangeModePixmap);
        painter.drawPixmap(mResizeItemButtonRect.topLeft(), *mResizeItem);
    }
}

void UBMagnifier::mousePressEvent ( QMouseEvent * event )
{
    if(m_isInteractive)
    {

        QWidget::mousePressEvent(event);

        if (event->pos().x() >= size().width() - mResizeItem->width() - 14 &&
            event->pos().x() < size().width() - 14 &&
            event->pos().y() >= size().height() - mResizeItem->height() - 14 &&
            event->pos().y() < size().height() - - 14)
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

        if (mResizeItemButtonRect.contains(event->pos())&&
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

        if (sClosePixmapButtonRect.contains(event->pos()))
        {
            event->accept();
            emit magnifierClose_Signal();
        }
        else
        if (sIncreasePixmapButtonRect.contains(event->pos()))
        {
            event->accept();
            emit magnifierZoomIn_Signal();
        }
        else
        if (sDecreasePixmapButtonRect.contains(event->pos()))
        {
            event->accept();
            emit magnifierZoomOut_Signal();
        }
        else
        if (sChangeModePixmapButtonRect.contains(event->pos()))
        {
            event->accept();
            emit magnifierDrawingModeChange_Signal(static_cast<int>(mDrawingMode+1)%modesCount);
        }
        else
            QWidget::mouseReleaseEvent(event); // don't propgate to parent, the widget is deleted in UBApplication::boardController->removeTool
    }
    else
        event->ignore();

}

void UBMagnifier::slot_refresh()
{
    if(!(updPointGrab.isNull()))
        grabPoint(updPointGrab);

    if(isCusrsorAlreadyStored)
    {
        QPoint globalCursorPos = QCursor::pos();
        QPoint cursorPos = mapFromGlobal(globalCursorPos);
        if (cursorPos.x() < size().width() - mResizeItem->width() - 14 ||
            cursorPos.x() > size().width() - 14 ||
            cursorPos.y() < size().height() - mResizeItem->height() - 14 ||
            cursorPos.y() > size().height() - - 14
            )
        {
            isCusrsorAlreadyStored = false;
            setCursor(mOldCursor);
        }
    }
}

void UBMagnifier::grabPoint()
{
    QMatrix transM = UBApplication::boardController->controlView()->matrix();
    QPointF itemPos = gView->mapFromGlobal(updPointGrab);

    qreal zWidth = width() / (params.zoom * transM.m11());
    qreal zWidthHalf = zWidth / 2;
    qreal zHeight = height() / (params.zoom * transM.m22());
    qreal zHeightHalf = zHeight / 2;


    QPointF pfScLtF(UBApplication::boardController->controlView()->mapToScene(QPoint(itemPos.x(), itemPos.y())));

    float x = pfScLtF.x() - zWidthHalf;
    float y = pfScLtF.y() - zHeightHalf;

    QPointF leftTop(x,y);
    QPointF rightBottom(x + zWidth, y + zHeight);
    QRectF srcRect(leftTop, rightBottom);

    QPixmap newPixMap(QSize(width(), height()));
    QPainter painter(&newPixMap);

    UBApplication::boardController->activeScene()->render(&painter, QRectF(0,0,width(),height()), srcRect);
    painter.end();

    pMap.fill(Qt::transparent);
    pMap = newPixMap.scaled(QSize(width(), height()));
    pMap.setMask(bmpMask);

    update();
}

void UBMagnifier::grabPoint(const QPoint &pGrab)
{
    QMatrix transM = UBApplication::boardController->controlView()->matrix();
    updPointGrab = pGrab;
    QPointF itemPos = gView->mapFromGlobal(pGrab);

    qreal zWidth = width() / (params.zoom * transM.m11());
    qreal zWidthHalf = zWidth / 2;
    qreal zHeight = height() / (params.zoom * transM.m22());
    qreal zHeightHalf = zHeight / 2;


    QPointF pfScLtF(UBApplication::boardController->controlView()->mapToScene(QPoint(itemPos.x(), itemPos.y())));

    float x = pfScLtF.x() - zWidthHalf;
    float y = pfScLtF.y() - zHeightHalf;

    QPointF leftTop(x,y);
    QPointF rightBottom(x + zWidth, y + zHeight);
    QRectF srcRect(leftTop, rightBottom);

    QPixmap newPixMap(QSize(width(), height()));
    QPainter painter(&newPixMap);

    UBApplication::boardController->activeScene()->render(&painter, QRectF(0,0,width(),height()), srcRect);
    painter.end();

   // pMap.fill(Qt::transparent);
    pMap = newPixMap;
    pMap.setMask(bmpMask);

    update();
}



// from global
void UBMagnifier::grabNMove(const QPoint &pGrab, const QPoint &pMove, bool needGrab, bool needMove)
{
    QPoint pointToGrab = pGrab;
    updPointGrab = pointToGrab;
    updPointMove = pMove;

    if(needGrab)
        grabPoint(pointToGrab);

    if(needMove)
    {
        QPoint movePos = mView->mapFromGlobal(pMove);
        move(movePos.x() - width()/2, movePos.y() - height()/2);
        //    move(itemPos.x(), itemPos.y());
    }
}

void UBMagnifier::setGrabView(QWidget *view)
{
    gView = view;
    mRefreshTimer.setInterval(40);
    mRefreshTimer.start();
}

void UBMagnifier::setDrawingMode(int mode)
{
    mDrawingMode = static_cast<DrawingMode>(mode);

    QString sMode;

    if (circular == mDrawingMode)
    {
        sMode = "roundeRrectangle";
        resize(width(), width());
    }

    if (rectangular == mDrawingMode)
    {
        sMode = "circle";
        resize(width(), height()/3);

        if (mView)
        {
            qreal newPercentSize = size().width()/3 * 100 / mView->width();
            emit magnifierResized_Signal(newPercentSize);
        }
    }

    sChangeModePixmap->load(":/images/"+sMode+".svg");

    calculateButtonsPositions();
    if (mView && gView)
        UBApplication::boardController->controlView()->scene()->moveMagnifier();

    createMask();

    UBSettings::settings()->magnifierDrawingMode->set(mode);
}
