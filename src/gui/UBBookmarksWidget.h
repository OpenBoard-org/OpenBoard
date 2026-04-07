#ifndef UBBOOKMARKSWIDGET_H
#define UBBOOKMARKSWIDGET_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>

#include "UBDockPaletteWidget.h"
#include "document/UBDocumentProxy.h"

class UBBookmarksWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBBookmarksWidget(QWidget* parent=0, const char* name="UBBookmarksWidget");
    ~UBBookmarksWidget();

    bool visibleInMode(eUBDockPaletteWidgetMode mode)
    {
        return mode == eUBDockPaletteWidget_BOARD;
    }

signals:
    void addBookmarkClicked();
    void bookmarkClicked(const QVariantMap& bookmark);

public slots:
    void onDocumentSet(std::shared_ptr<UBDocumentProxy> documentProxy);
    void updateBookmarks();
    void activeSceneChanged();

private slots:
    void onAddButtonClicked();
    void onItemClicked(QListWidgetItem* item);
    void onRenameBookmarkClicked();
    void onRemoveBookmarkClicked();

private:
    QVBoxLayout* mLayout;
    QListWidget* mBookmarksList;
    QPushButton* mAddButton;
    QPushButton* mRenameButton;
    QPushButton* mRemoveButton;
    std::shared_ptr<UBDocumentProxy> mCurrentDocument;
};

#endif // UBBOOKMARKSWIDGET_H
