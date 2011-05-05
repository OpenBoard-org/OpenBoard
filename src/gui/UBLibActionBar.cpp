#include <QIcon>
#include <QSize>
#include <QDebug>

#include "UBLibActionBar.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"


/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBLibActionBar::UBLibActionBar(QWidget *parent, const char *name):QWidget(parent)
    , mCrntButtonSet(eButtonSet_Default)
    , mPreviousButtonSet(eButtonSet_Default)
    , mButtonGroup(NULL)
    , mSearchBar(NULL)
    , mLayout(NULL)
    , mpFavoriteAction(NULL)
    , mpSocialAction(NULL)
    , mpDeleteAction(NULL)
    , mpSearchAction(NULL)
    , mpCloseAction(NULL)
    , mpRemoveFavorite(NULL)
    , mpNewFolderAction(NULL)
    , mpFavoriteBtn(NULL)
    , mpSocialBtn(NULL)
    , mpDeleteBtn(NULL)
    , mpSearchBtn(NULL)
    , mpCloseBtn(NULL)
    , mpRemoveFavoriteBtn(NULL)
    , mpNewFolderBtn(NULL)
    , bFavorite(false)
    , bIsInTrash(false)
{
    setObjectName(name);
    setStyleSheet(QString("background: #EEEEEE; border-radius : 10px; border : 2px solid #999999;"));

    setAcceptDrops(true);

    mButtonGroup = new QButtonGroup(this);
    mSearchBar = new QLineEdit(this);
    mSearchBar->setStyleSheet(QString("background-color:white; border-radius : 10px; padding : 2px;"));
    connect(mSearchBar, SIGNAL(returnPressed()), this, SLOT(onActionSearch()));

    mLayout = new QHBoxLayout();
    setLayout(mLayout);

    setMaximumHeight(ACTIONBAR_HEIGHT);

    // Create the actions
    mpFavoriteAction = new QAction(QIcon(":/images/libpalette/miniFavorite.png"), tr("Add to favorites"), this);
    mpSocialAction = new QAction(QIcon(":/images/libpalette/social.png"), tr("Share"), this);
    mpSearchAction = new QAction(QIcon(":/images/libpalette/miniSearch.png"), tr("Search"), this);
    mpDeleteAction = new QAction(QIcon(":/images/libpalette/miniTrash.png"), tr("Delete"), this);
    mpCloseAction = new QAction(QIcon(":/images/close.svg"), tr("Back to folder"), this);
    mpRemoveFavorite = new QAction(QIcon(":/images/libpalette/trash_favorite.svg"), tr("Remove from favorites"), this);
    mpNewFolderAction = new QAction(QIcon(":/images/libpalette/miniNewFolder.png"), tr("Create new folder"), this);

    // Create the buttons
    mpFavoriteBtn = new UBActionButton(this, mpFavoriteAction);
    mpSocialBtn = new UBActionButton(this, mpSocialAction);
    mpSearchBtn = new UBActionButton(this, mpSearchAction);
    mpDeleteBtn = new UBActionButton(this, mpDeleteAction);
    mpCloseBtn = new UBActionButton(this, mpCloseAction);
    mpRemoveFavoriteBtn = new UBActionButton(this, mpRemoveFavorite);
    mpNewFolderBtn = new UBActionButton(this, mpNewFolderAction);

    // Initialize the buttons
    mpSearchBtn->setEnabled(false);
    mpNewFolderBtn->setEnabled(false);

    // Add the buttons to the button group
    mButtonGroup->addButton(mpFavoriteBtn);
    mButtonGroup->addButton(mpSocialBtn);
    mButtonGroup->addButton(mpSearchBtn);
    mButtonGroup->addButton(mpDeleteBtn);
    mButtonGroup->addButton(mpCloseBtn);
    mButtonGroup->addButton(mpRemoveFavoriteBtn);
    mButtonGroup->addButton(mpNewFolderBtn);
    // Connect signals & slots
    connect(mpFavoriteAction,SIGNAL(triggered()), this, SLOT(onActionFavorite()));
    connect(mpSocialAction,SIGNAL(triggered()), this, SLOT(onActionSocial()));
    connect(mpSearchAction,SIGNAL(triggered()), this, SLOT(onActionSearch()));
    connect(mpDeleteAction,SIGNAL(triggered()), this, SLOT(onActionTrash()));
    connect(mpCloseAction, SIGNAL(triggered()), this, SLOT(onActionClose()));
    connect(mpRemoveFavorite, SIGNAL(triggered()), this, SLOT(onActionRemoveFavorite()));
    connect(mSearchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(mpNewFolderAction, SIGNAL(triggered()), this, SLOT(onActionNewFolder()));

    // Build the default toolbar
    mLayout->addWidget(mpFavoriteBtn);
    mLayout->addWidget(mpSocialBtn);
    mLayout->addWidget(mpNewFolderBtn);
    mLayout->addWidget(mSearchBar);
    mLayout->addWidget(mpSearchBtn);
    mLayout->addWidget(mpDeleteBtn);
    mLayout->addWidget(mpCloseBtn);
    mLayout->addWidget(mpRemoveFavoriteBtn);

    setButtons(eButtonSet_Default);
}

/**
 * \brief Destructor
 */
UBLibActionBar::~UBLibActionBar()
{
    if(NULL != mpNewFolderAction)
    {
        delete mpNewFolderAction;
        mpNewFolderAction = NULL;
    }
    if(NULL != mpNewFolderBtn)
    {
        delete mpNewFolderBtn;
        mpNewFolderBtn = NULL;
    }
    if(NULL != mpRemoveFavorite)
    {
        delete mpRemoveFavorite;
        mpRemoveFavorite = NULL;
    }
    if(NULL != mpRemoveFavoriteBtn)
    {
        delete mpRemoveFavoriteBtn;
        mpRemoveFavoriteBtn = NULL;
    }
    if(NULL != mpCloseAction)
    {
        delete mpCloseAction;
        mpCloseAction = NULL;
    }
    if(NULL != mpDeleteAction)
    {
        delete mpDeleteAction;
        mpDeleteAction = NULL;
    }
    if(NULL != mpFavoriteAction)
    {
        delete mpFavoriteAction;
        mpFavoriteAction = NULL;
    }
    if(NULL != mpSearchAction)
    {
        delete mpSearchAction;
        mpSearchAction = NULL;
    }
    if(NULL != mpSocialAction)
    {
        delete mpSocialAction;
        mpSocialAction = NULL;
    }
    if(NULL != mpCloseBtn)
    {
        delete mpCloseBtn;
        mpCloseBtn = NULL;
    }
    if(NULL != mpDeleteBtn)
    {
        delete mpDeleteBtn;
        mpDeleteBtn = NULL;
    }
    if(NULL != mpFavoriteBtn)
    {
        delete mpFavoriteBtn;
        mpFavoriteBtn = NULL;
    }
    if(NULL != mpSearchBtn)
    {
        delete mpSearchBtn;
        mpSearchBtn = NULL;
    }
    if(NULL != mpSocialBtn)
    {
        delete mpSocialBtn;
        mpSocialBtn = NULL;
    }
    if(NULL != mButtonGroup)
    {
        delete mButtonGroup;
        mButtonGroup = NULL;
    }
    if(NULL != mSearchBar)
    {
        delete mSearchBar;
        mSearchBar = NULL;
    }
    if(NULL != mLayout)
    {
        delete mLayout;
        mLayout = NULL;
    }
}

/**
 * \brief Set the buttons of the action bar
 * @param setID as the button set
 */
void UBLibActionBar::setButtons(eButtonSet setID)
{
    mPreviousButtonSet = mCrntButtonSet;
    mCrntButtonSet = setID;
    switch(setID)
    {
    case eButtonSet_Default:
        mpFavoriteBtn->show();
        mpSocialBtn->hide();
        mSearchBar->show();
        mpSearchBtn->show();
        mpDeleteBtn->show();
        mpCloseBtn->hide();
        mpRemoveFavoriteBtn->hide();
        mpNewFolderBtn->show();
        break;
    case eButtonSet_Properties:
        mpFavoriteBtn->show();
        mpSocialBtn->hide();
        mSearchBar->show();
        mpSearchBtn->show();
        mpDeleteBtn->hide();
        mpCloseBtn->hide();
        mpRemoveFavoriteBtn->hide();
        mpNewFolderBtn->hide();
        break;
    case eButtonSet_Favorite:
        mpFavoriteBtn->hide();
        mpSocialBtn->hide();
        mSearchBar->show();
        mpSearchBtn->show();
        mpDeleteBtn->hide();
        mpCloseBtn->hide();
        mpRemoveFavoriteBtn->show();
        mpNewFolderBtn->hide();
        break;
    default:
        break;
    }
}

/**
 * \brief (un)set the selected element to favorite
 */
void UBLibActionBar::onActionFavorite()
{
    mpFavoriteBtn->setIcon(QIcon(":/images/libpalette/miniFavorite.png"));
    libraryController()->addToFavorite(mSelectedElements);
}

/**
 * \brief Handle the mouse enter event
 * @param event as the event
 */
void UBLibActionBar::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    setCursor(Qt::ArrowCursor);
}

