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


#include "UBDocumentTreeWidget.h"

#include "document/UBDocumentProxy.h"

#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBMimeData.h"
#include "core/UBApplicationController.h"
#include "core/UBDocumentManager.h"
#include "document/UBDocumentController.h"

#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/memcheck.h"

UBDocumentTreeWidget::UBDocumentTreeWidget(QWidget * parent)
    : QTreeWidget(parent)
    , mSelectedProxyTi(0)
    , mDropTargetProxyTi(0)
{
    setDragDropMode(QAbstractItemView::InternalMove);
    setAutoScroll(true);

    mScrollTimer = new QTimer(this);
    connect(UBDocumentManager::documentManager(), SIGNAL(documentUpdated(UBDocumentProxy*))
            , this, SLOT(documentUpdated(UBDocumentProxy*)));

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int))
            , this,  SLOT(itemChangedValidation(QTreeWidgetItem *, int)));
    connect(mScrollTimer, SIGNAL(timeout())
            , this, SLOT(autoScroll()));
}


UBDocumentTreeWidget::~UBDocumentTreeWidget()
{
    // NOOP
}

void UBDocumentTreeWidget::itemChangedValidation(QTreeWidgetItem * item, int column)
{

    if (column == 0)
    {
        UBDocumentGroupTreeItem* group = dynamic_cast<UBDocumentGroupTreeItem *>(item);
        if(group)
        {
            QString name = group->text(0);

            for(int i = 0; i < topLevelItemCount (); i++)
            {
                QTreeWidgetItem *someTopLevelItem = topLevelItem(i);

                if (someTopLevelItem != group &&
                        someTopLevelItem->text(0) == name)
                {
                    group->setText(0, tr("%1 (copy)").arg(name));
                }
            }
        }
    }
}

Qt::DropActions UBDocumentTreeWidget::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}


void UBDocumentTreeWidget::mousePressEvent(QMouseEvent *event)
{
    QTreeWidgetItem* twItem = this->itemAt(event->pos());

    mSelectedProxyTi = dynamic_cast<UBDocumentProxyTreeItem*>(twItem);

    QTreeWidget::mousePressEvent(event);
}


void UBDocumentTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}


void UBDocumentTreeWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);

    if (mScrollTimer->isActive())
    {
        mScrollMagnitude = 0;
        mScrollTimer->stop();
    }

    if (mDropTargetProxyTi)
    {
        mDropTargetProxyTi->setBackground(0, mBackground);
        mDropTargetProxyTi = 0;
    }
}


void UBDocumentTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QRect boundingFrame = frameRect();
    //setting up automatic scrolling
    const int SCROLL_DISTANCE = 4;
    int bottomDist = boundingFrame.bottom() - event->pos().y(), topDist = boundingFrame.top() - event->pos().y();
    if(qAbs(bottomDist) <= SCROLL_DISTANCE)
    {
        mScrollMagnitude = (SCROLL_DISTANCE - bottomDist)*4;
        if(verticalScrollBar()->isVisible() && !mScrollTimer->isActive()) mScrollTimer->start(100);
    }
    else if(qAbs(topDist) <= SCROLL_DISTANCE)
    {
        mScrollMagnitude = (- SCROLL_DISTANCE - topDist)*4;
        if(verticalScrollBar()->isVisible() && !mScrollTimer->isActive()) mScrollTimer->start(100);
    }
    else
    {
        mScrollMagnitude = 0;
        mScrollTimer->stop();
    }


    QTreeWidgetItem* underlyingItem = this->itemAt(event->pos());

    if (event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage))
    {
        UBDocumentProxyTreeItem *targetProxyTreeItem = dynamic_cast<UBDocumentProxyTreeItem*>(underlyingItem);
        if (targetProxyTreeItem && targetProxyTreeItem != mSelectedProxyTi)
        {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        UBDocumentGroupTreeItem *groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(underlyingItem);

        if (groupItem && mSelectedProxyTi && groupItem != mSelectedProxyTi->parent())
            event->acceptProposedAction();
        else
            event->ignore();
    }

    if (event->isAccepted())
    {
        if (mDropTargetProxyTi)
        {
            if (underlyingItem != mDropTargetProxyTi)
            {
                mBackground = underlyingItem->background(0);
                mDropTargetProxyTi->setBackground(0, mBackground);
                mDropTargetProxyTi = underlyingItem;
                mDropTargetProxyTi->setBackground(0, QBrush(QColor("#6682b5")));
            }
        }
        else
        {
            mBackground = underlyingItem->background(0);
            mDropTargetProxyTi = underlyingItem;
            mDropTargetProxyTi->setBackground(0, QBrush(QColor("#6682b5")));
        }
    }
    else if (mDropTargetProxyTi)
    {
        mDropTargetProxyTi->setBackground(0, mBackground);
        mDropTargetProxyTi = 0;
    }
}


void UBDocumentTreeWidget::focusInEvent(QFocusEvent *event)
{
    QTreeWidget::focusInEvent(event);
}

