#include "UBBookmarksWidget.h"
#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QVariantMap>
#include <QInputDialog>

UBBookmarksWidget::UBBookmarksWidget(QWidget *parent, const char *name)
    : UBDockPaletteWidget(parent)
{
    setObjectName(name);
    mName = "Bookmarks";
    mVisibleState = true;
    mIconToRight = QPixmap(":images/toolbar/bookmarks.png");
    mIconToLeft = QPixmap(":images/toolbar/bookmarks.png");

    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    mAddButton = new QPushButton(tr("Add Bookmark"), this);
    mRenameButton = new QPushButton(tr("Rename"), this);
    mRemoveButton = new QPushButton(tr("Remove"), this);
    btnLayout->addWidget(mAddButton);
    btnLayout->addWidget(mRenameButton);
    btnLayout->addWidget(mRemoveButton);

    mBookmarksList = new QListWidget(this);
    mBookmarksList->setStyleSheet(QString("QListView { background-color: transparent; color: white; font-weight: bold; font-size: 14px; border: 1px solid white; }"));

    mLayout->addLayout(btnLayout);
    mLayout->addWidget(mBookmarksList);
    
    connect(mAddButton, SIGNAL(clicked()), this, SLOT(onAddButtonClicked()));
    connect(mRenameButton, SIGNAL(clicked()), this, SLOT(onRenameBookmarkClicked()));
    connect(mRemoveButton, SIGNAL(clicked()), this, SLOT(onRemoveBookmarkClicked()));
    connect(mBookmarksList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));
}

UBBookmarksWidget::~UBBookmarksWidget()
{
}

void UBBookmarksWidget::onDocumentSet(std::shared_ptr<UBDocumentProxy> documentProxy)
{
    mCurrentDocument = documentProxy;
    updateBookmarks();
}

void UBBookmarksWidget::activeSceneChanged()
{
    if (UBApplication::boardController) {
        mCurrentDocument = UBApplication::boardController->selectedDocument();
    }
    updateBookmarks();
}

void UBBookmarksWidget::updateBookmarks()
{
    mBookmarksList->clear();
    if (!mCurrentDocument) return;

    QString bookmarksJson = mCurrentDocument->metaData(UBSettings::documentBookmarks).toString();
    if (bookmarksJson.isEmpty()) return;

    QJsonDocument doc = QJsonDocument::fromJson(bookmarksJson.toUtf8());
    if (doc.isNull() || !doc.isArray()) return;

    QJsonArray array = doc.array();
    for (int i = 0; i < array.size(); ++i) {
        QJsonObject obj = array[i].toObject();
        QString name = obj.value("name").toString(QString("Bookmark %1").arg(i+1));
        
        QListWidgetItem* item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, obj.toVariantMap());
        mBookmarksList->addItem(item);
    }
}

void UBBookmarksWidget::onAddButtonClicked()
{
    emit addBookmarkClicked();
}

void UBBookmarksWidget::onRenameBookmarkClicked()
{
    QListWidgetItem* current = mBookmarksList->currentItem();
    if (!current || !mCurrentDocument) return;
    
    QVariantMap map = current->data(Qt::UserRole).toMap();
    QString targetId = map["id"].toString();
    
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename Bookmark"),
                                            tr("New name:"), QLineEdit::Normal,
                                            current->text(), &ok);
    if (ok && !newName.isEmpty()) {
        QString bookmarksJson = mCurrentDocument->metaData(UBSettings::documentBookmarks).toString();
        QJsonDocument doc = QJsonDocument::fromJson(bookmarksJson.toUtf8());
        QJsonArray array = doc.array();
        QJsonArray newArray;
        
        for (int i = 0; i < array.size(); ++i) {
            QJsonObject obj = array[i].toObject();
            if (obj["id"].toVariant().toString() == targetId) {
                obj["name"] = newName;
            }
            newArray.append(obj);
        }
        
        mCurrentDocument->setMetaData(UBSettings::documentBookmarks, QJsonDocument(newArray).toJson(QJsonDocument::Compact));
        updateBookmarks();
    }
}

void UBBookmarksWidget::onRemoveBookmarkClicked()
{
    QListWidgetItem* current = mBookmarksList->currentItem();
    if (!current || !mCurrentDocument) return;
    
    QVariantMap map = current->data(Qt::UserRole).toMap();
    QString targetId = map["id"].toString();

    QString bookmarksJson = mCurrentDocument->metaData(UBSettings::documentBookmarks).toString();
    QJsonDocument doc = QJsonDocument::fromJson(bookmarksJson.toUtf8());
    QJsonArray array = doc.array();
    QJsonArray newArray;
    
    for (int i = 0; i < array.size(); ++i) {
        if (array[i].toObject()["id"].toVariant().toString() != targetId) {
            newArray.append(array[i]);
        }
    }
    
    mCurrentDocument->setMetaData(UBSettings::documentBookmarks, QJsonDocument(newArray).toJson(QJsonDocument::Compact));
    updateBookmarks();
}

void UBBookmarksWidget::onItemClicked(QListWidgetItem* item)
{
    if (item) {
        QVariantMap map = item->data(Qt::UserRole).toMap();
        emit bookmarkClicked(map);
    }
}
