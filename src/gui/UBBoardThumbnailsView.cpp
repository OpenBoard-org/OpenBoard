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




#include <QList>
#include <QPointF>
#include <QPixmap>
#include <QTransform>
#include <QScrollBar>
#include <QFontMetrics>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>

#include "core/UBApplication.h"
#include "UBBoardThumbnailsView.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "document/UBDocumentController.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBBoardPaletteManager.h"
#include "core/UBApplicationController.h"
#include "core/UBPersistenceManager.h"
#include "UBThumbnailView.h"
#include "UBThumbnailWidget.h"

UBBoardThumbnailsView::UBBoardThumbnailsView(QWidget *parent, const char *name)
    : QGraphicsView(parent)
    , mThumbnailWidth(0)
    , mThumbnailMinWidth(100)
    , mMargin(20)
    , mDropSource(NULL)
    , mDropTarget(NULL)
    , mDropBar(new QGraphicsRectItem(0))
    , mLongPressInterval(350)
{
    setScene(new QGraphicsScene(this));    

    mDropBar->setPen(QPen(Qt::darkGray));
    mDropBar->setBrush(QBrush(Qt::lightGray));
    scene()->addItem(mDropBar);
    mDropBar->hide();

    setObjectName(name);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShadow(QFrame::Plain);

    mThumbnailWidth = width() - 2*mMargin;

    mLongPressTimer.setInterval(mLongPressInterval);
    mLongPressTimer.setSingleShot(true);

    connect(UBApplication::boardController, SIGNAL(initThumbnailsRequired(UBDocumentProxy*)), this, SLOT(initThumbnails(UBDocumentProxy*)));
    connect(UBApplication::boardController, SIGNAL(addThumbnailRequired(UBDocumentProxy*, int)), this, SLOT(addThumbnail(UBDocumentProxy*, int)));
    connect(UBApplication::boardController, SIGNAL(moveThumbnailRequired(int, int)), this, SLOT(moveThumbnail(int, int)));
    connect(this, SIGNAL(moveThumbnailRequired(int, int)), this, SLOT(moveThumbnail(int, int)));
    connect(UBApplication::boardController, SIGNAL(updateThumbnailsRequired()), this, SLOT(updateThumbnails()));
    connect(UBApplication::boardController, SIGNAL(removeThumbnailRequired(int)), this, SLOT(removeThumbnail(int)));

    connect(&mLongPressTimer, SIGNAL(timeout()), this, SLOT(longPressTimeout()), Qt::UniqueConnection);

    connect(this, SIGNAL(mousePressAndHoldEventRequired(QPoint)), this, SLOT(mousePressAndHoldEvent(QPoint)), Qt::UniqueConnection);

    connect(UBApplication::boardController, SIGNAL(pageSelectionChanged(int)), this, SLOT(ensureVisibleThumbnail(int)), Qt::UniqueConnection);
    connect(UBApplication::boardController, SIGNAL(centerOnThumbnailRequired(int)), this, SLOT(centerOnThumbnail(int)), Qt::UniqueConnection);

    connect(UBApplication::boardController->controlView(), &UBBoardView::painted, this, &UBBoardThumbnailsView::updateThumbnailPixmap);
}

void UBBoardThumbnailsView::moveThumbnail(int from, int to)
{
    mThumbnails.move(from, to);

    updateThumbnailsPos();
}

void UBBoardThumbnailsView::updateThumbnails()
{
    updateThumbnailsPos();
}

void UBBoardThumbnailsView::removeThumbnail(int i)
{
    UBDraggableLivePixmapItem* item = mThumbnails.at(i);

    scene()->removeItem(item->selectionItem());
    scene()->removeItem(item->pageNumber());
    scene()->removeItem(item);

    mThumbnails.removeAt(i);

    updateThumbnailsPos();
}

