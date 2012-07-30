/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#include <QPixmap>
#include <QDrag>
#include <QPainter>

#include "UBLibPathViewer.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#include "core/UBDownloadManager.h"
#include "board/UBBoardPaletteManager.h"

#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBLibPathViewer::UBLibPathViewer(QWidget *parent, const char *name):QGraphicsView(parent)
    , mpElems(NULL)
    , mpElemsBackup(NULL)
    , mpScene(NULL)
    , mpLayout(NULL)
    , mpContainer(NULL)
    , mpBackElem(NULL)
{
    setObjectName(name);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setAcceptDrops(true);

    mpBackElem = new UBLibElement();
    mpBackElem->setThumbnail(QPixmap(":images/libpalette/back.png").toImage());
    mpBackElem->setDeletable(false);

    mpScene = new UBPathScene(this);
    setScene(mpScene);

    mpContainer = new QGraphicsWidget();
    mpContainer->setMinimumWidth(width() - 20);
    mpScene->addItem(mpContainer);
    mpLayout = new QGraphicsLinearLayout();
    mpContainer->setLayout(mpLayout);

    connect(mpScene, SIGNAL(mouseClick(UBChainedLibElement*)), this, SLOT(onMouseClicked(UBChainedLibElement*)));
    connect(mpScene, SIGNAL(elementsDropped(QList<QString>,UBLibElement*)), this, SLOT(onElementsDropped(QList<QString>,UBLibElement*)));
    connect(horizontalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(onSliderMoved(int)));
}

/**
 * \brief Destructor
 */
UBLibPathViewer::~UBLibPathViewer()
{
    if(NULL != mpContainer)
    {
        delete mpContainer;
        mpContainer = NULL;
    }
    if(NULL != mpBackElem)
    {
        delete mpBackElem;
        mpBackElem = NULL;
    }
    if(NULL != mpElems)
    {
        delete mpElems;
        mpElems = NULL;
    }
    //if(NULL != mpElemsBackup)
    //{
    //    delete mpElemsBackup;
    //    mpElemsBackup = NULL;
    //}
    //if(NULL != mpLayout)
    //{
    //    delete mpLayout;
    //    mpLayout = NULL;
    //}
    if(NULL != mpScene)
    {
        delete mpScene;
        mpScene = NULL;
    }
}

/**
 * \brief Display the current path
 * @param elementsChain as the path to display
 */
void UBLibPathViewer::displayPath(UBChainedLibElement *elementsChain)
{
    if(NULL != elementsChain)
    {
        mpElems = elementsChain;
        refreshPath();
    }
}

/**
 * \brief Refresh the current path
 */
void UBLibPathViewer::refreshPath()
{
    if (mpScene && mpContainer)
        mpScene->removeItem(mpContainer);
    if(mpContainer)
        delete mpContainer;
    mVItems.clear();
    mpScene->mapWidgetToChainedElem()->clear();
    mpContainer = new QGraphicsWidget();

    mpScene->addItem(mpContainer);
    mpLayout = new QGraphicsLinearLayout();
    mpContainer->setLayout(mpLayout);
    mSceneWidth = 0;
    addItem(mpElems);
    mpLayout->addStretch();

    updateScrolls();

}

/**
 * \brief Handle the slider moved event
 * @param value as the current slider position
 */
void UBLibPathViewer::onSliderMoved(int value)
{
    Q_UNUSED(value);
}

/**
 * \brief Update the scroll bar status
 */
void UBLibPathViewer::updateScrolls()
{
    int iLimit = mSceneWidth + 40; // 2x 20 pixels margin
    int iVp = viewport()->width();

    if(iLimit >= iVp)
    {
        int iDiff = iLimit - iVp;
        horizontalScrollBar()->setRange(0, iDiff);
    }
    else
    {
        horizontalScrollBar()->setRange(0, 0);
    }
}

/**
 * \brief Append an item to the path
 * @param elem as the element to add to the path
 */
void UBLibPathViewer::addItem(UBChainedLibElement *elem)
{
    if(NULL != elem)
    {
        // Add the icon
        QLabel* pIconLabel = new QLabel();
        pIconLabel->setStyleSheet(QString("background-color: transparent;"));
        pIconLabel->setPixmap((QPixmap::fromImage(*elem->element()->thumbnail())).scaledToWidth(PATHITEMWIDTH));
        UBFolderPath* iconWidget = reinterpret_cast<UBFolderPath*>(mpScene->addWidget(pIconLabel));
        //iconWidget->setToolTip(elem->element()->name());
        iconWidget->setWindowFlags(Qt::BypassGraphicsProxyWidget);
        mpLayout->addItem(iconWidget);
        mVItems << iconWidget;
        mpScene->mapWidgetToChainedElem()->insert(iconWidget,elem);
        mSceneWidth += pIconLabel->pixmap()->width() + 4; // 2px border

        if(NULL != elem->nextElement())
        {
            // Add the arrow
            QLabel* pArrowLabel = new QLabel();
            pArrowLabel->setStyleSheet(QString("background-color: transparent;"));
            pArrowLabel->setPixmap(QPixmap(":images/navig_arrow.png"));
            QGraphicsWidget* arrowWidget = mpScene->addWidget(pArrowLabel);
            mpLayout->addItem(arrowWidget);
            mVItems << arrowWidget;
            mSceneWidth += pArrowLabel->pixmap()->width() + 4; // 2px border

            // Recursively call this method while a next item exists
            addItem(elem->nextElement());
        }
    }
}

