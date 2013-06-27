/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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
{
    setObjectName(name);
    mScene = new QGraphicsScene(this);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(mScene);
    mThumbnailWidth = width() - 2*border();

    setFrameShadow(QFrame::Plain);

    connect(UBApplication::boardController, SIGNAL(documentThumbnailsUpdated(UBDocumentContainer*)), this, SLOT(generateThumbnails(UBDocumentContainer*)));
    connect(UBApplication::boardController, SIGNAL(documentPageUpdated(int)), this, SLOT(updateSpecificThumbnail(int)));
    connect(UBApplication::boardController, SIGNAL(pageSelectionChanged(int)), this, SLOT(onScrollToSelectedPage(int)));
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
//    foreach(QGraphicsItem* it, mScene->items())
    int selectedIndex = -1;
    QList<QGraphicsItem*> graphicsItemList = mScene->items();
    for(int i = 0; i < graphicsItemList.size(); i+=1)
    {
        QGraphicsItem* item = graphicsItemList.at(i);
        if(item->isSelected())
            selectedIndex = i;
        mScene->removeItem(item);
        delete item;
        item = NULL;
    }

    for(int i = 0; i < source->selectedDocument()->pageCount(); i++)
    {
        const QPixmap* pix = source->pageAt(i);
        Q_ASSERT(!pix->isNull());
        int pageIndex = UBDocumentContainer::pageFromSceneIndex(i);

        UBSceneThumbnailNavigPixmap* pixmapItem = new UBSceneThumbnailNavigPixmap(*pix, source->selectedDocument(), i);

        QString label = tr("Page %0").arg(pageIndex);
        UBThumbnailTextItem *labelItem = new UBThumbnailTextItem(label);

        UBImgTextThumbnailElement thumbWithText(pixmapItem, labelItem);
        thumbWithText.setBorder(border());
        mThumbsWithLabels.append(thumbWithText);

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
    centerOn(mSelectedThumbnail);
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
        centerOn(mSelectedThumbnail);

    // Refresh the scene
    refreshScene();
}

/**
 * \brief Handle the mouse press event
 * @param event as the mouse event
 */
void UBDocumentNavigator::mousePressEvent(QMouseEvent *event)
{
    QGraphicsItem* pClickedItem = itemAt(event->pos());
    if(NULL != pClickedItem)
    {

        // First, select the clicked item
        UBSceneThumbnailNavigPixmap* pCrntItem = dynamic_cast<UBSceneThumbnailNavigPixmap*>(pClickedItem);

        if(NULL == pCrntItem)
        {
            // If we fall here we may have clicked on the label instead of the thumbnail
            UBThumbnailTextItem* pTextItem = dynamic_cast<UBThumbnailTextItem*>(pClickedItem);
            if(NULL != pTextItem)
            {
                for(int i = 0; i < mThumbsWithLabels.size(); i++)
                {
                    const UBImgTextThumbnailElement& el = mThumbsWithLabels.at(i);
                    if(el.getCaption() == pTextItem)
                    {
                        pCrntItem = el.getThumbnail();
                        break;
                    }
                }
            }
        }

        int index = 0;
        for(int i = 0; i < mThumbsWithLabels.size(); i++)
        {
            if (mThumbsWithLabels.at(i).getThumbnail() == pCrntItem)
            {
                mSelectedThumbnail = pCrntItem;
                index = i;
                break;
            }
        }
        UBApplication::boardController->setActiveDocumentScene(index);
    }
    QGraphicsView::mousePressEvent(event);
}

void UBDocumentNavigator::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
}
