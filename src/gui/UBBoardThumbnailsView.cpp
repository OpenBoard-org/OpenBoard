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

#include "UBBoardThumbnailsView.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBPersistenceManager.h"

#include "document/UBDocument.h"

#include "gui/UBThumbnail.h"
#include "gui/UBThumbnailArranger.h"
#include "gui/UBThumbnailScene.h"

UBBoardThumbnailsView::UBBoardThumbnailsView(QWidget *parent, const char *name)
    : UBThumbnailsView(parent)
    , mThumbnailWidth(0)
    , mThumbnailMinWidth(100)
    , mMargin(20)
    , mDropSource(NULL)
    , mDropTarget(NULL)
    , mDropBar(new QGraphicsRectItem(0))
    , mLongPressInterval(350)
{
    setThumbnailArranger(new UBBoardThumbnailArranger(this));

    mDropBar->setPen(QPen(Qt::darkGray));
    mDropBar->setBrush(QBrush(Qt::lightGray));

    setObjectName(name);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setFrameShadow(QFrame::Plain);

    mThumbnailWidth = width() - 2*mMargin;

    mLongPressTimer.setInterval(mLongPressInterval);
    mLongPressTimer.setSingleShot(true);

    connect(&mLongPressTimer, SIGNAL(timeout()), this, SLOT(longPressTimeout()));

    connect(this, SIGNAL(mousePressAndHoldEventRequired(QPoint)), this, SLOT(mousePressAndHoldEvent(QPoint)));

    connect(UBApplication::boardController, SIGNAL(pageSelectionChanged(int)), this, SLOT(updateActiveThumbnail(int)));

    connect(UBApplication::boardController->controlView(), &UBBoardView::mouseReleased, this, &UBBoardThumbnailsView::adjustThumbnail);

    connect(UBApplication::boardController->controlView(), &UBBoardView::painted, this, &UBBoardThumbnailsView::updateThumbnailPixmap);

    connect(UBApplication::boardController, &UBDocumentContainer::documentSet, this, [this](std::shared_ptr<UBDocumentProxy> proxy){
        setDocument(UBDocument::getDocument(proxy));
    });
}

void UBBoardThumbnailsView::setDocument(std::shared_ptr<UBDocument> document)
{
    mDocument = document;

    if (document)
    {
        auto thumbnailScene = document->thumbnailScene();
        setScene(thumbnailScene);
        thumbnailScene->arrangeThumbnails();
        thumbnailScene->addItem(mDropBar);
    }
    else
    {
        if (mDropBar->scene())
        {
            mDropBar->scene()->removeItem(mDropBar);
        }

        setScene(nullptr);
    }
}


void UBBoardThumbnailsView::adjustThumbnail()
{
    auto thumbnail = mDocument->thumbnailScene()->thumbnailAt(mCurrentIndex);

    if (thumbnail)
    {
        thumbnail->adjustThumbnail();
    }
}


void UBBoardThumbnailsView::centerOnThumbnail(int index)
{
    auto thumbnail = mDocument->thumbnailScene()->thumbnailAt(index);

    if (thumbnail)
    {
        centerOn(thumbnail);
    }
}

void UBBoardThumbnailsView::ensureVisibleThumbnail(int index)
{
    auto previousThumbnail = mDocument->thumbnailScene()->thumbnailAt(mCurrentIndex);

    if (previousThumbnail && index != mCurrentIndex)
    {
        previousThumbnail->setPageScene(nullptr);
    }

    mCurrentIndex = index;

    auto currentThumbnail = mDocument->thumbnailScene()->thumbnailAt(mCurrentIndex);

    if (currentThumbnail)
    {
        std::shared_ptr<UBGraphicsScene> pageScene = UBPersistenceManager::persistenceManager()->getDocumentScene(mDocument->proxy(), index);
        currentThumbnail->setPageScene(pageScene);
        ensureVisible(currentThumbnail);
    }
}

void UBBoardThumbnailsView::updateActiveThumbnail(int newActiveIndex)
{
    mDocument->thumbnailScene()->hightlightItem(newActiveIndex, true);

    ensureVisibleThumbnail(newActiveIndex);

    update();
}

