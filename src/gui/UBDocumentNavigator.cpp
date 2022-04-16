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




#include <QList>
#include <QPointF>
#include <QPixmap>
#include <QTransform>
#include <QScrollBar>
#include <QFontMetrics>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>

#include "core/UBApplication.h"
#include "UBDocumentNavigator.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "document/UBDocumentController.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBBoardPaletteManager.h"
#include "core/UBApplicationController.h"

#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBDocumentNavigator::UBDocumentNavigator(QWidget *parent, const char *name):QGraphicsView(parent)
  , mScene(NULL)
  , mNbColumns(1)
  , mThumbnailWidth(0)
  , mThumbnailMinWidth(100)
  , mSelectedThumbnail(NULL)
  , mLastClickedThumbnail(NULL)
  , mDropSource(NULL)
  , mDropTarget(NULL)
  , mDropBar(new QGraphicsRectItem())
  , mLongPressInterval(350)
{
    setObjectName(name);
    mScene = new QGraphicsScene(this);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(mScene);
    mThumbnailWidth = width() - 2*border();

    mDropBar->setPen(QPen(Qt::darkGray));
    mDropBar->setBrush(QBrush(Qt::lightGray));
    scene()->addItem(mDropBar);
    mDropBar->hide();

    mLongPressTimer.setInterval(mLongPressInterval);
    mLongPressTimer.setSingleShot(true);

    setFrameShadow(QFrame::Plain);

    connect(UBApplication::boardController, SIGNAL(documentThumbnailsUpdated(UBDocumentContainer*)), this, SLOT(generateThumbnails(UBDocumentContainer*)));
    connect(UBApplication::boardController, SIGNAL(documentPageUpdated(int)), this, SLOT(updateSpecificThumbnail(int)));
    connect(UBApplication::boardController, SIGNAL(pageSelectionChanged(int)), this, SLOT(onScrollToSelectedPage(int)));

    connect(&mLongPressTimer, SIGNAL(timeout()), this, SLOT(longPressTimeout()), Qt::UniqueConnection);

    connect(this, SIGNAL(mousePressAndHoldEventRequired()), this, SLOT(mousePressAndHoldEvent()), Qt::UniqueConnection);
}

/**
 * \brief Destructor
 */
UBDocumentNavigator::~UBDocumentNavigator()
{
    if(NULL != mScene)
    {
        delete mScene;
        mScene = NULL;
    }
}

/**
 * \brief Generate the thumbnails
 */
void UBDocumentNavigator::generateThumbnails(UBDocumentContainer* source)
{
    mThumbsWithLabels.clear();
    int selectedIndex = -1;
    int lastClickedIndex = -1;
    if (mLastClickedThumbnail)
    {
        lastClickedIndex = mLastClickedThumbnail->sceneIndex();
    }

    QList<QGraphicsItem*> graphicsItemList = mScene->items();
    for(int i = 0; i < graphicsItemList.size(); i+=1)
    {
        QGraphicsItem* item = graphicsItemList.at(i);
        if(item->isSelected())
            selectedIndex = i;

        if (item != mDropBar)
        {
            mScene->removeItem(item);
            delete item;
            item = NULL;
        }
    }

    for(int i = 0; i < source->selectedDocument()->pageCount(); i++)
    {
        //claudio This is a very bad hack and shows a architectural problem
        // source->selectedDocument()->pageCount()  !=   source->pageCount()
        if(i>=source->pageCount() || !source->pageAt(i))
            source->insertThumbPage(i);

        const QPixmap* pix = source->pageAt(i);
        Q_ASSERT(!pix->isNull());
        int pageIndex = UBDocumentContainer::pageFromSceneIndex(i);

        UBSceneThumbnailNavigPixmap* pixmapItem = new UBSceneThumbnailNavigPixmap(*pix, source->selectedDocument(), i);

        QString label = tr("Page %0").arg(pageIndex);
        UBThumbnailTextItem *labelItem = new UBThumbnailTextItem(label);

        UBImgTextThumbnailElement thumbWithText(pixmapItem, labelItem);
        thumbWithText.setBorder(border());
        mThumbsWithLabels.append(thumbWithText);

        if (lastClickedIndex == i)
            mLastClickedThumbnail = pixmapItem;

        mScene->addItem(pixmapItem);
        mScene->addItem(labelItem);
    }

    if (selectedIndex >= 0 && selectedIndex < mThumbsWithLabels.count())
        mSelectedThumbnail = mThumbsWithLabels.at(selectedIndex).getThumbnail();
    else
        mSelectedThumbnail = NULL;

    // Draw the items
    refreshScene();
}

void UBDocumentNavigator::onScrollToSelectedPage(int index)
{
    int c  = 0;
    foreach(UBImgTextThumbnailElement el, mThumbsWithLabels)
    {
        if (c==index)
        {
            el.getThumbnail()->setSelected(true);
            mSelectedThumbnail = el.getThumbnail();
        }
        else
        {
            el.getThumbnail()->setSelected(false);
        }
        c++;
    }
    if(NULL != mSelectedThumbnail)
        ensureVisible(mSelectedThumbnail);
}

