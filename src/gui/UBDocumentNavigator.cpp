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
  , mCrntItem(NULL)
  , mCrntDoc(NULL)
  , mNbColumns(1)
  , mThumbnailWidth(0)
  , mThumbnailMinWidth(100)
  , bNavig(false)
{
    setObjectName(name);
    mScene = new QGraphicsScene(this);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(mScene);
    mThumbnailWidth = width() - 2*border();

    setFrameShadow(QFrame::Plain);

    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(generateThumbnails()));
    connect(UBApplication::boardController, SIGNAL(newPageAdded()), this, SLOT(addNewPage()));
    connect(mScene, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
    connect(UBApplication::boardController, SIGNAL(documentReorganized(int)), this, SLOT(onMovedToIndex(int)));
}

/**
 * \brief Destructor
 */
UBDocumentNavigator::~UBDocumentNavigator()
{
    if(NULL != mCrntItem)
    {
        delete mCrntItem;
        mCrntItem = NULL;
    }

    if(NULL != mScene)
    {
        delete mScene;
        mScene = NULL;
    }
}

/**
 * \brief Set the current document
 * @param document as the new document
 */
void UBDocumentNavigator::setDocument(UBDocumentProxy *document)
{
    //	Here we set a new document to the navigator. We must clear the current
    // content and add all the pages of the given document.
    if(document)
    {
        mCrntDoc = document;
        generateThumbnails();
    }
}

/**
 * \brief Generate the thumbnails
 */
void UBDocumentNavigator::generateThumbnails()
{
	// Get the thumbnails
    QList<QPixmap> thumbs = UBThumbnailAdaptor::load(mCrntDoc);

	mThumbsWithLabels.clear();
	foreach(QGraphicsItem* it, mScene->items())
    {
        mScene->removeItem(it);
        delete it;
    }

	for(int i = 0; i < thumbs.count(); i++)
    {
        QPixmap pix = thumbs.at(i);
        QGraphicsPixmapItem* pixmapItem = new UBSceneThumbnailNavigPixmap(pix, mCrntDoc, i);
		UBThumbnailTextItem *labelItem = new UBThumbnailTextItem(tr("Page %0").arg(i + 1));

		UBImgTextThumbnailElement thumbWithText(pixmapItem, labelItem);
		thumbWithText.setBorder(border());
		mThumbsWithLabels.append(thumbWithText);

		mScene->addItem(pixmapItem);
		mScene->addItem(labelItem);

        // Get the selected item
        if(UBApplication::boardController->activeSceneIndex() == i)
        {
            mCrntItem = dynamic_cast<UBSceneThumbnailNavigPixmap*>(pixmapItem);
            mCrntItem->setSelected(true);
        }
    }
    
	// Draw the items
    refreshScene();
}

/**
 * \brief Refresh the given thumbnail
 * @param iPage as the given page related thumbnail
 */
void UBDocumentNavigator::updateSpecificThumbnail(int iPage)
{
    // Generate the new thumbnail
    UBGraphicsScene* pScene = UBApplication::boardController->activeScene();

    if(NULL != pScene)
    {
        // Save the current state of the scene
        pScene->setModified(true);
        if(UBApplication::boardController)
        {
            UBApplication::boardController->persistCurrentScene();
        }else
        {
            UBThumbnailAdaptor::persistScene(mCrntDoc->persistencePath(), pScene, iPage);
        }

        // Load it
        QPixmap pix = UBThumbnailAdaptor::load(mCrntDoc, iPage);
        UBSceneThumbnailNavigPixmap* pixmapItem = new UBSceneThumbnailNavigPixmap(pix, mCrntDoc, iPage);
        if(pixmapItem)
        {
            // Get the old thumbnail
            QGraphicsItem* pItem = mThumbsWithLabels.at(iPage).getThumbnail();
            if(NULL != pItem)
            {
                mScene->removeItem(pItem);
                mScene->addItem(pixmapItem);
                mThumbsWithLabels[iPage].setThumbnail(pixmapItem);
                delete pItem;
            }
        }
    }
}

/**
 * \brief Add a new page to the thumbnails list
 *
 * This method is called automatically by the board controller each time the user
 * adds a new page, duplicates a page or imports a document.
 */
void UBDocumentNavigator::addNewPage()
{
    if(!bNavig)
    {
        generateThumbnails();
        if(NULL != mCrntItem)
        {
            mCrntItem->setSelected(true);
        }
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

    // Update the scene rect
    //    QRect sceneRect;
    //    sceneRect.setWidth(width() - 2*border());
    //    sceneRect.setHeight(height() - 2*border());
    //    sceneRect.moveLeft(border());
    //    sceneRect.moveTop(border());
    //    scene()->setSceneRect(sceneRect);

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
        bNavig = true;

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
						pCrntItem = dynamic_cast<UBSceneThumbnailNavigPixmap*>(el.getThumbnail());
						break;
					}
				}
            }
        }
		else
		{
			if(NULL != mCrntItem && mCrntItem != pCrntItem)
			{
				// Unselect the previous item
				mCrntItem->setSelected(false);
				int iOldPage = -1;
				for(int i = 0; i < mThumbsWithLabels.size(); i++)
					if (mThumbsWithLabels.at(i).getThumbnail() == mCrntItem)
					{
						iOldPage = i;
						break;
					}
				updateSpecificThumbnail(iOldPage);
				mCrntItem = pCrntItem;
			}

			// Then display the related page
			emit changeCurrentPage();
			refreshScene();
		}
        
		bNavig = false;
    }
	QGraphicsView::mousePressEvent(event);
}

/**
 * \brief Get the selected page number
 * @return the selected page number
 */
int UBDocumentNavigator::selectedPageNumber()
{
    int nbr = NO_PAGESELECTED;

    if(NULL != mCrntItem)
    {
		for(int i = 0; i < mThumbsWithLabels.size(); i++)
			if (mThumbsWithLabels.at(i).getThumbnail() == mCrntItem)
			{
				nbr = i;
				break;
			}
    }

    return nbr;
}

/**
 * \brief Get the current document
 * @return the current document
 */
UBDocumentProxy* UBDocumentNavigator::currentDoc()
{
    return mCrntDoc;
}

/**
 * \brief Occurs when the selection changed
 */
void UBDocumentNavigator::onSelectionChanged()
{
    //    QList<QGraphicsItem*> qlItems = mScene->selectedItems();
    //    qDebug() << "The number of selected items is " << qlItems.count();
}

/**
 * \brief Occurs when a page has been moved to another index in the document
 * @param index as the new index
 */
void UBDocumentNavigator::onMovedToIndex(int index)
{
    if(index < mThumbsWithLabels.size()){
        UBSceneThumbnailNavigPixmap* pItem = dynamic_cast<UBSceneThumbnailNavigPixmap*>(mThumbsWithLabels.at(index).getThumbnail());
        if(NULL != pItem)
        {
            if(mCrntItem) mCrntItem->setSelected(false);//deselecting previous one
			mCrntItem = pItem;
            mCrntItem->setSelected(true);
            centerOn(mCrntItem);
        }
    }
}