/**
 * \brief Handles the resize event
 * @param event as the resize event
 */
void UBLibPathViewer::resizeEvent(QResizeEvent *event)
{
 
    if(event->oldSize() == event->size())
        event->ignore();
    else{
        if(NULL != mpContainer)
            mpContainer->setMinimumWidth(width() - 20);
        
        viewport()->resize(width() - 10, viewport()->height());

        updateScrolls();
        event->accept();
    }
}

void UBLibPathViewer::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    updateScrolls();
}

/**
 * \brief Handles the mouse move event
 * @param event as the mouse move event
 */
void UBLibPathViewer::mouseMoveEvent(QMouseEvent *event)
{
    event->ignore();
}

void UBLibPathViewer::onMouseClicked(UBChainedLibElement *elem)
{
    emit mouseClick(elem);
}

int UBLibPathViewer::widgetAt(QPointF p)
{
    int position = -1;

    for(int i = 0; i < mVItems.size(); i++)
    {
        QGraphicsWidget* pCrntWidget = mVItems.at(i);
        if(NULL != pCrntWidget)
        {
            QRectF r = pCrntWidget->rect();
            QPointF wPos = pCrntWidget->scenePos();
            int xMin = wPos.x() + r.x();
            int xMax = wPos.x() + r.x() + r.width();
            int yMin = wPos.y() + r.y();
            int yMax = wPos.y() + r.y() + r.height();

            if(p.x() >= xMin &&
               p.x() <= xMax &&
               p.y() >= yMin &&
               p.y() <= yMax)
            {
                return i;
            }
        }
    }

    return position;
}

void UBLibPathViewer::onElementsDropped(QList<QString> elements, UBLibElement *target)
{
    emit elementsDropped(elements, target);
}

void UBLibPathViewer::showBack()
{
    // Backup the current path so we can go back by clicking on the back button
    mpElemsBackup = mpElems;

    // Set the correct path to the backElem
    UBChainedLibElement* pLastElem = mpElemsBackup->lastElement();

    if(NULL != pLastElem)
    {
        mpBackElem->setPath(pLastElem->element()->path());
        mpBackElem->setType(pLastElem->element()->type());
        mpBackElem->setName(pLastElem->element()->name());
    }

    // Display the 'back' element
    displayPath(new UBChainedLibElement(mpBackElem));
}

UBFolderPath::UBFolderPath():QGraphicsProxyWidget()
{

}

UBFolderPath::~UBFolderPath()
{

}

/**
 * \brief Handles the drag enter event
 * @param pEvent as the drag enter event
 */
void UBFolderPath::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->acceptProposedAction();
}

/**
 * \brief Handles the drop event
 * @param pEvent as the drop event
 */
void UBFolderPath::dropEvent(QDropEvent *pEvent)
{
    processMimeData(pEvent->mimeData());
    pEvent->acceptProposedAction();
}

/**
 * \brief Handles the drag move event
 * @param pEvent as the drag move event
 */
void UBFolderPath::dragMoveEvent(QDragMoveEvent* pEvent)
{
    pEvent->acceptProposedAction();
}

/**
 * \brief Process the given MIME data
 * @param pData as the MIME data to process
 */
void UBFolderPath::processMimeData(const QMimeData *pData)
{
    Q_UNUSED(pData);
}

/**
 * \brief Handles the mouse press event
 * @param event as the mouse press event
 */
void UBFolderPath::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

/**
 * \brief Handles the mouse move event
 * @param event as the mouse move event
 */
void UBFolderPath::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

/**
 * \brief Handles the mouse release event
 * @param event as the mouse release event
 */
void UBFolderPath::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}


UBPathScene::UBPathScene(QWidget* parent):QGraphicsScene(parent)
{
    connect(UBDownloadManager::downloadManager(), SIGNAL(allDownloadsFinished()), this, SLOT(onAllDownloadsFinished()));
}

UBPathScene::~UBPathScene()
{

}


void UBPathScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mDragStartPos = event->scenePos();
        mClickTime = QTime::currentTime();
    }
}

/**
 * \brief Handles the mouse release event
 * @param event as the mouse release event
 */
void UBPathScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    int elapsedTimeSincePress = mClickTime.elapsed();

    if(elapsedTimeSincePress < STARTDRAGTIME)
    {
        QGraphicsWidget* pGWidget = dynamic_cast<QGraphicsWidget*>(itemAt(event->pos()));
        if(NULL != pGWidget)
        {
            // We have only one view at a time
            UBLibPathViewer* pView = dynamic_cast<UBLibPathViewer*>(this->views().at(0));
            if(NULL != pView)
            {
                int iClickedItem = pView->widgetAt(event->scenePos());
				QGraphicsLayout* wgtLayout = pGWidget->layout();
                if(iClickedItem != -1 && wgtLayout != NULL)
                {
					QGraphicsWidget* pFolderW = dynamic_cast<QGraphicsWidget*>(wgtLayout->itemAt(iClickedItem));
                    if(NULL != pFolderW)
                    {
                        UBChainedLibElement* chElem = mMapWidgetToChainedElem[pFolderW];
                        if(NULL != chElem)
                        {
                            emit mouseClick(chElem);
                        }
                    }
                }
            }
        }
    }
}

