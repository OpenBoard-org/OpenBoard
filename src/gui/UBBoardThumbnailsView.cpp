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
#include "gui/UBDocumentThumbnailsView.h"

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

    mUpdateThumbnailsTimer.setSingleShot(true);

    connect(UBApplication::boardController, SIGNAL(initThumbnailsRequired(std::shared_ptr<UBDocumentProxy>)), this, SLOT(initThumbnails(std::shared_ptr<UBDocumentProxy>)));
    connect(UBApplication::boardController, SIGNAL(addThumbnailRequired(std::shared_ptr<UBDocumentProxy>, int)), this, SLOT(addThumbnail(std::shared_ptr<UBDocumentProxy>, int)));
    connect(UBApplication::boardController, SIGNAL(moveThumbnailRequired(int, int)), this, SLOT(moveThumbnail(int, int)));
    connect(this, SIGNAL(moveThumbnailRequired(int, int)), this, SLOT(moveThumbnail(int, int)));
    connect(UBApplication::boardController, SIGNAL(updateThumbnailsRequired()), this, SLOT(updateThumbnails()));
    connect(UBApplication::boardController, SIGNAL(removeThumbnailRequired(int)), this, SLOT(removeThumbnail(int)));

    connect(&mLongPressTimer, SIGNAL(timeout()), this, SLOT(longPressTimeout()));
    connect(&mUpdateThumbnailsTimer, SIGNAL(timeout()), this, SLOT(updateThumbnailsPos()));

    connect(this, SIGNAL(mousePressAndHoldEventRequired(QPoint)), this, SLOT(mousePressAndHoldEvent(QPoint)));

    connect(UBApplication::boardController, SIGNAL(pageSelectionChanged(int)), this, SLOT(updateActiveThumbnail(int)));
    connect(UBApplication::boardController, SIGNAL(centerOnThumbnailRequired(int)), this, SLOT(centerOnThumbnail(int)));

    connect(UBApplication::boardController->controlView(), &UBBoardView::mouseReleased, this, &UBBoardThumbnailsView::adjustThumbnail);

    connect(UBApplication::boardController->controlView(), &UBBoardView::painted, this, &UBBoardThumbnailsView::updateThumbnailPixmap);
}

void UBBoardThumbnailsView::moveThumbnail(int from, int to)
{
    mThumbnails.move(from, to);

    updateThumbnails();
}

void UBBoardThumbnailsView::updateThumbnails()
{
    // Update the thumbnails width
    int verticalScrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;

    mThumbnailWidth = std::max(width() - verticalScrollBarWidth - 2*mMargin, mThumbnailMinWidth);

    mUpdateThumbnailsTimer.setInterval(std::min(std::min(100, (int)mThumbnails.size()), 1000));
    mUpdateThumbnailsTimer.start();
}

void UBBoardThumbnailsView::adjustThumbnail()
{
    if (mCurrentIndex >= 0 && mCurrentIndex < mThumbnails.size())
    {
        mThumbnails.at(mCurrentIndex)->adjustThumbnail();
    }
}

void UBBoardThumbnailsView::removeThumbnail(int i)
{
    UBDraggableLivePixmapItem* item = mThumbnails.at(i);

    scene()->removeItem(item->selectionItem());
    scene()->removeItem(item->pageNumber());
    scene()->removeItem(item);

    mThumbnails.removeAt(i);

    updateThumbnails();
}

UBDraggableLivePixmapItem* UBBoardThumbnailsView::createThumbnail(std::shared_ptr<UBDocumentProxy> document, int i)
{
    QPixmap thumbnail = UBThumbnailAdaptor::get(document, i);

    return new UBDraggableLivePixmapItem(nullptr, document, i, thumbnail);
}

