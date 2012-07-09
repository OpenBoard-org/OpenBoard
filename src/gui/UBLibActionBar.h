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
#ifndef UBLIBACTIONBAR_H
#define UBLIBACTIONBAR_H

#include <QWidget>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QList>
#include <QAction>
#include <QLineEdit>
#include <QToolButton>
#include <QStackedWidget>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>

#include "board/UBLibraryController.h"

#define BUTTON_SIZE 24
#define ACTIONBAR_HEIGHT 42

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

class UBLibActionBar : public QWidget
{
    Q_OBJECT
public:
    UBLibActionBar(QWidget* parent=0, const char* name="UBLibActionBar");
    ~UBLibActionBar();

    void setButtons(eButtonSet setID);
    eButtonSet previousButtonSet(){return mPreviousButtonSet;}

signals:
    void deleteDone();
    void showFolderContent();
    void searchElement(QString text);
    void newFolderToCreate();

public slots:
    void onSelectionChanged(QList<UBLibElement*> itemList, bool isInTrash);
    void onFavoritesEntered(bool bFav);
    void onNavigbarUpdate(UBLibElement* crntElem);
    void onItemChanged();

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void onActionFavorite();
    void onActionSocial();
    void onActionSearch();
    void onActionTrash();
    void onActionClose();
    void onActionRemoveFavorite();
    void onSearchTextChanged(QString txt);
    void onActionNewFolder();

private:
    QWidget* widgetAtPos(const QPoint p);
    UBLibraryController* libraryController();

    eButtonSet mCrntButtonSet;
    eButtonSet mPreviousButtonSet;
    QButtonGroup* mButtonGroup;
    QList<UBLibElement*> mSelectedElements;
    QLineEdit* mSearchBar;
    QHBoxLayout* mLayout;
    QAction* mpFavoriteAction;
    QAction* mpSocialAction;
    QAction* mpDeleteAction;
    QAction* mpSearchAction;
    QAction* mpCloseAction;
    QAction* mpRemoveFavorite;
    QAction* mpNewFolderAction;
    UBActionButton* mpFavoriteBtn;
    UBActionButton* mpSocialBtn;
    UBActionButton* mpDeleteBtn;
    UBActionButton* mpSearchBtn;
    UBActionButton* mpCloseBtn;
    UBActionButton* mpRemoveFavoriteBtn;
    UBActionButton* mpNewFolderBtn;

    // This flag will be removed after the test, normally we should ask
    // the selected item if it is in favorite or not
    bool bFavorite;
    bool bIsInTrash;

};

#endif // UBLIBACTIONBAR_H