/**
 * \brief Handles the mouse move event
 * @param event as the mouse move event
 */
void UBPathScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if((event->pos() - mDragStartPos).manhattanLength() < QApplication::startDragDistance())
        {
            // The user is not doing a drag
            return;
        }

        // The user is performing a drag operation
        QDrag* drag = new QDrag(event->widget());
        QMimeData* mimeData = new QMimeData();
        drag->setMimeData(mimeData);
        drag->start();
    }
}


void UBPathScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}

void UBPathScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}

void UBPathScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    bool bAccept = false;
    const QMimeData* pMimeData = event->mimeData();

    if(NULL != event->source() && 0 == QString::compare(event->source()->metaObject()->className(), "UBLibraryWidget")){
        UBLibElement* pTargetElement = elementFromPos(event->scenePos());
        if(NULL != pTargetElement){
            if(eUBLibElementType_Folder == pTargetElement->type()){
                // The drag comes from this application, we have now to get the list of UBLibElements*
                QList<QString> qlDroppedElems;

                foreach(QUrl url, pMimeData->urls())
                    qlDroppedElems << url.toString();

                if(!qlDroppedElems.empty()){
                    // Send a signal with the target dir and the list of ublibelement*
                    emit elementsDropped(qlDroppedElems, pTargetElement);
                }
            }
        }

        bAccept = true;
    }else if(NULL != pMimeData && pMimeData->hasUrls()){
        QList<QUrl> urls = pMimeData->urls();
        foreach(QUrl eachUrl, urls){
            QString sUrl = eachUrl.toString();
            if(!sUrl.startsWith("uniboardTool://") && !sUrl.startsWith("file://") && !sUrl.startsWith("/")){
                // The dropped URL comes from the web
                // Show the download palette if it is hidden
                UBApplication::boardController->paletteManager()->startDownloads();

                // Add the dropped url to the download list
                sDownloadFileDesc desc;
                desc.currentSize = 0;
                desc.id = 0;
                desc.isBackground = false;
                desc.modal = false;
                desc.name = QFileInfo(sUrl).fileName();
                desc.totalSize = 0;
                desc.url = sUrl;
                UBDownloadManager::downloadManager()->addFileToDownload(desc);
				bAccept = true;
            }
        }
    }
	if(!bAccept && NULL != pMimeData && pMimeData->hasText()){
        //  The user can only drop an Url in this location so if the text is not an Url,
        //  we discard it.
        QString qsTxt = pMimeData->text().remove(QRegExp("[\\0]"));
        if(qsTxt.startsWith("http")){
            // Show the download palette if it is hidden
            UBApplication::boardController->paletteManager()->startDownloads();

            // Add the dropped url to the download list
            sDownloadFileDesc desc;
            desc.currentSize = 0;
            desc.id = 0;
            desc.isBackground = false;
            desc.modal = false;
            desc.name = QFileInfo(qsTxt).fileName();
            desc.totalSize = 0;
            desc.url = qsTxt;
            UBDownloadManager::downloadManager()->addFileToDownload(desc);
            bAccept = true;
        }
    }
	if(!bAccept && NULL != pMimeData && pMimeData->hasHtml()){
        QString html = pMimeData->html();
        QString url = UBApplication::urlFromHtml(html);
        if("" != url)
        {
            // Show the download palette if it is hidden
            UBApplication::boardController->paletteManager()->startDownloads();

            // Add the dropped url to the download list
            sDownloadFileDesc desc;
            desc.currentSize = 0;
            desc.id = 0;
            desc.isBackground = false;
            desc.modal = false;
            desc.name = QFileInfo(url).fileName();
            desc.totalSize = 0;
            desc.url = url;
            UBDownloadManager::downloadManager()->addFileToDownload(desc);
			bAccept = true;
        }
    }
    if(bAccept){
        event->accept();
    }
    else{
        event->ignore();
    }
}

/**
 * \brief Return the element related to the given position
 * @param p as the given position
 *
 */
UBLibElement* UBPathScene::elementFromPos(QPointF p)
{
    UBLibElement* pElem = NULL;

    QGraphicsWidget* pGWidget = dynamic_cast<QGraphicsWidget*>(itemAt(p));
    if(NULL != pGWidget)
    {
        UBChainedLibElement* chElem = mMapWidgetToChainedElem[pGWidget];
        if(NULL != chElem)
        {
            return chElem->element();
        }
    }

    return pElem;
}

void UBPathScene::onAllDownloadsFinished()
{
    // Hide the download tab
    UBApplication::boardController->paletteManager()->stopDownloads();
}
