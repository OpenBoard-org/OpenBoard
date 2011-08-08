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
#ifndef UBLIBRARYWIDGET_H
#define UBLIBRARYWIDGET_H

#include <QList>
#include <QVector>
#include <QGraphicsItem>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "UBThumbnailWidget.h"
#include "board/UBLibraryController.h"

class UBLibraryWidget : public UBThumbnailWidget
{
    Q_OBJECT
public:
    UBLibraryWidget(QWidget* parent=0, const char* name="UBLibraryWidget");
    ~UBLibraryWidget();

    UBChainedLibElement* chainedElements;
    void setCurrentElemsAndRefresh(UBChainedLibElement* elem);

    void updateThumbnailsSize(int newSize);
    void init();

public slots:
    void onRefreshCurrentFolder();
    void onElementsDropped(QList<QString> elements, UBLibElement* target);
    void onSearchElement(QString elem);
    void onNewFolderToCreate();
    void onDropMe(const QMimeData* _data);
signals:
    void navigBarUpdate(UBLibElement* pElem);
    void itemsSelected(QList<UBLibElement*> elemList, bool inTrash);
    void propertiesRequested(UBLibElement* pElem);
    void favoritesEntered(bool bFav);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void onItemClicked(QGraphicsItem* pItem, int index);
    void onSelectionChanged();

private:
    void refreshView();
    void generateItems();
    void appendChainedElement(UBChainedLibElement* element, UBChainedLibElement* toElem);
    UBLibElement* elementAt(QPoint p);
    UBLibElement* elementFromFilePath(const QString& filePath);
    UBLibraryController* libraryController();

    UBLibElement* mpCrntDir;
    UBLibElement* mpCrntElem;
    QList<UBLibElement*> mCurrentElems;
    QList<UBLibElement*> mOrigCurrentElems;
    QList<QGraphicsItem*> mItems;
    QString mCrntPath;

};

class UBNewFolderDlg : public QDialog
{
public:
    UBNewFolderDlg(QWidget* parent=0, const char* name="NewFolderDlg");
    ~UBNewFolderDlg();

    QString folderName();

private:
    QLabel* mpLabel;
    QLineEdit* mpLineEdit;
    QDialogButtonBox* mpButtons;
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpHLayout;
};

#endif // UBLIBRARYWIDGET_H
