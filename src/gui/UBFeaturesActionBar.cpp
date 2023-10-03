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
    , mpAddToFavoritesAction(NULL)
    , mpRemoveFromFavoritesAction(NULL)
    , mpSocialAction(NULL)
    , mpRescanModelAction(NULL)
    , mpDeleteAction(NULL)
    , mpSearchAction(NULL)
    , mpCloseAction(NULL)
    , mpNewFolderAction(NULL)
    , mpAddToFavoritesBtn(NULL)
    , mpRemoveFromFavoritesBtn(NULL)
    , mpSocialBtn(NULL)
    , mpRescanModelBtn(NULL)
    , mpDeleteBtn(NULL)
    , mpCloseBtn(NULL)
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
    mpAddToFavoritesAction = new QAction(QPixmap(":/images/addToFavorites.png"), tr("Add to favorites"), this);
    mpRemoveFromFavoritesAction = new QAction(QPixmap(":/images/removeFromFavorites.png"), tr("Remove from favorites"), this);
    mpSocialAction = new QAction(QIcon(":/images/libpalette/social.png"), tr("Share"), this);
    mpSearchAction = new QAction(QIcon(":/images/libpalette/miniSearch.png"), tr("Search"), this);
    mpRescanModelAction = new QAction(QIcon(":/images/cursors/rotate.png"), tr("Rescan file system"), this);
    mpDeleteAction = new QAction(QIcon(":/images/libpalette/miniTrash.png"), tr("Delete"), this);
    mpCloseAction = new QAction(QIcon(":/images/close.svg"), tr("Back to folder"), this);
    mpNewFolderAction = new QAction(QIcon(":/images/libpalette/miniNewFolder.png"), tr("Create new folder"), this);

    // Create the buttons
    mpAddToFavoritesBtn = new UBActionButton(this, mpAddToFavoritesAction);
    mpRemoveFromFavoritesBtn = new UBActionButton(this, mpRemoveFromFavoritesAction);

    mpSocialBtn = new UBActionButton(this, mpSocialAction);

    //mpSearchBtn = new UBActionButton(this, mpSearchAction);
    mpRescanModelBtn = new UBActionButton(this, mpRescanModelAction);
    mpRescanModelBtn->hide();

    mpDeleteBtn = new UBActionButton(this, mpDeleteAction);
    mpCloseBtn = new UBActionButton(this, mpCloseAction);
    mpNewFolderBtn = new UBActionButton(this, mpNewFolderAction);

    // Initialize the buttons
    //mpSearchBtn->setEnabled(false);
    mpNewFolderBtn->setEnabled(false);

    // Add the buttons to the button group
    mButtonGroup->addButton(mpAddToFavoritesBtn);
    mButtonGroup->addButton(mpRemoveFromFavoritesBtn);
    mButtonGroup->addButton(mpSocialBtn);
    //mButtonGroup->addButton(mpSearchBtn);
    mButtonGroup->addButton(mpDeleteBtn);
    mButtonGroup->addButton(mpCloseBtn);
    mButtonGroup->addButton(mpNewFolderBtn);

    // Connect signals & slots
    connect(mpAddToFavoritesAction,SIGNAL(triggered()), this, SLOT(onActionAddToFavorites()));
    connect(mpRemoveFromFavoritesAction,SIGNAL(triggered()), this, SLOT(onActionRemoveFromFavorites()));
    connect(mSearchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(mpNewFolderAction, SIGNAL(triggered()), this, SLOT(onActionNewFolder()));
    connect(mpRescanModelAction, SIGNAL(triggered()), this , SLOT(onActionRescanModel()));
    connect(mpDeleteAction,SIGNAL(triggered()), this, SLOT(onActionTrash()));


    // Build the default toolbar
    mLayout->addWidget(mpAddToFavoritesBtn);
    mLayout->addWidget(mpRemoveFromFavoritesBtn);
    mLayout->addWidget(mpSocialBtn);
    mLayout->addWidget(mpNewFolderBtn);
    mLayout->addWidget(mSearchBar);
    //mLayout->addWidget(mpSearchBtn);
    mLayout->addWidget(mpRescanModelBtn);
    mLayout->addWidget(mpDeleteBtn);
    mLayout->addWidget(mpCloseBtn);
    setCurrentState( IN_ROOT );
    mpDeleteBtn->setAcceptDrops(true);
    setAcceptDrops( true );
}

