#include <QList>
#include <QFileInfo>

#include "UBLibraryWidget.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"

#include "board/UBBoardController.h"
#include "board/UBLibraryController.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the widget object name
 */
UBLibraryWidget::UBLibraryWidget(QWidget *parent, const char *name):UBThumbnailWidget(parent)
        , chainedElements(NULL)
        , mpCrntDir(NULL)
        , mpCrntElem(NULL)
        , mCrntPath("/")
{
    setObjectName(name);
    setSpacing(5);
    setStyleSheet(QString("background: #EEEEEE; border-radius : 10px; border : 2px solid #999999;"));
}

/**
 * \brief Destructor
 */
UBLibraryWidget::~UBLibraryWidget()
{
    if(NULL != chainedElements)
    {
        delete chainedElements;
        chainedElements = NULL;
    }
    if(NULL != mpCrntDir)
    {
        delete mpCrntDir;
        mpCrntDir = NULL;
    }
    if(NULL != mpCrntElem)
    {
        delete mpCrntElem;
        mpCrntElem = NULL;
    }
}

/**
 * \brief Initialize the widget content
 */
void UBLibraryWidget::init()
{
    setAcceptDrops(true);
    mpCrntElem = new UBLibElement();
    mpCrntElem->setThumbnail(new QImage(":images/libpalette/home.png"));
    chainedElements = new UBChainedLibElement(mpCrntElem);
    QList<UBLibElement*> qlElems = libraryController()->getContent(mpCrntElem);
    mCurrentElems = qlElems;

    setCurrentElemsAndRefresh(chainedElements);

    connect(this, SIGNAL(mouseClick(QGraphicsItem*,int)), this, SLOT(onItemClicked(QGraphicsItem*,int)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
}

/**
 * \brief Refresh the view
 */
void UBLibraryWidget::refreshView()
{
    // Clear the view
    mItems.clear();
    mLabels.clear();
    mItemsPaths.clear();
    mGraphicItems.clear();

    // Generate the graphics items
    generateItems();

    // Set the new items
    setGraphicsItems(mGraphicItems, mItemsPaths, mLabels);

    // Refresh the view
    refreshScene();

    emit navigBarUpdate(mpCrntElem);

    bool bFavorite = false;
    if(NULL != mpCrntDir && libraryController()->favoritePath() == mpCrntDir->path().toLocalFile())
    {
        bFavorite = true;
    }
    emit favoritesEntered(bFavorite);
}

/**
 * \brief Generate the graphic items related to the current directory
 */
void UBLibraryWidget::generateItems()
{
    for(int i = 0; i < mCurrentElems.size(); i++)
    {
        UBLibElement* pElem = mCurrentElems.at(i);
        mLabels << pElem->name();
        mItemsPaths << pElem->path();
        QGraphicsPixmapItem *pixmapItem = new UBThumbnailPixmap(QPixmap::fromImage(*pElem->thumbnail()));
        mGraphicItems << pixmapItem;
    }
}

/**
 * \brief Handles the click on an item
 * @param item as the clicked item
 * @param index as the given index
 */
void UBLibraryWidget::onItemClicked(QGraphicsItem *item, int index)
{
    Q_UNUSED(index);
    if(NULL != item)
    {
        int iItem = mGraphicItems.indexOf(item);
        if(0 <= iItem)
        {
            UBLibElement* pElem = mCurrentElems.at(iItem);
            if(NULL != pElem)
            {
                mpCrntElem = pElem;
                if(eUBLibElementType_Folder == pElem->type() ||
                   eUBLibElementType_VirtualFolder == pElem->type())
                {
                    // Add the clicked element to the end of the elements list
                    // (at this level, the user can only go down in the path)
                    UBChainedLibElement* pNextElem = new UBChainedLibElement(pElem);
                    appendChainedElement(pNextElem, chainedElements);

                    mpCrntDir = pElem;
                    // Display the content of the folder
                    QList<UBLibElement*> qlElems = libraryController()->getContent(pElem);
                    mCurrentElems = qlElems;
                    refreshView();
                }
                else
                {
                    // Display the properties view
                    emit propertiesRequested(pElem);
                }
            }
        }
    }
}

/**
 * \brief Append the given element to the given chain
 * @param element as the element to append
 * @param toElem as the given chain
 */
void UBLibraryWidget::appendChainedElement(UBChainedLibElement *element, UBChainedLibElement *toElem)
{
    if(NULL != toElem)
    {
        if(NULL != toElem->nextElement())
        {
            appendChainedElement(element, toElem->nextElement());
        }
        else
        {
            toElem->setNextElement(element);
        }
    }
}

/**
 * \brief Set the current element and refresh the scene
 * @param elem as the current element
 */
void UBLibraryWidget::setCurrentElemsAndRefresh(UBChainedLibElement *elem)
{
    if(NULL != elem)
    {
        UBLibElement* pLibElem = elem->element();
        if(NULL != pLibElem)
        {
            if(eUBLibElementType_Item != pLibElem->type())
            {
                QList<UBLibElement*> qlElements = libraryController()->getContent(pLibElem);
                mCurrentElems = qlElements;
                mpCrntElem = pLibElem;
                refreshView();
                mpCrntDir = pLibElem;
                bool bFavorite = false;
                if(NULL != mpCrntDir && libraryController()->favoritePath() == mpCrntDir->path().toLocalFile())
                {
                    bFavorite = true;
                }
                emit favoritesEntered(bFavorite);
            }
        }
    }
}

/**
 * \brief Handles the selection changed event
 */
void UBLibraryWidget::onSelectionChanged()
{
    // Get the selected items
    QList<UBLibElement*> qlSelectedItems;
    QList<QGraphicsItem*> qlGI = selectedItems();

    bCanDrag = true;
    foreach(QGraphicsItem* it, qlGI)
    {
        int itIndex = mGraphicItems.indexOf(it);
        if(0 <= itIndex)
        {
            UBLibElement* pElem = mCurrentElems.at(itIndex);
            if(NULL != pElem)
            {
                if(eUBLibElementType_Category != pElem->type() &&
                   eUBLibElementType_VirtualFolder != pElem->type())
                {
                    qlSelectedItems << pElem;
                }

                if(!pElem->isMoveable())
                {
                    bCanDrag = false;
                }
            }
        }
    }

    // Check if we are in the trash folder
    bool bInTrash = false;
    if(NULL != mpCrntDir)
    {
        if("Trash" == mpCrntDir->name())
        {
            bInTrash = true;
        }
    }

    // Send the signal with these items
    emit itemsSelected(qlSelectedItems, bInTrash);
}

/**
 * \brief Handle the delete done event
 */
void UBLibraryWidget::onRefreshCurrentFolder()
{
    // Refresh the current view
    mCurrentElems = libraryController()->getContent(mpCrntDir);
    refreshView();
}

/**
 * \brief Handles the drag enter event
 * @param event as the drag enter event
 */
void UBLibraryWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

/**
 * \brief Handles the drag move event
 * @param event as the drag move event
 */
void UBLibraryWidget::dragMoveEvent(QDragMoveEvent *event)
{
    UBLibElement* pElem = elementAt(event->pos());
    if(NULL != pElem)
    {
        // We can only drop an item into a folder
        if(eUBLibElementType_Folder == pElem->type() ||
           eUBLibElementType_VirtualFolder == pElem->type())
        {
            event->acceptProposedAction();
        }
    }
}

void UBLibraryWidget::onDropMe(const QMimeData *_data)
{
	Q_UNUSED(_data);
}

/**
 * \brief Handles the drop event
 * @param event as the drop event
 */
void UBLibraryWidget::dropEvent(QDropEvent *event)
{
    const QMimeData* pMimeData = event->mimeData();
    if(event->source() == this)
    {
        event->accept();

        // Get the destination item
        UBLibElement* pElem = elementAt(event->pos());
        if(NULL != pElem)
        {
            if(eUBLibElementType_Folder == pElem->type())
            {
                // The drag comes from this application, we have now to get the list of UBLibElements*
                QList<QString> qlDroppedElems;

                foreach(QUrl url, pMimeData->urls())
                {
                    qlDroppedElems << url.toString();
                }

                if(!qlDroppedElems.empty())
                    onElementsDropped(qlDroppedElems, pElem);
            }
        }
    }
    else
    {
        bool bDropAccepted = false;
        if (pMimeData->hasImage())
        {
            qDebug() << "hasImage";
            QImage image = qvariant_cast<QImage>(pMimeData->imageData());
            libraryController()->importImageOnLibrary(image);
            bDropAccepted = true;
        }
        else if (pMimeData->hasHtml())
        {
            qDebug() << "hasHtml  Unsupported yet";
        }
        else if (pMimeData->hasText())
        {
            // On linux external dragged element are considered as text;
            qDebug()  << "hasText: " << pMimeData->text();
            QString filePath = QUrl(pMimeData->text()).toLocalFile();
            libraryController()->importItemOnLibrary(filePath);
            bDropAccepted = true;
        }
        else if (pMimeData->hasUrls())
        {
            qDebug() << "hasUrls";
            QList<QUrl> urlList = pMimeData->urls();
            for (int i = 0; i < urlList.size() && i < 32; ++i)
            {
                QString filePath = QUrl(urlList.at(i).path()).toLocalFile();
                libraryController()->importItemOnLibrary(filePath);
                bDropAccepted = true;
            }
        }
        else
        {
            qWarning() << "Cannot import data";
        }

        if(bDropAccepted)
        {
            onRefreshCurrentFolder();
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
}

/**
 * \brief Get the element at the given position
 * @param p as the given position
 * @return a pointer on the related element
 */
UBLibElement* UBLibraryWidget::elementAt(QPoint p)
{
    QGraphicsItem* pItem = itemAt(p);
    if(NULL != pItem)
    {
        int iItem = mGraphicItems.indexOf(pItem);
        if(-1 != iItem)
        {
            return mCurrentElems.at(iItem);
        }
    }

    // If no element is found, return NULL
    return NULL;
}


/**
 * \brief Get the element from the given name
 * @param name as the given element name
 * @return the UBLibElement related to the given name
 */
UBLibElement* UBLibraryWidget::elementFromFilePath(const QString &filePath)
{
    UBLibElement* pElem = NULL;

    foreach(UBLibElement* elem, mCurrentElems)
    {
        if(elem->path().toLocalFile() == QUrl(filePath).toLocalFile())
        {
            return elem;
        }

    }

    return pElem;
}

/**
 * \brief Get a pointer on the library controller
 * @return a pointer on the library controller
 */
UBLibraryController* UBLibraryWidget::libraryController()
{
    return UBApplication::boardController->libraryController();
}

/**
 * \brief Update the thumbnails size
 * @param newSize as the thumbnail size
 */
void UBLibraryWidget::updateThumbnailsSize(int newSize)
{
    setThumbnailWidth(newSize);
    refreshView();
}

/**
 * \brief Handles the element dropped event
 * @param elements as the list of dropped elements
 * @param target as the drop target
 */
void UBLibraryWidget::onElementsDropped(QList<QString> elements, UBLibElement *target)
{
    if(target != mpCrntDir)
    {
        QList<UBLibElement*> qlElements;

        foreach(QString qsElem, elements)
            qlElements << elementFromFilePath(qsElem);

        libraryController()->moveContent(qlElements, target);
        mCurrentElems = libraryController()->getContent(mpCrntDir);
        refreshView();
    }
}

/**
 * \brief Search the element related to the given text
 * @param elem as the searched element name
 */
void UBLibraryWidget::onSearchElement(QString elem)
{
    // Store the original list of items
    mOrigCurrentElems = libraryController()->getContent(mpCrntDir);

    // Build the filtered list
    mCurrentElems.clear();
    if(elem.isEmpty())
    {
        mCurrentElems = mOrigCurrentElems;
    }
    else
    {
        foreach(UBLibElement* ubLibElem, mOrigCurrentElems)
        {
            if(ubLibElem->name().toLower().contains(elem.toLower()))
            {
                mCurrentElems << ubLibElem;
            }
        }
    }
    refreshView();
}

/**
 * \brief Create a new folder
 */
void UBLibraryWidget::onNewFolderToCreate()
{
    // Create here a dialog asking the name of the new folder
    UBNewFolderDlg dlg;
    if(QDialog::Accepted == dlg.exec())
    {
        libraryController()->createNewFolder(dlg.folderName(), mpCrntElem);
        onRefreshCurrentFolder();
    }
}

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBNewFolderDlg::UBNewFolderDlg(QWidget *parent, const char *name):QDialog(parent)
    , mpLabel(NULL)
    , mpLineEdit(NULL)
    , mpButtons(NULL)
    , mpLayout(NULL)
    , mpHLayout(NULL)
{
    setObjectName(name);
    setWindowTitle(tr("Add new folder"));

    mpLabel = new QLabel(tr("New Folder name:"),this);
    mpLineEdit = new QLineEdit(this);
    mpButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mpLayout = new QVBoxLayout(this);
    mpHLayout = new QHBoxLayout(this);

    setLayout(mpLayout);
    mpLayout->addLayout(mpHLayout, 0);
    mpHLayout->addWidget(mpLabel, 0);
    mpHLayout->addWidget(mpLineEdit, 1);
    mpLayout->addWidget(mpButtons);

    connect(mpButtons->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(mpButtons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));

    setMaximumHeight(100);
    setMinimumHeight(100);
}

/**
 * \brief Destructor
 */
UBNewFolderDlg::~UBNewFolderDlg()
{
    if(NULL != mpButtons)
    {
        delete mpButtons;
        mpButtons = NULL;
    }
    if(NULL != mpLineEdit)
    {
        delete mpLineEdit;
        mpLineEdit = NULL;
    }
    if(NULL != mpLabel)
    {
        delete mpLabel;
        mpLabel = NULL;
    }
    if(NULL != mpHLayout)
    {
        delete mpHLayout;
        mpHLayout = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

/**
 * \brief Get the folder name
 * @return the entered folder name
 */
QString UBNewFolderDlg::folderName()
{
    return mpLineEdit->text();
}
