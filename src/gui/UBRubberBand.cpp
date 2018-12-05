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




#include "UBRubberBand.h"

#include <QtGui>
#include <QStyleFactory>
#include <QStyle>

#include "board/UBBoardView.h"

#include "core/memcheck.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

UBRubberBand::UBRubberBand(Shape s, QWidget * p)
    : QRubberBand(s, p)
    , mResizingMode(None)
    , mResizingBorderHeight(20)
    , mMouseIsPressed(false)
    , mLastPressedPoint(QPoint())
{
    customStyle = NULL;

#ifdef Q_OS_WIN
    customStyle = QStyleFactory::create("windows");
#elif defined(Q_OS_OSX)
    customStyle = QStyleFactory::create("macintosh");
#elif defined(Q_OS_LINUX)
    customStyle = QStyleFactory::create("oxygen");
#endif

    if (customStyle)
        QRubberBand::setStyle(customStyle);

    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);
}

UBRubberBand::~UBRubberBand()
{
    if (customStyle)
        delete customStyle;
}

UBRubberBand::enm_resizingMode UBRubberBand::determineResizingMode(QPoint pos)
{
    if (mMouseIsPressed)
        return mResizingMode;
    
    QRect resizerTop    (mResizingBorderHeight               , 0                             , rect().width()-2*mResizingBorderHeight, mResizingBorderHeight                    );
    QRect resizerBottom (mResizingBorderHeight               , rect().height() - mResizingBorderHeight, rect().width()-2*mResizingBorderHeight, mResizingBorderHeight                    );
    QRect resizerLeft   (0                          , mResizingBorderHeight                  , mResizingBorderHeight                 , rect().height() - 2*mResizingBorderHeight);
    QRect resizerRight  (rect().width()-mResizingBorderHeight, mResizingBorderHeight                  , mResizingBorderHeight                 , rect().height() - 2*mResizingBorderHeight);

    QRect resizerTopLeft    (0                          , 0                             , mResizingBorderHeight, mResizingBorderHeight);
    QRect resizerTopRight   (rect().width()-mResizingBorderHeight, 0                             , mResizingBorderHeight, mResizingBorderHeight);
    QRect resizerBottomLeft (0                          , rect().height() - mResizingBorderHeight, mResizingBorderHeight, mResizingBorderHeight);
    QRect resizerBottomRight(rect().width()-mResizingBorderHeight, rect().height() - mResizingBorderHeight, mResizingBorderHeight, mResizingBorderHeight);

    enm_resizingMode resizingMode;
    
    QTransform cursorTransrofm;

    if (resizerTop.contains(pos))
    {
        resizingMode = Top;
        cursorTransrofm.rotate(90);
    }
    else
    if (resizerBottom.contains(pos))
    {
        resizingMode = Bottom;
        cursorTransrofm.rotate(90);
    }
    else
    if (resizerLeft.contains(pos))
    {
        resizingMode = Left;
    }
    else
    if (resizerRight.contains(pos))
    {
        resizingMode = Right;
    }
    else
    if (resizerTopLeft.contains(pos))
    {
        resizingMode = TopLeft;
        cursorTransrofm.rotate(45);
    }
    else
    if (resizerTopRight.contains(pos))
    {
        resizingMode = TopRight;
        cursorTransrofm.rotate(-45);
    }
    else
    if (resizerBottomLeft.contains(pos))
    {
        resizingMode = BottomLeft;
        cursorTransrofm.rotate(-45);
    }
    else
    if (resizerBottomRight.contains(pos))
    {
        resizingMode = BottomRight;
        cursorTransrofm.rotate(45);
    }
    else
        resizingMode = None;
    
    if (None != resizingMode)
    {
        QPixmap pix(":/images/cursors/resize.png");
        QCursor resizeCursor  = QCursor(pix.transformed(cursorTransrofm, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
        setCursor(resizeCursor);
    }
    else
        unsetCursor();

    return resizingMode;
}

void UBRubberBand::mousePressEvent(QMouseEvent *event)
{
    mResizingMode = determineResizingMode(event->pos());
    mMouseIsPressed = true;

    mLastPressedPoint = event->pos();
    mLastMousePos = event->pos();

    if (None == mResizingMode)
    {
        UBApplication::boardController->controlView()->rubberItems();
        setCursor(QCursor(Qt::SizeAllCursor));
    }
}

void UBRubberBand::mouseMoveEvent(QMouseEvent *event)
{
    determineResizingMode(event->pos());

    if (mMouseIsPressed)
    {
        UBBoardView *view = UBApplication::boardController->controlView();
        QRect currentGeometry = geometry();

        QPoint pressPoint(event->pos());
        QPoint pressPointGlobal(view->mapToGlobal(pressPoint));
        QPoint prevPressPointGlobal(view->mapToGlobal(mLastPressedPoint));
        QPoint movePointGlogal = (view->mapToGlobal(mLastMousePos));

        QPoint topLeftResizeVector(pressPointGlobal - prevPressPointGlobal);
        QPoint rightBottomResizeVector(pressPointGlobal - movePointGlogal);

        bool bGeometryChange = true;

        switch(mResizingMode)
        {
        case None:
            {
                QPointF itemsMoveVector(view->mapToScene(pressPointGlobal) - view->mapToScene(prevPressPointGlobal));

                move(pos()+pressPointGlobal - prevPressPointGlobal);
                view->moveRubberedItems(itemsMoveVector);

                bGeometryChange = false;
                break;
            }

        case Top:
            {
                currentGeometry.setY(currentGeometry.y()+topLeftResizeVector.y());
            }break;

        case Bottom:
            {
                currentGeometry.setHeight(currentGeometry.height()+rightBottomResizeVector.y());
            }break;

        case Left:
            {
                currentGeometry.setX(currentGeometry.x()+topLeftResizeVector.x());
            }break;
        case Right:
            {
                currentGeometry.setWidth(currentGeometry.width()+rightBottomResizeVector.x());
            }break;

        case TopLeft:
            {
                currentGeometry.setX(currentGeometry.x()+topLeftResizeVector.x());  
                currentGeometry.setY(currentGeometry.y()+topLeftResizeVector.y());
            }break;

        case TopRight:
            {
                currentGeometry.setY(currentGeometry.y()+topLeftResizeVector.y());  
                currentGeometry.setWidth(currentGeometry.width()+rightBottomResizeVector.x());                
            }
            break;

        case BottomLeft:
            {
                currentGeometry.setX(currentGeometry.x()+topLeftResizeVector.x());
                currentGeometry.setHeight(currentGeometry.height()+rightBottomResizeVector.y());  
            }break;

        case BottomRight:
            {
                currentGeometry.setWidth(currentGeometry.width()+rightBottomResizeVector.x());
                currentGeometry.setHeight(currentGeometry.height()+rightBottomResizeVector.y());        
            }break;
        
        }
        if(bGeometryChange)
        {   
            setGeometry(currentGeometry);
        }

        mLastMousePos = event->pos();
    }

    QRubberBand::mouseMoveEvent(event);
} 

void UBRubberBand::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    mMouseIsPressed = false;
    unsetCursor();
}