/**
 * \brief Handle the mouse leave event
 * @param event as the event
 */
void UBLibActionBar::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    unsetCursor();
}

/**
 * \brief Perform the search
 */
void UBLibActionBar::onActionSearch()
{
    emit searchElement(mSearchBar->text());
}

/**
 * \brief Trigger the social action
 */
void UBLibActionBar::onActionSocial()
{
    // To be implemented
}

/**
 * \brief Handles the close action
 */
void UBLibActionBar::onActionClose()
{
    emit showFolderContent();
}

/**
 * \brief Delete the selected element
 */
void UBLibActionBar::onActionTrash()
{
    if(!bIsInTrash)
    {
        libraryController()->trashElements(mSelectedElements);
    }
    else
    {
        libraryController()->emptyElementsOnTrash(mSelectedElements);
    }
    emit deleteDone();
}

/**
 * \brief Remove the selected favorite(s)
 */
void UBLibActionBar::onActionRemoveFavorite()
{
    libraryController()->removeFromFavorite(mSelectedElements);
    emit deleteDone();
}

/**
 * \brief Handles the selection change event
 * @param itemList as the list of selected items
 * @param isInTrash indicates if the current folder is the trash
 */
void UBLibActionBar::onSelectionChanged(QList<UBLibElement *> itemList, bool isInTrash)
{
    bIsInTrash = isInTrash;
    mSelectedElements = itemList;
    bool bEnable = (itemList.count() != 0) ? true : false;

    if(mCrntButtonSet == eButtonSet_Favorite)
    {
        mpRemoveFavoriteBtn->setEnabled(bEnable);
        return;
    }

    mpFavoriteAction->setEnabled(bEnable);
    mpSocialAction->setEnabled(bEnable);
    mpDeleteAction->setEnabled(bEnable && libraryController()->canItemsOnElementBeDeleted(itemList.at(0)));
}