void UBDocumentTreeWidget::dropEvent(QDropEvent *event)
{
    if (mDropTargetProxyTi) {
        mDropTargetProxyTi->setBackground(0, mBackground);
        mDropTargetProxyTi = 0;
    }

    QTreeWidgetItem * underlyingItem = this->itemAt(event->pos());

    // If the destination is a folder, move the selected document(s) there
    UBDocumentGroupTreeItem * destinationFolder = dynamic_cast<UBDocumentGroupTreeItem*>(underlyingItem);

    if (destinationFolder) {
        UBDocumentProxyTreeItem * lastMovedDocument;
        foreach(QTreeWidgetItem * item, this->selectedItems()) {
            UBDocumentProxyTreeItem * document = dynamic_cast<UBDocumentProxyTreeItem*>(item);
            if (document && moveDocument(document, destinationFolder))
                lastMovedDocument = document;
        }

        if (lastMovedDocument) {
            expandItem(destinationFolder);
            scrollToItem(lastMovedDocument);
            setCurrentItem(lastMovedDocument);
            lastMovedDocument->setSelected(true);

            event->setDropAction(Qt::IgnoreAction);
            event->accept();
        }
    }

    // If the destination is a document and the dropped item is a page, copy the page to that document
    else {
        QTreeWidgetItem* underlyingTreeItem = this->itemAt(event->pos());

        UBDocumentProxyTreeItem *targetProxyTreeItem = dynamic_cast<UBDocumentProxyTreeItem*>(underlyingTreeItem);
        if (targetProxyTreeItem && targetProxyTreeItem != mSelectedProxyTi)
        {
            if (event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage))
            {
                event->setDropAction(Qt::CopyAction);
                event->accept();

                const UBMimeData *mimeData = qobject_cast <const UBMimeData*>(event->mimeData());

                if (mimeData && mimeData->items().size() > 0)
                {
                        int count = 0;
                        int total = mimeData->items().size();

                        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

                    foreach (UBMimeDataItem sourceItem, mimeData->items())
                    {
                        count++;

                        UBApplication::applicationController->showMessage(tr("Copying page %1/%2").arg(count).arg(total), true);

                        // TODO UB 4.x Move following code to some controller class
                        UBGraphicsScene *scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(sourceItem.documentProxy(), sourceItem.sceneIndex());
                        if (scene)
                        {
                            UBGraphicsScene* sceneClone = scene->sceneDeepCopy();

                            UBDocumentProxy *targetDocProxy = targetProxyTreeItem->proxy();

                            foreach (QUrl relativeFile, scene->relativeDependencies())
                            {
                                QString source = scene->document()->persistencePath() + "/" + relativeFile.toString();
                                QString target = targetDocProxy->persistencePath() + "/" + relativeFile.toString();

                                QString sourceDecoded = scene->document()->persistencePath() + "/" + relativeFile.toString(QUrl::DecodeReserved);
                                QString targetDecoded = targetDocProxy->persistencePath() + "/" + relativeFile.toString(QUrl::DecodeReserved);

                                if(QFileInfo(source).isDir())
                                    UBFileSystemUtils::copyDir(source,target);
                                else{
                                    QFileInfo fi(targetDecoded);
                                    QDir d = fi.dir();
                                    d.mkpath(d.absolutePath());
                                    QFile::copy(sourceDecoded, targetDecoded);
                                }
                            }

                            UBPersistenceManager::persistenceManager()->insertDocumentSceneAt(targetDocProxy, sceneClone, targetDocProxy->pageCount());

                            //due to incorrect generation of thumbnails of invisible scene I've used direct copying of thumbnail files
                            //it's not universal and good way but it's faster
                            QString from = sourceItem.documentProxy()->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceItem.sceneIndex());
                            QString to  = targetDocProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetDocProxy->pageCount());
                            QFile::remove(to);
                            QFile::copy(from, to);
                          }
                    }

                    QApplication::restoreOverrideCursor();

                    UBApplication::applicationController->showMessage(tr("%1 pages copied", "", total).arg(total), false);
                }
            }
            else
            {
                event->setDropAction(Qt::IgnoreAction);
                event->ignore();
            }
        }
    }
}


void UBDocumentTreeWidget::documentUpdated(UBDocumentProxy *pDocument)
{
    UBDocumentProxyTreeItem *treeItem = UBApplication::documentController->findDocument(pDocument);
    if (treeItem)
    {
        QTreeWidgetItem * parent = treeItem->parent();

        if (parent)
        {
            for (int i = 0; i < parent->indexOfChild(treeItem); i++)
            {
                QTreeWidgetItem *ti = parent->child(i);
                UBDocumentProxyTreeItem* pi = dynamic_cast<UBDocumentProxyTreeItem*>(ti);
                if (pi)
                {
                    if (pDocument->metaData(UBSettings::documentDate).toString() >= pi->proxy()->metaData(UBSettings::documentDate).toString())
                    {
                        bool selected = treeItem->isSelected();
                        parent->removeChild(treeItem);
                        parent->insertChild(i, treeItem);
                        for (int j = 0; j < selectedItems().count(); j++)
                            selectedItems().at(j)->setSelected(false);
                        if (selected)
                            treeItem->setSelected(true);
                        break;
                    }
                }
            }
        }
    }
}

