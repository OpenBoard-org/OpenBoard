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


#include <QButtonGroup>
#include <QAction>

#include "UBFeaturesActionBar.h"
#include "core/memcheck.h"
#include "gui/UBFeaturesWidget.h"

UBFeaturesActionBar::UBFeaturesActionBar( UBFeaturesController *controller, QWidget* parent, const char* name ) : QWidget (parent)
    , featuresController(controller)
    , mButtonGroup(NULL)
    , mSearchBar(NULL)
    , mLayout(NULL)
    , mpFavoriteAction(NULL)
    , mpSocialAction(NULL)
    , mpRescanModelAction(NULL)
    , mpDeleteAction(NULL)
    , mpSearchAction(NULL)
    , mpCloseAction(NULL)
    , mpRemoveFavorite(NULL)
    , mpNewFolderAction(NULL)
    , mpFavoriteBtn(NULL)
    , mpSocialBtn(NULL)
    , mpRescanModelBtn(NULL)
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

    mLayout = new QHBoxLayout();
    setLayout(mLayout);

    setMaximumHeight(ACTIONBAR_HEIGHT);

    // Create the actions
    mpFavoriteAction = new QAction(QIcon(":/images/libpalette/miniFavorite.png"), tr("Add to favorites"), this);
    mpSocialAction = new QAction(QIcon(":/images/libpalette/social.png"), tr("Share"), this);
    mpSearchAction = new QAction(QIcon(":/images/libpalette/miniSearch.png"), tr("Search"), this);
    mpRescanModelAction = new QAction(QIcon(":/images/cursors/rotate.png"), tr("Rescan file system"), this);
    mpDeleteAction = new QAction(QIcon(":/images/libpalette/miniTrash.png"), tr("Delete"), this);
    mpCloseAction = new QAction(QIcon(":/images/close.svg"), tr("Back to folder"), this);
    mpRemoveFavorite = new QAction(QIcon(":/images/libpalette/trash_favorite.svg"), tr("Remove from favorites"), this);
    mpNewFolderAction = new QAction(QIcon(":/images/libpalette/miniNewFolder.png"), tr("Create new folder"), this);

    // Create the buttons
    mpFavoriteBtn = new UBActionButton(this, mpFavoriteAction);
    mpSocialBtn = new UBActionButton(this, mpSocialAction);

    //mpSearchBtn = new UBActionButton(this, mpSearchAction);
    mpRescanModelBtn = new UBActionButton(this, mpRescanModelAction);
    mpRescanModelBtn->hide();

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

    connect(mpFavoriteAction,SIGNAL(triggered()), this, SLOT(onActionFavorite()));
    connect(mSearchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(mpNewFolderAction, SIGNAL(triggered()), this, SLOT(onActionNewFolder()));
    connect(mpRemoveFavorite, SIGNAL(triggered()), this, SLOT(onActionRemoveFavorite()));
    connect(mpRescanModelAction, SIGNAL(triggered()), this , SLOT(onActionRescanModel()));
    connect(mpDeleteAction,SIGNAL(triggered()), this, SLOT(onActionTrash()));


    // Build the default toolbar
    mLayout->addWidget(mpFavoriteBtn);
    mLayout->addWidget(mpSocialBtn);
    mLayout->addWidget(mpNewFolderBtn);
    mLayout->addWidget(mSearchBar);
    //mLayout->addWidget(mpSearchBtn);
    mLayout->addWidget(mpRescanModelBtn);
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
//        mpRescanModelBtn->show();
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
//        mpRescanModelBtn->show();
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
//        mpRescanModelBtn->hide();
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
//        mpRescanModelBtn->hide();
        break;
    case IN_TRASH:
        mpFavoriteBtn->hide();
        mpSocialBtn->hide();
        mSearchBar->show();
        mpDeleteBtn->show();
        mpDeleteBtn->setEnabled(true);
        //mpSearchBtn->show();
        //mpDeleteBtn->hide();
        mpCloseBtn->hide();
        //mpRemoveFavoriteBtn->show();
        mpNewFolderBtn->hide();
//        mpRescanModelBtn->hide();
        break;
    default:
        break;
    }
}

void UBFeaturesActionBar::onSearchTextChanged(QString txt)
{
    Q_UNUSED(txt)
    emit searchElement(mSearchBar->text());
}
   
void UBFeaturesActionBar::onActionNewFolder()
{
    emit newFolderToCreate();
}

void UBFeaturesActionBar::onActionFavorite()
{
    emit addElementsToFavorite();
}

void UBFeaturesActionBar::onActionRemoveFavorite()
{
    emit removeElementsFromFavorite();
}

void UBFeaturesActionBar::onActionTrash()
{
    emit deleteSelectedElements();
}
void UBFeaturesActionBar::onActionRescanModel()
{
    emit rescanModel();
}

void UBFeaturesActionBar::lockIt()
{
    setEnabled(false);
}

void UBFeaturesActionBar::unlockIt()
{
    setEnabled(true);
}

void UBFeaturesActionBar::dragEnterEvent( QDragEnterEvent *event )
{
    const UBFeaturesMimeData *fMimeData = qobject_cast<const UBFeaturesMimeData*>(event->mimeData());
    if (fMimeData) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void UBFeaturesActionBar::dropEvent(QDropEvent *event)
{    
    const UBFeaturesMimeData *fMimeData = qobject_cast<const UBFeaturesMimeData*>(event->mimeData());

    if (!fMimeData) {
        qWarning() << "data came from not supported widget";
        event->ignore();
        return;
    }

    QWidget *dest = childAt(event->pos());
    if (dest == mpDeleteBtn) {
        QList<UBFeature> featuresList = fMimeData->features();
        foreach (UBFeature curFeature, featuresList) {
            if (!curFeature.isDeletable()) {
                qWarning() << "Undeletable feature found, stopping deleting process";
                event->ignore();
                return;
            }
        }
        event->setDropAction(Qt::MoveAction);
        event->accept();

        emit deleteElements(fMimeData);

    } else if (dest == mpFavoriteBtn) {
        event->setDropAction( Qt::CopyAction);
        event->accept();

        emit addToFavorite(fMimeData);

    } else if (dest == mpRemoveFavoriteBtn) {
        event->setDropAction( Qt::MoveAction );
        event->accept();

        emit removeFromFavorite(fMimeData);
    }
}

UBFeaturesActionBar::~UBFeaturesActionBar()
{
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