/**
 * \brief Refresh the given thumbnail
 * @param iPage as the given page related thumbnail
 */
void UBDocumentNavigator::updateSpecificThumbnail(int iPage)
{
    const QPixmap* pix = UBApplication::boardController->pageAt(iPage);
    UBSceneThumbnailNavigPixmap* newItem = new UBSceneThumbnailNavigPixmap(*pix, UBApplication::boardController->selectedDocument(), iPage);

    // Get the old thumbnail
    UBSceneThumbnailNavigPixmap* oldItem = mThumbsWithLabels.at(iPage).getThumbnail();
    if(NULL != oldItem)
    {
        mScene->removeItem(oldItem);
        mScene->addItem(newItem);
        mThumbsWithLabels[iPage].setThumbnail(newItem);
        if (mLastClickedThumbnail == oldItem)
            mLastClickedThumbnail = newItem;
        delete oldItem;
        oldItem = NULL;
    }

}

/**
 * \brief Put the element in the right place in the scene.
 */
void UBDocumentNavigator::refreshScene()
{
    qreal thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;

    for(int i = 0; i < mThumbsWithLabels.size(); i++)
    {
        // Get the item
        UBImgTextThumbnailElement& item = mThumbsWithLabels[i];
        int columnIndex = i % mNbColumns;
        int rowIndex = i / mNbColumns;
        item.Place(rowIndex, columnIndex, mThumbnailWidth, thumbnailHeight);
    }
    scene()->setSceneRect(scene()->itemsBoundingRect());
}

/**
 * \brief  Set the number of thumbnails columns
 * @param nbColumns as the number of columns
 */
void UBDocumentNavigator::setNbColumns(int nbColumns)
{
    mNbColumns = nbColumns;
}

/**
 * \brief Get the number of columns
 * @return the number of thumbnails columns
 */
int UBDocumentNavigator::nbColumns()
{
    return mNbColumns;
}

/**
 * \brief Set the thumbnails minimum width
 * @param width as the minimum width
 */
void UBDocumentNavigator::setThumbnailMinWidth(int width)
{
    mThumbnailMinWidth = width;
}

/**
 * \brief Get the thumbnails minimum width
 * @return the minimum thumbnails width
 */
int UBDocumentNavigator::thumbnailMinWidth()
{
    return mThumbnailMinWidth;
}

/**
 * \brief Get the border size
 * @return the border size in pixels
 */
int UBDocumentNavigator::border()
{
    return 20;
}

/**
 * \brief Handle the resize event
 * @param event as the resize event
 */
void UBDocumentNavigator::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // Update the thumbnails width
    mThumbnailWidth = (width() > mThumbnailMinWidth) ? width() - 2*border() : mThumbnailMinWidth;

    if(mSelectedThumbnail)
        ensureVisible(mSelectedThumbnail);

    // Refresh the scene
    refreshScene();
}

/**
 * \brief Handle the mouse press event
 * @param event as the mouse event
 */
void UBDocumentNavigator::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    if (!event->isAccepted())
    {
        mLongPressTimer.start();
        mLastPressedMousePos = event->pos();

        mLastClickedThumbnail = clickedThumbnail(mLastPressedMousePos);

        if(mLastClickedThumbnail)
        {
            UBApplication::boardController->persistViewPositionOnCurrentScene();
            UBApplication::boardController->persistCurrentScene();
            UBApplication::boardController->setActiveDocumentScene(mLastClickedThumbnail->sceneIndex());
            UBApplication::boardController->centerOn(UBApplication::boardController->activeScene()->lastCenter());
        }
    }
}

UBSceneThumbnailNavigPixmap* UBDocumentNavigator::clickedThumbnail(const QPoint pos) const
{
    UBSceneThumbnailNavigPixmap* clickedThumbnail = NULL;

    QGraphicsItem* clickedItem = itemAt(pos);

    if(clickedItem)
    {
        clickedThumbnail = dynamic_cast<UBSceneThumbnailNavigPixmap*>(clickedItem);

        if(!clickedThumbnail)
        {
            // If we fall here we may have clicked on the label instead of the thumbnail
            UBThumbnailTextItem* clickedTextItem = dynamic_cast<UBThumbnailTextItem*>(clickedItem);
            if(clickedTextItem)
            {
                for(int i = 0; i < mThumbsWithLabels.size(); i++)
                {
                    const UBImgTextThumbnailElement& el = mThumbsWithLabels.at(i);
                    if(el.getCaption() == clickedTextItem)
                    {
                        clickedThumbnail = el.getThumbnail();
                        break;
                    }
                }
            }
        }
    }

    return clickedThumbnail;
}

void UBDocumentNavigator::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
    mLongPressTimer.stop();
}