/**
 * \brief Get the library controller
 * @return a pointer on the library controller
 */
UBLibraryController* UBLibActionBar::libraryController()
{
    return UBApplication::boardController->libraryController();
}

/**
 * \brief Show the actions related to the Favorites folder
 */
void UBLibActionBar::onFavoritesEntered(bool bFav)
{
    setButtons(bFav ? eButtonSet_Favorite : eButtonSet_Default);
}

/**
 * \brief Handles the drag enter event
 * @param event as the drag enter event
 */
void UBLibActionBar::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

/**
 * \brief Handles the drag move event
 * @param event as the drag move event
 */
void UBLibActionBar::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

/**
 * \brief Handles the drop event
 * @param event as the given drop event
 */
void UBLibActionBar::dropEvent(QDropEvent *event)
{
    const QPoint droppedPoint = event->pos();


    QWidget* pTargetW = widgetAtPos(droppedPoint);
    if(NULL != pTargetW)
    {
        if(mpFavoriteBtn == pTargetW)
        {
            onActionFavorite();
        }
        else if(mpRemoveFavoriteBtn == pTargetW)
        {
            onActionRemoveFavorite();
        }
        else if(mpDeleteBtn == pTargetW)
        {
            if(mpDeleteBtn->isEnabled())
            {
                onActionTrash();
            }
        }
        else if(mpSocialBtn == pTargetW)
        {
            onActionSocial();
        }
    }
    event->acceptProposedAction();
}

/**
 * \brief Get the widget at the given position
 * @param p as the given position
 * @return a pointer on the related QWidget
 */
QWidget* UBLibActionBar::widgetAtPos(const QPoint p)
{
    Q_ASSERT(mpDeleteBtn != NULL);
    Q_ASSERT(mpFavoriteBtn != NULL);
    Q_ASSERT(mpRemoveFavoriteBtn != NULL);
    Q_ASSERT(mpSocialBtn != NULL);

    QList<UBActionButton*> qlBttns;
    qlBttns << mpFavoriteBtn;
    qlBttns << mpDeleteBtn;
    qlBttns << mpRemoveFavoriteBtn;
    qlBttns << mpSocialBtn;

    foreach(UBActionButton* bt, qlBttns)
    {
        if(bt->pos().x() <= p.x() &&
           bt->pos().x() + bt->rect().width() >= p.x() &&
           bt->pos().y() <= p.y() &&
           bt->pos().y() + bt->rect().height() >= p.y())
        {
            if(bt->isVisible())
            {
                return bt;
            }
        }
    }

    // No interesting button has been found
    return NULL;
}

/**
 * \brief Handles the text changed event of the search bar
 */
void UBLibActionBar::onSearchTextChanged(QString txt)
{
    Q_UNUSED(txt);
    onActionSearch();
}

/**
 * \brief Add a new folder
 */
void UBLibActionBar::onActionNewFolder()
{
    emit newFolderToCreate();
}

/**
 * \brief Update the action bar elements
 * @param crntElem as the current element
 */
void UBLibActionBar::onNavigbarUpdate(UBLibElement *crntElem)
{
    if(NULL != crntElem)
    {
        if(crntElem->type() == eUBLibElementType_Folder)
        {
            mpNewFolderBtn->setEnabled(true);
        }
        else
        {
            mpNewFolderBtn->setEnabled(false);
        }
    }
}

/**
 * \brief Construtor
 * @param parent as the parent widget
 * @param action as the related action
 * @param name as the related object name
 */
UBActionButton::UBActionButton(QWidget *parent, QAction* action, const char *name):QToolButton(parent)
{
    setObjectName(name);
    addAction(action);
    setDefaultAction(action);
    setIconSize(QSize(BUTTON_SIZE, BUTTON_SIZE));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    setFocusPolicy(Qt::NoFocus);
}

/**
 * \brief Destructor
 */
UBActionButton::~UBActionButton()
{

}