void UBFeaturesActionBar::setCurrentState( UBFeaturesActionBarState state )
{
    currentState = state;
    setButtons();
}

void UBFeaturesActionBar::updateButtons(UBFeature feature)
{
    if (!feature.isFolder())
    {
        QString featureFullPath = feature.getFullPath().toString();
        if (featureFullPath.endsWith('.rdf'))
        {
            QString documentFoldername = featureFullPath.section('/', -2, -2); //section before "/metadata.rdf" is documentFolderName
            if (featuresController->isDocumentInFavoriteList(documentFoldername))
            {
                mpRemoveFromFavoritesBtn->show();
                mpAddToFavoritesBtn->hide();
            }
            else
            {
                mpAddToFavoritesBtn->show();
                mpRemoveFromFavoritesBtn->hide();
            }
        }
        else
        {
            if (featuresController->isInFavoriteList(feature.getFullPath()))
            {
                mpRemoveFromFavoritesBtn->show();
                mpAddToFavoritesBtn->hide();
            }
            else
            {
                mpAddToFavoritesBtn->show();
                mpRemoveFromFavoritesBtn->hide();
            }
        }
    }
    else
    {
        mpAddToFavoritesBtn->hide();
        mpRemoveFromFavoritesBtn->hide();
    }
}

void UBFeaturesActionBar::setButtons()
{
    switch( currentState )
    {
    case IN_FOLDER:
        mpSocialBtn->hide();
        mSearchBar->show();
        mpDeleteBtn->show();
        mpCloseBtn->hide();
        mpNewFolderBtn->show();
        mpNewFolderBtn->setEnabled(true);
        mpDeleteBtn->setEnabled(true);
        break;
    case IN_ROOT:
        mpAddToFavoritesBtn->hide();
        mpRemoveFromFavoritesBtn->hide();
        mpSocialBtn->hide();
        mSearchBar->show();
        mpDeleteBtn->show();
        mpCloseBtn->hide();
        mpNewFolderBtn->show();
        mpNewFolderBtn->setEnabled(false);
        mpDeleteBtn->setEnabled(false);
        break;
    case IN_PROPERTIES:
        mpSocialBtn->hide();
        mSearchBar->show();
        mpDeleteBtn->hide();
        mpCloseBtn->hide();
        mpNewFolderBtn->hide();
        break;
    case IN_FAVORITE:
        mpSocialBtn->hide();
        mSearchBar->show();
        mpDeleteBtn->hide();
        mpCloseBtn->hide();
        mpNewFolderBtn->hide();
        break;
    case IN_TRASH:
        mpSocialBtn->hide();
        mSearchBar->show();
        mpDeleteBtn->show();
        mpDeleteBtn->setEnabled(true);
        mpCloseBtn->hide();
        mpNewFolderBtn->hide();
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

void UBFeaturesActionBar::onActionAddToFavorites()
{
    emit addElementsToFavorite();

    mpAddToFavoritesBtn->hide();
    mpRemoveFromFavoritesBtn->show();
}

void UBFeaturesActionBar::onActionRemoveFromFavorites()
{
    emit removeElementsFromFavorite();

    mpAddToFavoritesBtn->show();
    mpRemoveFromFavoritesBtn->hide();
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

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QPoint eventPos = event->position().toPoint();
#else
    QPoint eventPos = event->pos();
#endif
    QWidget *dest = childAt(eventPos);
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
