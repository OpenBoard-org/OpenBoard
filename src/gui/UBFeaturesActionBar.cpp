#include "UBFeaturesActionBar.h"

UBFeaturesActionBar::UBFeaturesActionBar( UBFeaturesController *controller, QWidget* parent, const char* name ) : QWidget (parent)
	, featuresController(controller)
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
    , mpCloseBtn(NULL)
    , mpRemoveFavoriteBtn(NULL)
    , mpNewFolderBtn(NULL)
{
	setObjectName(name);
    setStyleSheet(QString("background: #EEEEEE; border-radius : 10px; border : 2px solid #999999;"));

    setAcceptDrops(true);

    mButtonGroup = new QButtonGroup(this);
    mSearchBar = new QLineEdit(this);
    mSearchBar->setStyleSheet(QString("background-color:white; border-radius : 10px; padding : 2px;"));
    //connect(mSearchBar, SIGNAL(returnPressed()), this, SLOT(onActionSearch()));

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
    //mpSearchBtn = new UBActionButton(this, mpSearchAction);
    mpDeleteBtn = new UBActionButton(this, mpDeleteAction);
    mpCloseBtn = new UBActionButton(this, mpCloseAction);
    mpRemoveFavoriteBtn = new UBActionButton(this, mpRemoveFavorite);
    mpNewFolderBtn = new UBActionButton(this, mpNewFolderAction);

    // Initialize the buttons
    //mpSearchBtn->setEnabled(false);
    mpNewFolderBtn->setEnabled(false);

    // Add the buttons to the button group
    mButtonGroup->addButton(mpFavoriteBtn);
    mButtonGroup->addButton(mpSocialBtn);
    //mButtonGroup->addButton(mpSearchBtn);
    mButtonGroup->addButton(mpDeleteBtn);
    mButtonGroup->addButton(mpCloseBtn);
    mButtonGroup->addButton(mpRemoveFavoriteBtn);
    mButtonGroup->addButton(mpNewFolderBtn);
    // Connect signals & slots
    /*connect(mpFavoriteAction,SIGNAL(triggered()), this, SLOT(onActionFavorite()));
    connect(mpSocialAction,SIGNAL(triggered()), this, SLOT(onActionSocial()));
    connect(mpSearchAction,SIGNAL(triggered()), this, SLOT(onActionSearch()));
    connect(mpDeleteAction,SIGNAL(triggered()), this, SLOT(onActionTrash()));
    connect(mpCloseAction, SIGNAL(triggered()), this, SLOT(onActionClose()));
    connect(mpRemoveFavorite, SIGNAL(triggered()), this, SLOT(onActionRemoveFavorite()));
    connect(mSearchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(mpNewFolderAction, SIGNAL(triggered()), this, SLOT(onActionNewFolder()));*/

	connect(mSearchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
	connect(mpNewFolderAction, SIGNAL(triggered()), this, SLOT(onActionNewFolder()));

    // Build the default toolbar
    mLayout->addWidget(mpFavoriteBtn);
    mLayout->addWidget(mpSocialBtn);
    mLayout->addWidget(mpNewFolderBtn);
    mLayout->addWidget(mSearchBar);
    //mLayout->addWidget(mpSearchBtn);
    mLayout->addWidget(mpDeleteBtn);
    mLayout->addWidget(mpCloseBtn);
    mLayout->addWidget(mpRemoveFavoriteBtn);
	setCurrentState( IN_ROOT );
	mpDeleteBtn->setAcceptDrops(true);
	setAcceptDrops( true );
}

void UBFeaturesActionBar::setCurrentState( UBFeaturesActionBarState state )
{
	currentState = state;
	setButtons();
}

void UBFeaturesActionBar::setButtons()
{
    switch( currentState )
    {
	case IN_FOLDER:
		mpFavoriteBtn->show();
        mpSocialBtn->hide();
        mSearchBar->show();
        mpDeleteBtn->show();
        mpCloseBtn->hide();
        mpRemoveFavoriteBtn->hide();
        mpNewFolderBtn->show();
		mpNewFolderBtn->setEnabled(true);
		mpDeleteBtn->setEnabled(true);
		break;
    case IN_ROOT:
        mpFavoriteBtn->show();
        mpSocialBtn->hide();
        mSearchBar->show();
        mpDeleteBtn->show();
        mpCloseBtn->hide();
        mpRemoveFavoriteBtn->hide();
        mpNewFolderBtn->show();
		mpNewFolderBtn->setEnabled(false);
		mpDeleteBtn->setEnabled(false);
        break;
    case IN_PROPERTIES:
        mpFavoriteBtn->show();
        mpSocialBtn->hide();
        mSearchBar->show();
        //mpSearchBtn->show();
        mpDeleteBtn->hide();
        mpCloseBtn->hide();
        mpRemoveFavoriteBtn->hide();
        mpNewFolderBtn->hide();
        break;
    case IN_FAVORITE:
        mpFavoriteBtn->hide();
        mpSocialBtn->hide();
        mSearchBar->show();
        //mpSearchBtn->show();
        mpDeleteBtn->hide();
        mpCloseBtn->hide();
        mpRemoveFavoriteBtn->show();
        mpNewFolderBtn->hide();
        break;
    default:
        break;
    }
}

void UBFeaturesActionBar::onSearchTextChanged(QString txt)
{
    emit searchElement(mSearchBar->text());
}
   
void UBFeaturesActionBar::onActionNewFolder()
{
    emit newFolderToCreate();
}

/*
void UBFeaturesActionBar::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}
*/

void UBFeaturesActionBar::dragEnterEvent( QDragEnterEvent *event )
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void UBFeaturesActionBar::dropEvent( QDropEvent *event )
{	
	QWidget *dest = childAt( event->pos() );
	if ( dest == mpDeleteBtn )
	{
		event->setDropAction( Qt::MoveAction );
		event->accept();
		emit deleteElements( *event->mimeData() );
	}
	if ( dest == mpFavoriteBtn )
	{
		event->setDropAction( Qt::CopyAction );
		event->accept();
		emit addToFavorite( *event->mimeData() );
	}

}

UBFeaturesActionBar::~UBFeaturesActionBar()
{
}