/**
 * @brief Move a document to the specified destination folder
 * @param document Pointer to the document to move
 * @param destinationFolder Pointer to the folder to move the document to
 * @return true if document was moved successfully, false otherwise
 */
bool UBDocumentTreeWidget::moveDocument(UBDocumentProxyTreeItem* document, UBDocumentGroupTreeItem* destinationFolder)
{
    if (!document || !(document->proxy()) || !destinationFolder)
        return false;

    UBDocumentGroupTreeItem * sourceFolder = dynamic_cast<UBDocumentGroupTreeItem*>(document->parent());
    bool documentIsInTrash = (sourceFolder && sourceFolder->isTrashFolder());

    if (documentIsInTrash && destinationFolder->isTrashFolder())
        return false;

    if (!documentIsInTrash && document->proxy()->groupName() == destinationFolder->groupName())
        return false;

    QString destinationFolderName;

    if (destinationFolder->isTrashFolder()) {
        UBApplication::app()->documentController->moveDocumentToTrash(sourceFolder, document, true);
        destinationFolderName = document->proxy()->metaData(UBSettings::documentGroupName).toString();
    }

    else {
        if (destinationFolder->groupName() == UBApplication::app()->documentController->defaultDocumentGroupName())
            destinationFolderName = "";
        else
            destinationFolderName = destinationFolder->groupName();
    }

    // Update the folder name in the document
    document->proxy()->setMetaData(UBSettings::documentGroupName, destinationFolderName);
    UBPersistenceManager::persistenceManager()->persistDocumentMetadata(document->proxy());

    // Remove document from its old folder
    document->parent()->removeChild(document);

    // Insert document at the right spot in the destination folder (ordered by document date)
    int i = 0;
    for (i = 0; i < destinationFolder->childCount(); i++) {
        QTreeWidgetItem *ti = destinationFolder->child(i);
        UBDocumentProxyTreeItem* pi = dynamic_cast<UBDocumentProxyTreeItem*>(ti);
        if (pi && document->proxy()->metaData(UBSettings::documentDate).toString() >= pi->proxy()->metaData(UBSettings::documentDate).toString())
            break;
    }

    destinationFolder->insertChild(i, document);

    // Update editable status of the document if it was moved to or from the trash
    if (documentIsInTrash)
        document->setFlags(document->flags() | Qt::ItemIsEditable);

    if (destinationFolder->isTrashFolder())
        document->setFlags(document->flags() ^ Qt::ItemIsEditable);

    return true;
}

UBDocumentProxyTreeItem::UBDocumentProxyTreeItem(QTreeWidgetItem * parent, UBDocumentProxy* proxy, bool isEditable)
    : QTreeWidgetItem()
    , mProxy(proxy)
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;

    if (isEditable)
        flags |= Qt::ItemIsEditable;

    setFlags(flags);

    int i = 0;
    for (i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem *ti = parent->child(i);
        UBDocumentProxyTreeItem* pi = dynamic_cast<UBDocumentProxyTreeItem*>(ti);
        if (pi)
        {
            if (proxy->metaData(UBSettings::documentDate).toString() >= pi->proxy()->metaData(UBSettings::documentDate).toString())
            {
                break;
            }
        }
    }
    parent->insertChild(i, this);
}

bool UBDocumentProxyTreeItem::isInTrash()
{
    UBDocumentGroupTreeItem * parentFolder = dynamic_cast<UBDocumentGroupTreeItem*>(this->parent());
    if (parentFolder)
        return parentFolder->isTrashFolder();
    else {
        qWarning() << "UBDocumentProxyTreeItem::isInTrash: document has no parent folder. Assuming it is in trash.";
        return true;
    }
}


UBDocumentGroupTreeItem::UBDocumentGroupTreeItem(QTreeWidgetItem *parent, bool isEditable)
    : QTreeWidgetItem(parent)
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    if (isEditable)
        flags |= Qt::ItemIsEditable;
    setFlags(flags);
}


UBDocumentGroupTreeItem::~UBDocumentGroupTreeItem()
{
    // NOOP
}


void UBDocumentGroupTreeItem::setGroupName(const QString& groupName)
{
    setText(0, groupName);
}


QString UBDocumentGroupTreeItem::groupName() const
{
    return text(0);
}

bool UBDocumentGroupTreeItem::isTrashFolder() const
{
    return (0 == (flags() & Qt::ItemIsEditable)) &&  UBApplication::app()->documentController && (groupName() == UBApplication::app()->documentController->documentTrashGroupName());
}

bool UBDocumentGroupTreeItem::isDefaultFolder() const
{
    return (0 == (flags() & Qt::ItemIsEditable)) && UBApplication::app()->documentController && (groupName() == UBApplication::app()->documentController->defaultDocumentGroupName());
}


void UBDocumentTreeWidget::autoScroll()
{
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() + mScrollMagnitude);
}
