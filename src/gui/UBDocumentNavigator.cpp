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

    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(addNewPage()));
    connect(mScene, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
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
    QList<QGraphicsItem*> items;
    //QList<QUrl> itemsPath;
    QStringList labels;

    // Get the thumbnails
    QList<QPixmap> thumbs = UBThumbnailAdaptor::load(mCrntDoc);
    QGraphicsPixmapItem* selection = NULL;

    for(int i = 0; i < thumbs.count(); i++)
    {
	QPixmap pix = thumbs.at(i);
	QGraphicsPixmapItem* pixmapItem = new UBSceneThumbnailPixmap(pix, mCrntDoc, i);

	// Get the selected item
        if(UBApplication::boardController->activeSceneIndex() == i)
	{
	    selection = pixmapItem;
	    mCrntItem = dynamic_cast<UBSceneThumbnailPixmap*>(pixmapItem);
	    mCrntItem->setSelected(true);
	}

	items << pixmapItem;
	labels << tr("Page %0").arg(i + 1);
        //itemsPath.append(QUrl::fromLocalFile(mCrntDoc->persistencePath() + QString("/pages/%0").arg(i + 1)));
    }

    // Draw the items
    setGraphicsItems(items, labels);
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
	UBSvgSubsetAdaptor::persistScene(mCrntDoc,pScene, iPage);

	UBThumbnailAdaptor::persistScene(mCrntDoc->persistencePath(), pScene, iPage);

	// Load it
	QList<QPixmap> thumbs = UBThumbnailAdaptor::load(mCrntDoc);
	QPixmap pix = thumbs.at(iPage);
	QGraphicsPixmapItem* pixmapItem = new UBSceneThumbnailPixmap(pix, mCrntDoc, iPage);
	if(pixmapItem)
	{
	    // Get the old thumbnail
	    QGraphicsItem* pItem = mThumbnails.at(iPage);
	    if(NULL != pItem)
	    {
		mScene->removeItem(pItem);
		mScene->addItem(pixmapItem);
		mThumbnails.replace(iPage, pixmapItem);
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
 * \brief Set the graphics items of the scene
 * @param items as the items list
 * @param labels as the page labels
 */
void UBDocumentNavigator::setGraphicsItems(QList<QGraphicsItem *> items, QStringList labels)
{
    mThumbnails = items;
    mLab = labels;
    // First, clear the actual thumbnails
    foreach(QGraphicsItem* it, mScene->items())
    {
	mScene->removeItem(it);
	delete it;
    }

    // Add the new thumbnails
    foreach(QGraphicsItem* it, items)
    {
	mScene->addItem(it);
    }

    // Add the labels
    mLabels.clear();
    foreach(QString lb, labels)
    {
	UBThumbnailTextItem *labelItem = new UBThumbnailTextItem(lb);
	mScene->addItem(labelItem);

	mLabels << labelItem;
    }

    // Refresh the scene
    refreshScene();
}

/**
 * \brief Put the element in the right place in the scene.
 */
void UBDocumentNavigator::refreshScene()
{
    int labelSpacing = 0;

    if(mLabels.size() > 0)
    {
	QFontMetrics fm(mLabels.at(0)->font());
	labelSpacing = UBSettings::thumbnailSpacing + fm.height();
    }

    qreal thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;

    for(int i = 0; i < mThumbnails.size(); i++)
    {
	// Get the item
	QGraphicsItem* item = mThumbnails.at(i);

	// Compute the scale factor
	qreal scaleWidth = mThumbnailWidth / item->boundingRect().width();
	qreal scaleHeight = thumbnailHeight / item->boundingRect().height();
	qreal scaleFactor = qMin(scaleWidth, scaleHeight);
	UBThumbnail* pix = dynamic_cast<UBThumbnail*>(item);

	if(pix)
	{
	 scaleFactor = qMin(scaleFactor, 1.0);
	}

	QTransform transform;
	transform.scale(scaleFactor, scaleFactor);

	// Apply the scaling
	item->setTransform(transform);
	item->setFlag(QGraphicsItem::ItemIsSelectable, true);

	int columnIndex = i % mNbColumns;
	int rowIndex = i / mNbColumns;

	if(pix)
	{
	    pix->setColumn(columnIndex);
	    pix->setRow(rowIndex);
	}

	int w = item->boundingRect().width();
	int h = item->boundingRect().height();

	QPointF pos( border() + (mThumbnailWidth - w * scaleFactor) / 2 + columnIndex * (mThumbnailWidth + border()),
		     border() + rowIndex * (thumbnailHeight + border() + labelSpacing) + (thumbnailHeight - h * scaleFactor) / 2);

	item->setPos(pos);

	// Add the labels "Page x"
	if(mLabels.size() > i)
	{
	    QFontMetrics fm(mLabels.at(i)->font(), this);
	    QString elidedText = fm.elidedText(mLab.at(i), Qt::ElideRight, mThumbnailWidth);

	    mLabels.at(i)->setPlainText(elidedText);
	    mLabels.at(i)->setWidth(fm.width(elidedText) + 2 * mLabels.at(i)->document()->documentMargin());
	    pos.setY(pos.y() + (thumbnailHeight + h * scaleFactor) / 2 + 5); // What is this 5 ??
	    qreal labelWidth = fm.width(elidedText);
	    pos.setX(border() + (mThumbnailWidth - labelWidth) / 2 + columnIndex * (mThumbnailWidth + border()));
	    mLabels.at(i)->setPos(pos);
	}
    }

    QScrollBar *vertScrollBar = verticalScrollBar();
    int scrollBarThickness = 0;
    if (vertScrollBar && vertScrollBar->isVisible())
	scrollBarThickness = vertScrollBar->width();
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
	UBSceneThumbnailPixmap* pCrntItem = dynamic_cast<UBSceneThumbnailPixmap*>(pClickedItem);

	if(NULL == pCrntItem)
	{
	    // If we fall here we may have clicked on the label instead of the thumbnail
	    UBThumbnailTextItem* pTextItem = dynamic_cast<UBThumbnailTextItem*>(pClickedItem);
	    if(NULL != pTextItem)
	    {
		pCrntItem = dynamic_cast<UBSceneThumbnailPixmap*>(mThumbnails.at(mLabels.indexOf(pTextItem)));
	    }
	}
        else
        {
            if(NULL != mCrntItem && mCrntItem != pCrntItem)
            {
                // Unselect the previous item
                int iOldPage = mThumbnails.indexOf(mCrntItem);
                mCrntItem->setSelected(false);
                updateSpecificThumbnail(iOldPage);
                mCrntItem = pCrntItem;
            }

            pCrntItem->setSelected(true);

            // Then display the related page
            emit changeCurrentPage();
            refreshScene();
        }
	bNavig = false;
    }
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
	nbr = mThumbnails.indexOf(mCrntItem);
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