void UBBoardThumbnailsView::addThumbnail(std::shared_ptr<UBDocumentProxy> document, int i)
{
    UBDraggableLivePixmapItem* item = createThumbnail(document, i);
    mThumbnails.insert(i, item);

    scene()->addItem(item);
    scene()->addItem(item->pageNumber());
    scene()->addItem(item->selectionItem());

    item->updatePos(mThumbnailWidth, mThumbnailWidth / UBSettings::minScreenRatio);

    updateThumbnails();
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

void UBBoardThumbnailsView::initThumbnails(std::shared_ptr<UBDocumentProxy> document)
{
    if (document)
    {
        clearThumbnails();

        // Update the thumbnails width
        int verticalScrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;

        mThumbnailWidth = std::max(width() - verticalScrollBarWidth - 2*mMargin, mThumbnailMinWidth);

        for(int i = 0; i < document->pageCount(); i++)
        {
            mThumbnails.append(createThumbnail(document, i));

            scene()->addItem(mThumbnails.last());
            scene()->addItem(mThumbnails.last()->pageNumber());
            scene()->addItem(mThumbnails.last()->selectionItem());

            mThumbnails.last()->updatePos(mThumbnailWidth, mThumbnailWidth / UBSettings::minScreenRatio);
        }

        updateActiveThumbnail(0);

        updateThumbnails();
    }
}

void UBBoardThumbnailsView::centerOnThumbnail(int index)
{
    if (index < mThumbnails.size())
    {
        centerOn(mThumbnails.at(index));
    }
}

void UBBoardThumbnailsView::ensureVisibleThumbnail(int index)
{
    if (mCurrentIndex >= 0 && mCurrentIndex < mThumbnails.size())
    {
        // detach scene from previous thumbnail
        mThumbnails.at(mCurrentIndex)->setScene(nullptr);
    }

    mCurrentIndex = index;

    if (index >= 0 && index < mThumbnails.size())
    {
        auto thumbnail = mThumbnails.at(index);
        std::shared_ptr<UBGraphicsScene> pageScene = UBPersistenceManager::persistenceManager()->getDocumentScene(thumbnail->documentProxy(), index);
        thumbnail->setScene(pageScene);
        ensureVisible(thumbnail);
    }
}

void UBBoardThumbnailsView::updateActiveThumbnail(int newActiveIndex)
{
    for (int i=0; i < mThumbnails.length(); i++)
    {
        mThumbnails.at(i)->setHighlighted(i == newActiveIndex);
    }

    ensureVisibleThumbnail(newActiveIndex);

    mThumbnails.at(newActiveIndex)->updatePos(mThumbnailWidth, mThumbnailWidth / UBSettings::minScreenRatio);

    update();
}

void UBBoardThumbnailsView::updateThumbnailsPos()
{
    if (isVisible())
    {
        qreal thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;

        for (int i=0; i < mThumbnails.length(); i++)
        {
            mThumbnails.at(i)->setSceneIndex(i);
            mThumbnails.at(i)->setPageNumber(i);
            mThumbnails.at(i)->setHighlighted(i == UBApplication::boardController->activeSceneIndex());
            mThumbnails.at(i)->updatePos(mThumbnailWidth, thumbnailHeight);
        }

        // for some reason, verticalScrollBar()->width() returns 100 while isVisible() is false... not the case with Qt 5.5 (when this code has been implemented)
        int verticalScrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
        setSceneRect(0, 0, scene()->itemsBoundingRect().size().width() - verticalScrollBarWidth, scene()->itemsBoundingRect().size().height());

        updateExposure();

        ensureVisibleThumbnail(UBApplication::boardController->activeSceneIndex());

        update();
    }
}

void UBBoardThumbnailsView::updateExposure()
{
    QRect viewportRect(QPoint(0, 0), viewport()->size());
    QRectF visibleSceneRect = mapToScene(viewportRect).boundingRect();

    for (UBDraggableLivePixmapItem* thumbnail : std::as_const(mThumbnails))
    {
        thumbnail->setExposed(visibleSceneRect.intersects(thumbnail->sceneBoundingRect()));
    }
}

void UBBoardThumbnailsView::resizeEvent(QResizeEvent *event)
{
    int verticalScrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;

    // Refresh the scene
    if (event->size().width() > 0 && std::abs(event->oldSize().width() - event->size().width()) != verticalScrollBarWidth)
    {
        updateThumbnails();

        ensureVisibleThumbnail(UBApplication::boardController->activeSceneIndex());
    }

    QGraphicsView::resizeEvent(event);
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
    if (mThumbnails.size() > index)
    {
        mThumbnails.at(index)->updatePixmap(region);
    }
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
}

void UBBoardThumbnailsView::dragMoveEvent(QDragMoveEvent *event)
{        
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QPoint eventPos = event->position().toPoint();
#else
    QPoint eventPos = event->pos();
#endif

    //autoscroll during drag'n'drop
    QPointF scenePos = mapToScene(eventPos);
    int thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;
    QRectF thumbnailArea(0, scenePos.y() - thumbnailHeight/2, mThumbnailWidth, thumbnailHeight);

    ensureVisible(thumbnailArea);

    UBDraggableLivePixmapItem* item = dynamic_cast<UBDraggableLivePixmapItem*>(itemAt(eventPos));
    if (item)
    {
        if (item != mDropTarget)
            mDropTarget = item;

        qreal scale = item->transform().m11();

        int verticalScrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;

        QPointF itemCenter(item->pos().x() + (item->boundingRect().width()-verticalScrollBarWidth) * scale,
                           item->pos().y() + item->boundingRect().height() * scale / 2);

        bool dropAbove = mapToScene(eventPos).y() < itemCenter.y();
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
