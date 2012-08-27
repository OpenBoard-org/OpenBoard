/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBFEATURESACTIONBAR_H
#define UBFEATURESACTIONBAR_H

#include <QWidget>
#include <QToolButton>
#include <QDropEvent>
#include "board/UBFeaturesController.h"

#define BUTTON_SIZE 24
#define ACTIONBAR_HEIGHT 42

class UBFeaturesMimeData;

typedef enum
{
    eButtonSet_Default,
    eButtonSet_Properties,
    eButtonSet_Favorite
} eButtonSet;

class UBActionButton : public QToolButton
{
public:
    UBActionButton(QWidget* parent=0, QAction* action=0, const char* name="UBActionButton");
    ~UBActionButton();
};

enum UBFeaturesActionBarState
{
	IN_ROOT,
	IN_FOLDER,
	IN_PROPERTIES,
	IN_FAVORITE,
	IN_TRASH
};

class UBFeaturesActionBar : public QWidget
{
	Q_OBJECT
public:
	UBFeaturesActionBar(UBFeaturesController *controller, QWidget* parent=0, const char* name="UBFeaturesActionBar");
    ~UBFeaturesActionBar();
	
	void setCurrentState( UBFeaturesActionBarState state );
    void cleanText(){ mSearchBar->clear(); }

signals:
	void searchElement(const QString &text);
	void newFolderToCreate();
    void deleteElements(const UBFeaturesMimeData *data);
    void addToFavorite(const UBFeaturesMimeData *data);
    void removeFromFavorite(const UBFeaturesMimeData *data);
    void addElementsToFavorite();
    void removeElementsFromFavorite();
    void deleteSelectedElements();
    void rescanModel();

private slots:
	void onSearchTextChanged(QString txt);
	void onActionNewFolder();
    void onActionFavorite();
    void onActionRemoveFavorite();
    void onActionTrash();
    void onActionRescanModel();
    void lockIt();
    void unlockIt();

protected:
    void dragEnterEvent( QDragEnterEvent *event );
	void dropEvent( QDropEvent *event );

private:
	void setButtons();
	UBFeaturesController *featuresController;
	UBFeaturesActionBarState currentState;

	eButtonSet mCrntButtonSet;
    eButtonSet mPreviousButtonSet;

	QButtonGroup* mButtonGroup;
    QLineEdit* mSearchBar;
    QHBoxLayout* mLayout;
    QAction* mpFavoriteAction;
    QAction* mpSocialAction;
    QAction* mpRescanModelAction;
    QAction* mpDeleteAction;
    QAction* mpSearchAction;
    QAction* mpCloseAction;
    QAction* mpRemoveFavorite;
    QAction* mpNewFolderAction;
    UBActionButton* mpFavoriteBtn;
    UBActionButton* mpSocialBtn;
    UBActionButton* mpRescanModelBtn;
    UBActionButton* mpDeleteBtn;
    UBActionButton* mpCloseBtn;
    UBActionButton* mpRemoveFavoriteBtn;
    UBActionButton* mpNewFolderBtn;
};

#endif