void UBDocumentNavigator::keyPressEvent(QKeyEvent *event)
{
    UBBoardController* controller = UBApplication::boardController;
    // send to the scene anyway
    QApplication::sendEvent (scene (), event);

    if (!event->isAccepted ())
    {
        switch (event->key ())
        {
        case Qt::Key_Up:
        case Qt::Key_PageUp:
        case Qt::Key_Left:
        {
            controller->previousScene ();
            break;
        }

        case Qt::Key_Down:
        case Qt::Key_PageDown:
        case Qt::Key_Right:
        case Qt::Key_Space:
        {
            controller->nextScene ();
            break;
        }

        case Qt::Key_Home:
        {
            controller->firstScene ();
            break;
        }
        case Qt::Key_End:
        {
            controller->lastScene ();
            break;
        }
        case Qt::Key_Insert:
        {
            controller->addScene ();
            break;
        }
        case Qt::Key_Control:
        case Qt::Key_Shift:
        {
            controller->controlView()->setMultiselection(true);
        }break;
        }


        if (event->modifiers () & Qt::ControlModifier) // keep only ctrl/cmd keys
        {
            switch (event->key ())
            {
            case Qt::Key_Plus:
            case Qt::Key_I:
            {
                controller->zoomIn ();
                event->accept ();
                break;
            }
            case Qt::Key_Minus:
            case Qt::Key_O:
            {
                controller->zoomOut ();
                event->accept ();
                break;
            }
            case Qt::Key_0:
            {
                controller->zoomRestore ();
                event->accept ();
                break;
            }
            case Qt::Key_Left:
            {
                controller->handScroll (-100, 0);
                event->accept ();
                break;
            }
            case Qt::Key_Right:
            {
                controller->handScroll (100, 0);
                event->accept ();
                break;
            }
            case Qt::Key_Up:
            {
                controller->handScroll (0, -100);
                event->accept ();
                break;
            }
            case Qt::Key_Down:
            {
                controller->handScroll (0, 100);
                event->accept ();
                break;
            }
            default:
            {
                // NOOP
            }
            }
        }
    }

    // if ctrl of shift was pressed combined with other keys - we need to disable multiple selection.
    if (event->isAccepted())
        controller->controlView()->setMultiselection(false);
}


void UBDocumentNavigator::longPressTimeout()
{
    if (QApplication::mouseButtons() != Qt::NoButton)
        emit mousePressAndHoldEventRequired();

    mLongPressTimer.stop();
}

void UBDocumentNavigator::mousePressAndHoldEvent()
{
    if (mLastClickedThumbnail)
    {
        mDropSource = mLastClickedThumbnail;
        mDropTarget = mLastClickedThumbnail;

        QPixmap pixmap = mLastClickedThumbnail->pixmap().scaledToWidth(mThumbnailWidth/2);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(new QMimeData());
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));

        drag->exec();
    }
}

void UBDocumentNavigator::dragEnterEvent(QDragEnterEvent *event)
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

void UBDocumentNavigator::dragMoveEvent(QDragMoveEvent *event)
{
    QPointF position = event->pos();

    //autoscroll during drag'n'drop
    QPointF scenePos = mapToScene(position.toPoint());
    int thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;
    QRectF thumbnailArea(0, scenePos.y() - thumbnailHeight/2, mThumbnailWidth, thumbnailHeight);

    ensureVisible(thumbnailArea);

    UBSceneThumbnailNavigPixmap* item = dynamic_cast<UBSceneThumbnailNavigPixmap*>(itemAt(position.toPoint()));
    if (item)
    {
        if (item != mDropTarget)
            mDropTarget = item;

        qreal scale = item->transform().m11();

        QPointF itemCenter(item->pos().x() + (item->boundingRect().width()-verticalScrollBar()->width()) * scale,
                           item->pos().y() + item->boundingRect().height() * scale / 2);

        bool dropAbove = mapToScene(position.toPoint()).y() < itemCenter.y();
        if (mDropSource)
        {
            bool movingUp = mDropSource->sceneIndex() > item->sceneIndex();
            qreal y = 0;

            if (movingUp)
            {
                if (dropAbove)
                {
                    y = item->pos().y() - UBSettings::thumbnailSpacing / 2;
                    if (mDropBar->y() != y)
                        mDropBar->setRect(QRectF(item->pos().x(), y, mThumbnailWidth-verticalScrollBar()->width(), 3));
                }
            }
            else
            {
                if (!dropAbove)
                {
                    y = item->pos().y() + item->boundingRect().height() * scale + UBSettings::thumbnailSpacing / 2;
                    if (mDropBar->y() != y)
                        mDropBar->setRect(QRectF(item->pos().x(), y, mThumbnailWidth-verticalScrollBar()->width(), 3));
                }
            }
        }
    }
    event->acceptProposedAction();
}

void UBDocumentNavigator::dropEvent(QDropEvent *event)
{
    Q_UNUSED(event);

    if (mDropSource && mDropTarget)
    {
        if (mDropSource->sceneIndex() != mDropTarget->sceneIndex())
            UBApplication::boardController->moveSceneToIndex(mDropSource->sceneIndex(), mDropTarget->sceneIndex());
    }

    mDropSource = NULL;
    mDropTarget = NULL;
    mLastClickedThumbnail = NULL;

    mDropBar->setRect(QRectF());
    mDropBar->hide();
}