UBDraggableLivePixmapItem* UBBoardThumbnailsView::createThumbnail(UBDocumentProxy* document, int i)
{
    UBApplication::showMessage(tr("Loading page (%1/%2)").arg(i+1).arg(document->pageCount()));

    UBGraphicsScene* pageScene = UBPersistenceManager::persistenceManager()->loadDocumentScene(document, i);

    return new UBDraggableLivePixmapItem(pageScene, document, i);
}

void UBBoardThumbnailsView::addThumbnail(UBDocumentProxy* document, int i)
{
    UBDraggableLivePixmapItem* item = createThumbnail(document, i);
    mThumbnails.insert(i, item);

    scene()->addItem(item);
    scene()->addItem(item->pageNumber());
    scene()->addItem(item->selectionItem());

    updateThumbnailsPos();
}

void UBBoardThumbnailsView::clearThumbnails()
{
    for(int i = 0; i < mThumbnails.size(); i++)
    {
        scene()->removeItem(mThumbnails.at(i)->selectionItem());
        scene()->removeItem(mThumbnails.at(i)->pageNumber());
        scene()->removeItem(mThumbnails.at(i));
        mThumbnails.at(i)->deleteLater();
    }

    mThumbnails.clear();
}

void UBBoardThumbnailsView::initThumbnails(UBDocumentProxy* document)
{
    clearThumbnails();

    for(int i = 0; i < document->pageCount(); i++)
    {
        mThumbnails.append(createThumbnail(document, i));

        scene()->addItem(mThumbnails.last());
        scene()->addItem(mThumbnails.last()->pageNumber());
        scene()->addItem(mThumbnails.last()->selectionItem());
    }

    updateThumbnailsPos();
}

void UBBoardThumbnailsView::centerOnThumbnail(int index)
{
    centerOn(mThumbnails.at(index));
}

void UBBoardThumbnailsView::ensureVisibleThumbnail(int index)
{
    ensureVisible(mThumbnails.at(index));
}

void UBBoardThumbnailsView::updateThumbnailsPos()
{    
    qreal thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;

    // for some reason, verticalScrollBar()->width() returns 100 while isVisible() is false... not the case with Qt 5.5 (when this code has been implemented)
    int verticalScrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
    scene()->setSceneRect(0, 0, scene()->itemsBoundingRect().size().width() - verticalScrollBarWidth, scene()->itemsBoundingRect().size().height());

    for (int i=0; i < mThumbnails.length(); i++)
    {
        mThumbnails.at(i)->setSceneIndex(i);
        mThumbnails.at(i)->setPageNumber(i);
        mThumbnails.at(i)->setHighlighted(i == UBApplication::boardController->activeSceneIndex());
        mThumbnails.at(i)->updatePos(mThumbnailWidth, thumbnailHeight);
    }

    updateExposure();
    update();
}

void UBBoardThumbnailsView::updateExposure()
{
    QRect viewportRect(QPoint(0, 0), viewport()->size());
    QRectF visibleSceneRect = mapToScene(viewportRect).boundingRect();

    for (UBDraggableLivePixmapItem* thumbnail : qAsConst(mThumbnails))
    {
        thumbnail->setExposed(visibleSceneRect.intersects(thumbnail->sceneBoundingRect()));
    }
}

void UBBoardThumbnailsView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // Update the thumbnails width

    int verticalScrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;

    mThumbnailWidth = std::max(width() - verticalScrollBarWidth - 2*mMargin, mThumbnailMinWidth);

    // Refresh the scene
    updateThumbnailsPos();

    emit UBApplication::boardController->centerOnThumbnailRequired(UBApplication::boardController->activeSceneIndex());
}

void UBBoardThumbnailsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    if (!event->isAccepted())
    {
        mLastPressedMousePos = event->pos();

        UBDraggableLivePixmapItem* item = dynamic_cast<UBDraggableLivePixmapItem*>(itemAt(event->pos()));

        if (item)
        {
            UBApplication::boardController->persistViewPositionOnCurrentScene();
            UBApplication::boardController->persistCurrentScene();
            UBApplication::boardController->setActiveDocumentScene(item->sceneIndex());
            UBApplication::boardController->centerOn(UBApplication::boardController->activeScene()->lastCenter());
        }

        mLongPressTimer.start();
    }
}

void UBBoardThumbnailsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void UBBoardThumbnailsView::longPressTimeout()
{
    if (QApplication::mouseButtons() != Qt::NoButton)
        emit mousePressAndHoldEventRequired(mLastPressedMousePos);

    mLongPressTimer.stop();
}

void UBBoardThumbnailsView::mousePressAndHoldEvent(QPoint pos)
{
    UBDraggableLivePixmapItem* item = dynamic_cast<UBDraggableLivePixmapItem*>(itemAt(pos));
    if (item)
    {
        mDropSource = item;
        mDropTarget = item;

        QPixmap pixmap = item->pixmap().scaledToWidth(mThumbnailWidth/2);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(new QMimeData());
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));

        drag->exec();
    }
}

void UBBoardThumbnailsView::updateThumbnailPixmap(const QRectF region)
{
    int index = UBApplication::boardController->activeSceneIndex();
    mThumbnails.at(index)->updatePixmap(region);
}

void UBBoardThumbnailsView::mouseReleaseEvent(QMouseEvent *event)
{
    mLongPressTimer.stop();

    QGraphicsView::mouseReleaseEvent(event);
}

void UBBoardThumbnailsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    updateExposure();
}

void UBBoardThumbnailsView::dragEnterEvent(QDragEnterEvent *event)
{
    mDropBar->show();

    if (event->source() == this)
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->acceptProposedAction();
    }
}

void UBBoardThumbnailsView::dragMoveEvent(QDragMoveEvent *event)
{        
    QPointF position = event->pos();

    //autoscroll during drag'n'drop
    QPointF scenePos = mapToScene(position.toPoint());
    int thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;
    QRectF thumbnailArea(0, scenePos.y() - thumbnailHeight/2, mThumbnailWidth, thumbnailHeight);

    ensureVisible(thumbnailArea);

    UBDraggableLivePixmapItem* item = dynamic_cast<UBDraggableLivePixmapItem*>(itemAt(position.toPoint()));
    if (item)
    {
        if (item != mDropTarget)
            mDropTarget = item;

        qreal scale = item->transform().m11();

        int verticalScrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;

        QPointF itemCenter(item->pos().x() + (item->boundingRect().width()-verticalScrollBarWidth) * scale,
                           item->pos().y() + item->boundingRect().height() * scale / 2);

        bool dropAbove = mapToScene(position.toPoint()).y() < itemCenter.y();
        bool movingUp = mDropSource->sceneIndex() > item->sceneIndex();
        qreal y = 0;

        if (movingUp)
        {
            if (dropAbove)
            {
                y = item->pos().y() - UBSettings::thumbnailSpacing / 2;
                if (mDropBar->y() != y)
                    mDropBar->setRect(QRectF(item->pos().x(), y, (item->boundingRect().width()-verticalScrollBarWidth)*scale, 3));
            }
        }
        else
        {
            if (!dropAbove)
            {
                y = item->pos().y() + item->boundingRect().height() * scale + UBSettings::thumbnailSpacing / 2;
                if (mDropBar->y() != y)
                    mDropBar->setRect(QRectF(item->pos().x(), y, (item->boundingRect().width()-verticalScrollBarWidth)*scale, 3));
            }
        }
    }

    event->acceptProposedAction();
}

void UBBoardThumbnailsView::dropEvent(QDropEvent *event)
{
    Q_UNUSED(event);

    if (mDropSource->sceneIndex() != mDropTarget->sceneIndex())
        UBApplication::boardController->moveSceneToIndex(mDropSource->sceneIndex(), mDropTarget->sceneIndex());

    mDropSource = NULL;
    mDropTarget = NULL;

    mDropBar->setRect(QRectF());
    mDropBar->hide();
}
