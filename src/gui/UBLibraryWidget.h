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