void UBBoardThumbnailsView::resizeEvent(QResizeEvent *event)
{
    bool scrollbarWasHidden = mScrollbarVisible && !verticalScrollBar()->isVisible();

    // Refresh the scene, except if resizing because scrollbar was hidden
    if (event->size().width() > 0 && !scrollbarWasHidden && mDocument)
    {
        mDocument->thumbnailScene()->arrangeThumbnails();
        ensureVisibleThumbnail(UBApplication::boardController->activeSceneIndex());
    }

    mScrollbarVisible = verticalScrollBar()->isVisible();

    QGraphicsView::resizeEvent(event);
}

void UBBoardThumbnailsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    mLastPressedMousePos = event->pos();

    UBThumbnail* item = dynamic_cast<UBThumbnail*>(itemAt(event->pos()));

    if (item && item->sceneIndex() != UBApplication::boardController->activeSceneIndex())
    {
        UBApplication::boardController->persistViewPositionOnCurrentScene();
        UBApplication::boardController->persistCurrentScene();
        UBApplication::boardController->setActiveDocumentScene(item->sceneIndex());
    }

    mLongPressTimer.start();
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
    UBThumbnail* item = dynamic_cast<UBThumbnail*>(itemAt(pos));
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
    const int index = UBApplication::boardController->activeSceneIndex();
    auto thumbnail = mDocument->thumbnailScene()->thumbnailAt(index);

    if (thumbnail)
    {
        thumbnail->updatePixmap(region);
    }
}

bool UBBoardThumbnailsView::event(QEvent* event)
{
    if (event->type() == QEvent::Show && mDocument)
    {
        mDocument->thumbnailScene()->arrangeThumbnails();
    }

    return UBThumbnailsView::event(event);
}

void UBBoardThumbnailsView::mouseReleaseEvent(QMouseEvent *event)
{
    mLongPressTimer.stop();

    QGraphicsView::mouseReleaseEvent(event);
}

void UBBoardThumbnailsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
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

    UBThumbnail* item = dynamic_cast<UBThumbnail*>(itemAt(eventPos));

    if (item)
    {
        mDropTarget = item;

        QPointF itemCenter{item->sceneBoundingRect().center()};

        bool dropAbove = mapToScene(eventPos).y() < itemCenter.y();
        bool movingUp = mDropSource->sceneIndex() > item->sceneIndex();

        if (movingUp == dropAbove)
        {
            const auto halfSpacingHeight = thumbnailArranger()->spacing().height() / 2;
            const auto offset = dropAbove ? -halfSpacingHeight : UBThumbnail::heightForWidth(thumbnailArranger()->thumbnailWidth()) + halfSpacingHeight;
            const auto y = item->pos().y() + offset - 1;

            if (mDropBar->y() != y)
            {
                mDropBar->setRect(QRectF{item->pos().x(), y, thumbnailArranger()->thumbnailWidth(), 3});
            }
        }
    }

    event->acceptProposedAction();
}

void UBBoardThumbnailsView::dropEvent(QDropEvent *event)
{
    Q_UNUSED(event);

    if (mDropSource->sceneIndex() != mDropTarget->sceneIndex())
        UBApplication::boardController->moveSceneToIndex(mDocument->proxy(), mDropSource->sceneIndex(), mDropTarget->sceneIndex());

    mDropSource = NULL;
    mDropTarget = NULL;

    mDropBar->setRect(QRectF());
    mDropBar->hide();
}

/* ---- UBBoardThumbnailArranger ---- */

UBBoardThumbnailsView::UBBoardThumbnailArranger::UBBoardThumbnailArranger(UBBoardThumbnailsView* thumbnailView)
    : UBThumbnailArranger(thumbnailView)
{
}

int UBBoardThumbnailsView::UBBoardThumbnailArranger::columnCount() const
{
    return 1;
}

double UBBoardThumbnailsView::UBBoardThumbnailArranger::thumbnailWidth() const
{
    return availableViewWidth();
}
