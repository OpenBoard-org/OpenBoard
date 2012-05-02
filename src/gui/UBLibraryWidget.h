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

class UBLibraryController;
class UBChainedLibElement;
class UBLibElement;

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

    UBLibraryController* libraryController() {return mLibraryController;}
	bool isLoadingLibraryItems() const { return mLoadingLibraryItems; }

public slots:
    void onRefreshCurrentFolder();
    void onElementsDropped(QList<QString> elements, UBLibElement* target);
    void onSearchElement(QString elem);
    void onNewFolderToCreate();
    void onDropMe(const QMimeData* _data);
    void onAddDownloadedFileToLibrary(bool pSuccess, QUrl sourceUrl, QString pContentHeader, QByteArray pData);

signals:
    void navigBarUpdate(UBLibElement* pElem);
    void itemsSelected(QList<UBLibElement*> elemList, bool inTrash);
    void propertiesRequested(UBLibElement* pElem);
    void displaySearchEngine(UBLibElement* pElem);
    void favoritesEntered(bool bFav);
    void itemClicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void onItemClicked(QGraphicsItem* pItem, int index);
    void onSelectionChanged();
    void onDisplayMetadata(QMap<QString,QString> metadatas);


private:

    void refreshView();
    void generateItems();
    void appendChainedElement(UBChainedLibElement* element, UBChainedLibElement* toElem);

    UBLibElement* elementAt(QPoint p);
    UBLibElement* elementFromFilePath(const QString& filePath);
    UBLibraryController* mLibraryController;

    UBLibElement* mpCrntDir;
    UBLibElement* mpCrntElem;
    UBLibElement* mpTmpElem;
    QList<UBLibElement*> mCurrentElems;
    QList<UBLibElement*> mOrigCurrentElems;
    QList<QGraphicsItem*> mItems;
	bool mLoadingLibraryItems;
};

class UBNewFolderDlg : public QDialog
{
    Q_OBJECT

public:
    UBNewFolderDlg(QWidget* parent=0, const char* name="NewFolderDlg");
    ~UBNewFolderDlg();

    QString folderName();

public slots:
    void text_Changed(const QString &);
    void text_Edited(const QString &);


private:
    QLabel* mpLabel;
    QLineEdit* mpLineEdit;
    QDialogButtonBox* mpButtons;
    QPushButton* mpAddButton;
    QPushButton* mpCancelButton;
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpHLayout;
};

#endif // UBLIBRARYWIDGET_H
