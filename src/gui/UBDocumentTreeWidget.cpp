/*
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
//#include "document/UBDocumentContainer.h"

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
        UBDocumentGroupTreeItem *group = dynamic_cast< UBDocumentGroupTreeItem *>(item);

        if (group)
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
    Q_UNUSED(event);

    // Tolik
    //itemSelectionChanged();

    QTreeWidget::focusInEvent(event);
}


void UBDocumentTreeWidget::dropEvent(QDropEvent *event)
{
    if (mDropTargetProxyTi)
    {
        mDropTargetProxyTi->setBackground(0, mBackground);
        mDropTargetProxyTi = 0;
    }

    QTreeWidgetItem* underlyingItem = this->itemAt(event->pos());

    UBDocumentGroupTreeItem *groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(underlyingItem);

    if (groupItem && mSelectedProxyTi && mSelectedProxyTi->proxy())
    {
        UBDocumentGroupTreeItem *sourceGroupItem = dynamic_cast<UBDocumentGroupTreeItem*>(mSelectedProxyTi->parent());
        bool isTrashItem = sourceGroupItem && sourceGroupItem->isTrashFolder();
        if ((isTrashItem && !groupItem->isTrashFolder()) ||
            (!isTrashItem && mSelectedProxyTi->proxy()->groupName() != groupItem->groupName()))
        {
            QString groupName;
            if (groupItem->isTrashFolder())
            {
                QString oldGroupName = mSelectedProxyTi->proxy()->metaData(UBSettings::documentGroupName).toString();
                groupName = UBSettings::trashedDocumentGroupNamePrefix + oldGroupName;
            }
            else
            {
                if (groupItem->groupName() == UBApplication::app()->documentController->defaultDocumentGroupName())
                    groupName = "";
                else
                    groupName = groupItem->groupName();
            }
            mSelectedProxyTi->proxy()->setMetaData(UBSettings::documentGroupName, groupName);
            UBPersistenceManager::persistenceManager()->persistDocumentMetadata(mSelectedProxyTi->proxy());

            mSelectedProxyTi->parent()->removeChild(mSelectedProxyTi);

            int i = 0;
            for (i = 0; i < groupItem->childCount(); i++)
            {
                QTreeWidgetItem *ti = groupItem->child(i);
                UBDocumentProxyTreeItem* pi = dynamic_cast<UBDocumentProxyTreeItem*>(ti);
                if (pi)
                {
                    if (mSelectedProxyTi->proxy()->metaData(UBSettings::documentDate).toString() >= pi->proxy()->metaData(UBSettings::documentDate).toString())
                    {
                        break;
                    }
                }
            }
            groupItem->insertChild(i, mSelectedProxyTi);

            if (isTrashItem)
                mSelectedProxyTi->setFlags(mSelectedProxyTi->flags() | Qt::ItemIsEditable);

            if (groupItem->isTrashFolder())
                mSelectedProxyTi->setFlags(mSelectedProxyTi->flags() ^ Qt::ItemIsEditable);

            //clearSelection();
            expandItem(groupItem);
            scrollToItem(mSelectedProxyTi);

            // disabled, as those 2 calls are buggy on windows, the item disappears if we selected them
            //
            setCurrentItem(mSelectedProxyTi);
            mSelectedProxyTi->setSelected(true);

            event->setDropAction(Qt::IgnoreAction);
            event->accept();
        }
    }
    else
    {
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
//                            UBGraphicsScene* sceneClone = scene;

                            UBDocumentProxy *targetDocProxy = targetProxyTreeItem->proxy();

                            foreach (QUrl relativeFile, scene->relativeDependencies())
                            {
                                QString source = scene->document()->persistencePath() + "/" + relativeFile.toString();
                                QString target = targetDocProxy->persistencePath() + "/" + relativeFile.toString();

                                if(QFileInfo(source).isDir())
                                    Q_ASSERT(UBFileSystemUtils::copyDir(source,target));
                                else{
                                    QFileInfo fi(target);
                                    QDir d = fi.dir();
                                    d.mkpath(d.absolutePath());
                                    Q_ASSERT(QFile::copy(source, target));
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
