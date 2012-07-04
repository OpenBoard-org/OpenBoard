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

#include "UBDocumentController.h"

#include <QtCore>
#include <QtGui>

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBStringUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBDocumentManager.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "adaptors/UBExportPDF.h"
#include "adaptors/UBThumbnailAdaptor.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBDrawingController.h"


#include "gui/UBThumbnailView.h"
#include "gui/UBDocumentTreeWidget.h"
#include "gui/UBMousePressFilter.h"
#include "gui/UBMessageWindow.h"
#include "gui/UBMainWindow.h"
#include "gui/UBDocumentToolsPalette.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"

#include "document/UBDocumentProxy.h"

#include "ui_documents.h"
#include "ui_mainWindow.h"

#include "core/memcheck.h"

UBDocumentController::UBDocumentController(UBMainWindow* mainWindow)
   : QObject(mainWindow->centralWidget())
   , mSelectionType(None)
   , mParentWidget(mainWindow->centralWidget())
   , mBoardController(UBApplication::boardController)
   , mDocumentUI(0)
   , mMainWindow(mainWindow)
   , mDocumentWidget(0)
   , mIsClosing(false)
   , mToolsPalette(0)
   , mToolsPalettePositionned(false)
   , mTrashTi(0)
{
    setupViews();
    setupToolbar();
    this->selectDocument(UBApplication::boardController->activeDocument());
    connect(this, SIGNAL(exportDone()), mMainWindow, SLOT(onExportDone()));
    connect(mMainWindow->actionNewPage, SIGNAL(triggered()), this, SLOT(reloadThumbs()));
}

void UBDocumentController::reloadThumbs()
{
    mDocumentThumbs = UBThumbnailAdaptor::load(selectedDocumentProxy());
}

UBDocumentController::~UBDocumentController()
{
   if (mDocumentUI)
       delete mDocumentUI;
}


void UBDocumentController::createNewDocument()
{
    UBDocumentGroupTreeItem* group = selectedDocumentGroupTreeItem();

    if (group)
    {
        UBDocumentProxy *document = UBPersistenceManager::persistenceManager()->createDocument(group->groupName());

        selectDocument(document);
    }
}


UBDocumentProxyTreeItem* UBDocumentController::findDocument(UBDocumentProxy* proxy)
{
    QTreeWidgetItemIterator it(mDocumentUI->documentTreeWidget);

    while (*it)
    {
        UBDocumentProxyTreeItem *treeItem = dynamic_cast<UBDocumentProxyTreeItem*>((*it));

        if (treeItem && treeItem->proxy() == proxy)
            return treeItem;

        ++it;
    }

    return 0;
}


void UBDocumentController::selectDocument(UBDocumentProxy* proxy, bool setAsCurrentDocument)
{
    if (!proxy)
        return;

    QTreeWidgetItemIterator it(mDocumentUI->documentTreeWidget);

    mDocumentUI->documentTreeWidget->clearSelection();
    mDocumentUI->documentTreeWidget->setCurrentItem(0);

    UBDocumentProxyTreeItem* selected = 0;

    while (*it)
    {
        UBDocumentProxyTreeItem* pi = dynamic_cast<UBDocumentProxyTreeItem*>((*it));

        if (pi)
        {
            if (setAsCurrentDocument)
                pi->setIcon(0, QIcon(""));

            pi->setSelected(false);

            if (pi->proxy() == proxy)
            {
                selected = pi;
            }
        }

        ++it;
    }

    if (selected)
    {
        selected->setSelected(true);

        selected->parent()->setExpanded(true);
        selected->setText(0, proxy->name());

        if (setAsCurrentDocument)
        {
            selected->setIcon(0, QIcon(":/images/currentDocument.png"));
            if (proxy != mBoardController->activeDocument())
                mBoardController->setActiveDocumentScene(proxy);
        }

        mDocumentUI->documentTreeWidget->setCurrentItem(selected);

        mDocumentUI->documentTreeWidget->scrollToItem(selected);

        mDocumentThumbs = UBThumbnailAdaptor::load(selectedDocumentProxy());
        refreshDocumentThumbnailsView();

        mSelectionType = Document;
    }
}


void UBDocumentController::createNewDocumentGroup()
{
    UBDocumentGroupTreeItem* docGroupItem = new UBDocumentGroupTreeItem(0); // deleted by the tree widget
    int i = 1;
    QString newFolderName = tr("New Folder");
    while (allGroupNames().contains(newFolderName))
    {
        newFolderName = tr("New Folder") + " " + QVariant(i++).toString();
    }
    docGroupItem->setGroupName(newFolderName);

    int trashIndex =  mDocumentUI->documentTreeWidget->indexOfTopLevelItem(mTrashTi);

    mDocumentUI->documentTreeWidget->insertTopLevelItem(trashIndex, docGroupItem);
    mDocumentUI->documentTreeWidget->setCurrentItem(docGroupItem);
    mDocumentUI->documentTreeWidget->expandItem(docGroupItem);
}


UBDocumentProxy* UBDocumentController::selectedDocumentProxy()
{
    UBDocumentProxyTreeItem* proxyItem = selectedDocumentProxyTreeItem();
    return proxyItem ? proxyItem->proxy() : 0;
}


UBDocumentProxyTreeItem* UBDocumentController::selectedDocumentProxyTreeItem()
{
    if (mDocumentUI && mDocumentUI->documentTreeWidget)
    {
        QList<QTreeWidgetItem *> selectedItems = mDocumentUI->documentTreeWidget->selectedItems();

        foreach (QTreeWidgetItem * item, selectedItems)
        {
            UBDocumentProxyTreeItem* proxyItem = dynamic_cast<UBDocumentProxyTreeItem*>(item);

            if (proxyItem)
            {
                return proxyItem;
            }
        }
    }

    return 0;
}


UBDocumentGroupTreeItem* UBDocumentController::selectedDocumentGroupTreeItem()
{
    QList<QTreeWidgetItem *> selectedItems = mDocumentUI->documentTreeWidget->selectedItems();

    foreach (QTreeWidgetItem * item, selectedItems)
    {
        UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);

        if (groupItem)
        {
            return groupItem;
        }
        else
        {
            UBDocumentGroupTreeItem* parent = dynamic_cast<UBDocumentGroupTreeItem*>(item->parent());
            if (parent)
            {
                return parent;
            }
        }
    }

    return 0;
}


void UBDocumentController::itemSelectionChanged()
{
    refreshDocumentThumbnailsView();

    if (selectedDocumentProxy())
        mSelectionType = Document;
    else if (selectedDocumentGroupTreeItem())
        mSelectionType = Folder;
    else
        mSelectionType = None;

    selectionChanged();
}


void UBDocumentController::refreshDocumentThumbnailsView()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> items;
    QList<QUrl> itemsPath;

    UBDocumentProxy *proxy = selectedDocumentProxy();
    QGraphicsPixmapItem *selection = 0;

    QStringList labels;

    if (proxy)
    {
        mCurrentDocument = proxy;

        for (int i = 0; i < mDocumentThumbs.count(); i++)
        {
            QPixmap pix = mDocumentThumbs.at(i);
            QGraphicsPixmapItem *pixmapItem = new UBSceneThumbnailPixmap(pix, proxy, i); // deleted by the tree widget

            if (proxy == mBoardController->activeDocument() && mBoardController->activeSceneIndex() == i)
            {
                selection = pixmapItem;
            }

            items << pixmapItem;
            labels << tr("Page %1").arg(UBApplication::boardController->pageFromSceneIndex(i));

            itemsPath.append(QUrl::fromLocalFile(proxy->persistencePath() + QString("/pages/%1").arg(UBApplication::boardController->pageFromSceneIndex(i))));
        }
    }

    mDocumentUI->thumbnailWidget->setGraphicsItems(items, itemsPath, labels, UBApplication::mimeTypeUniboardPage);

    UBDocumentProxyTreeItem* proxyTi = selectedDocumentProxyTreeItem();
    if (proxyTi && (proxyTi->parent() == mTrashTi))
        mDocumentUI->thumbnailWidget->setDragEnabled(false);
    else
        mDocumentUI->thumbnailWidget->setDragEnabled(true);

    mDocumentUI->thumbnailWidget->ensureVisible(0, 0, 10, 10);

    if (selection) {
        disconnect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()), this, SLOT(pageSelectionChanged()));
        UBSceneThumbnailPixmap *currentScene = dynamic_cast<UBSceneThumbnailPixmap*>(selection);
        if (currentScene)
            mDocumentUI->thumbnailWidget->hightlightItem(currentScene->sceneIndex());
        connect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()), this, SLOT(pageSelectionChanged()));
    }

    emit refreshThumbnails();
    QApplication::restoreOverrideCursor();
}


void UBDocumentController::setupViews()
{

    if (!mDocumentWidget)
    {
        mDocumentWidget = new QWidget(mMainWindow->centralWidget());
        mMainWindow->addDocumentsWidget(mDocumentWidget);

        mDocumentUI = new Ui::documents();

        mDocumentUI->setupUi(mDocumentWidget);

        int thumbWidth = UBSettings::settings()->documentThumbnailWidth->get().toInt();

        mDocumentUI->documentZoomSlider->setValue(thumbWidth);
        mDocumentUI->thumbnailWidget->setThumbnailWidth(thumbWidth);

        connect(mDocumentUI->documentZoomSlider, SIGNAL(valueChanged(int)), this,
                SLOT(documentZoomSliderValueChanged(int)));

        connect(mMainWindow->actionOpen, SIGNAL(triggered()), this, SLOT(openSelectedItem()));
        connect(mMainWindow->actionNewFolder, SIGNAL(triggered()), this, SLOT(createNewDocumentGroup()));
        connect(mMainWindow->actionNewDocument, SIGNAL(triggered()), this, SLOT(createNewDocument()));

        connect(mMainWindow->actionImport, SIGNAL(triggered(bool)), this, SLOT(importFile()));

        QMenu* addMenu = new QMenu(mDocumentWidget);
        mAddFolderOfImagesAction = addMenu->addAction(tr("Add Folder of Images"));
        mAddImagesAction = addMenu->addAction(tr("Add Images"));
        mAddFileToDocumentAction = addMenu->addAction(tr("Add Pages from File"));

        connect(mAddFolderOfImagesAction, SIGNAL(triggered(bool)), this, SLOT(addFolderOfImages()));
        connect(mAddFileToDocumentAction, SIGNAL(triggered(bool)), this, SLOT(addFileToDocument()));
        connect(mAddImagesAction, SIGNAL(triggered(bool)), this, SLOT(addImages()));

        foreach (QWidget* menuWidget,  mMainWindow->actionDocumentAdd->associatedWidgets())
        {
            QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

            if (tb && !tb->menu())
            {
                tb->setObjectName("ubButtonMenu");
                tb->setPopupMode(QToolButton::InstantPopup);

                QMenu* menu = new QMenu(mDocumentWidget);

                menu->addAction(mAddFolderOfImagesAction);
                menu->addAction(mAddImagesAction);
                menu->addAction(mAddFileToDocumentAction);

                tb->setMenu(menu);
            }
        }

        QMenu* exportMenu = new QMenu(mDocumentWidget);

        UBDocumentManager *documentManager = UBDocumentManager::documentManager();
        for (int i = 0; i < documentManager->supportedExportAdaptors().length(); i++)
        {
            UBExportAdaptor* adaptor = documentManager->supportedExportAdaptors()[i];
            QAction *currentExportAction = exportMenu->addAction(adaptor->exportName());
            currentExportAction->setData(i);
            connect(currentExportAction, SIGNAL(triggered (bool)), this, SLOT(exportDocument()));
            exportMenu->addAction(currentExportAction);
        }

        foreach (QWidget* menuWidget,  mMainWindow->actionExport->associatedWidgets())
        {
            QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

            if (tb && !tb->menu())
            {
                tb->setObjectName("ubButtonMenu");
                tb->setPopupMode(QToolButton::InstantPopup);

                tb->setMenu(exportMenu);
            }
        }

#ifdef Q_WS_MAC
        mMainWindow->actionDelete->setShortcut(QKeySequence(Qt::Key_Backspace));
#endif

        connect(mMainWindow->actionDelete, SIGNAL(triggered()), this, SLOT(deleteSelectedItem()));
        connect(mMainWindow->actionDuplicate, SIGNAL(triggered()), this, SLOT(duplicateSelectedItem()));
        connect(mMainWindow->actionRename, SIGNAL(triggered()), this, SLOT(renameSelectedItem()));
        connect(mMainWindow->actionAddToWorkingDocument, SIGNAL(triggered()), this, SLOT(addToDocument()));

        loadDocumentProxies();

        mDocumentUI->documentTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        mDocumentUI->documentTreeWidget->setDragEnabled(true);
        mDocumentUI->documentTreeWidget->viewport()->setAcceptDrops(true);
        mDocumentUI->documentTreeWidget->setDropIndicatorShown(true);
        mDocumentUI->documentTreeWidget->setIndentation(18); // 1.5 * /resources/style/treeview-branch-closed.png width
        mDocumentUI->documentTreeWidget->setDragDropMode(QAbstractItemView::InternalMove);

        connect(mDocumentUI->documentTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
        connect(mDocumentUI->documentTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(itemChanged(QTreeWidgetItem *, int)));
        connect(mDocumentUI->documentTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(itemClicked(QTreeWidgetItem *, int)));

        connect(mDocumentUI->thumbnailWidget, SIGNAL(sceneDropped(UBDocumentProxy*, int, int)), this, SLOT(moveSceneToIndex ( UBDocumentProxy*, int, int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(resized()), this, SLOT(thumbnailViewResized()));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseDoubleClick(QGraphicsItem*, int)),
                this, SLOT(pageDoubleClicked(QGraphicsItem*, int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseClick(QGraphicsItem*, int)),
                this, SLOT(pageClicked(QGraphicsItem*, int)));

        connect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()),
                this, SLOT(pageSelectionChanged()));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentCreated(UBDocumentProxy*)),
                        this, SLOT(addDocumentInTree(UBDocumentProxy*)));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentMetadataChanged(UBDocumentProxy*)),
                        this, SLOT(updateDocumentInTree(UBDocumentProxy*)));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneCreated(UBDocumentProxy*, int)),
                this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneWillBeDeleted(UBDocumentProxy*, int)),
                this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));

        mDocumentUI->thumbnailWidget->setBackgroundBrush(UBSettings::documentViewLightColor);

        mMessageWindow = new UBMessageWindow(mDocumentUI->thumbnailWidget);
        mMessageWindow->hide();

    }
}


QWidget* UBDocumentController::controlView()
{
    return mDocumentWidget;
}


void UBDocumentController::setupToolbar()
{
    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->documentToolBar, mMainWindow->actionBoard);
    connect(mMainWindow->actionDocumentTools, SIGNAL(triggered()), this, SLOT(toggleDocumentToolsPalette()));
}

void UBDocumentController::setupPalettes()
{

    mToolsPalette = new UBDocumentToolsPalette(controlView());

    mToolsPalette->hide();

    bool showToolsPalette = !mToolsPalette->isEmpty();
    mMainWindow->actionDocumentTools->setVisible(showToolsPalette);

    if (showToolsPalette)
    {
        mMainWindow->actionDocumentTools->trigger();
    }
}


void UBDocumentController::show()
{
    selectDocument(mBoardController->activeDocument());

    selectionChanged();

    if(!mToolsPalette)
        setupPalettes();
}


void UBDocumentController::hide()
{
    // NOOP
}


void UBDocumentController::openSelectedItem()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();

    if (selectedItems.count() > 0)
    {
        UBSceneThumbnailPixmap* thumb = dynamic_cast<UBSceneThumbnailPixmap*> (selectedItems.last());

        if (thumb)
        {
            UBDocumentProxy* proxy = thumb->proxy();

            if (proxy && isOKToOpenDocument(proxy))
            {
                UBApplication::applicationController->showBoard();
            }
        }
    }
    else
    {
        UBDocumentProxy* proxy = selectedDocumentProxy();

        if (proxy && isOKToOpenDocument(proxy))
        {
            mBoardController->setActiveDocumentScene(proxy);
            UBApplication::applicationController->showBoard();
        }
    }

    QApplication::restoreOverrideCursor();
}

void UBDocumentController::duplicateSelectedItem()
{
    if (UBApplication::applicationController->displayMode() != UBApplicationController::Document)
        return;

	if (mSelectionType == Page)
    {
        QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();
        QList<int> selectedSceneIndexes;
        foreach (QGraphicsItem *item, selectedItems)
        {
            UBSceneThumbnailPixmap *thumb = dynamic_cast<UBSceneThumbnailPixmap*>(item);
            if (thumb)
            {
                UBDocumentProxy *proxy = thumb->proxy();

                if (proxy)
                {
                    int sceneIndex = thumb->sceneIndex();
                    selectedSceneIndexes << sceneIndex;
                }
            }
        }
        if (selectedSceneIndexes.count() > 0)
        {
            UBSceneThumbnailPixmap *thumb = dynamic_cast<UBSceneThumbnailPixmap*>(selectedItems.at(0));
            UBDocumentProxy *proxy = thumb->proxy();
            int offset = 0;
            if (proxy)
            {
                foreach (int sceneIndex, selectedSceneIndexes)
                {
                    UBPersistenceManager::persistenceManager()->duplicateDocumentScene(proxy, sceneIndex + offset);
                    mDocumentThumbs.insert(sceneIndex + offset, mDocumentThumbs.at(sceneIndex + offset));
                    offset++;
                }
            }
            refreshDocumentThumbnailsView();
            proxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(proxy);
            mDocumentUI->thumbnailWidget->selectItemAt(selectedSceneIndexes.last() + offset);
        }
    }
    else
    {
        UBDocumentProxy* source = selectedDocumentProxy();
        UBDocumentGroupTreeItem* group = selectedDocumentGroupTreeItem();

        if (source && group)
        {
                QString docName = source->metaData(UBSettings::documentName).toString();

                showMessage(tr("Duplicating Document %1").arg(docName), true);

            UBDocumentProxy* duplicatedDoc = UBPersistenceManager::persistenceManager()->duplicateDocument(source);
            duplicatedDoc->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(duplicatedDoc);

            selectDocument(duplicatedDoc, false);

            showMessage(tr("Document %1 copied").arg(docName), false);
        }
    }
}


void UBDocumentController::deleteSelectedItem()
{
    if (mSelectionType == Page)
    {
        QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();

        deletePages(selectedItems);
    }
    else
    {

        UBDocumentProxyTreeItem *proxyTi = selectedDocumentProxyTreeItem();

        UBDocumentGroupTreeItem* groupTi = selectedDocumentGroupTreeItem();

        if (proxyTi && proxyTi->proxy() && proxyTi->parent())
        {
            if(UBApplication::mainWindow->yesNoQuestion(tr("Remove Document"), tr("Are you sure you want to remove the document '%1'?").arg(proxyTi->proxy()->metaData(UBSettings::documentName).toString())))
            {
                if (proxyTi->parent() == mTrashTi)
                {
                    int index = proxyTi->parent()->indexOfChild(proxyTi);
                    index --;

                    if (index >= 0)
                    {
                        if (proxyTi->proxy() == mBoardController->activeDocument())
                        {
                            selectDocument(((UBDocumentProxyTreeItem*)proxyTi->parent()->child(index))->proxy());
                        }
                        else
                            proxyTi->parent()->child(index)->setSelected(true);
                    }
                    else if (proxyTi->parent()->childCount() > 1)
                    {
                        if (proxyTi->proxy() == mBoardController->activeDocument())
                        {
                            selectDocument(((UBDocumentProxyTreeItem*)proxyTi->parent()->child(1))->proxy());
                        }
                        else
                            proxyTi->parent()->child(1)->setSelected(true);
                    }
                    else
                    {
                        if (proxyTi->proxy() == mBoardController->activeDocument())
                        {
                            for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
                            {
                                QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);
                                UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
                                if (groupItem != selectedDocumentGroupTreeItem() && groupItem->childCount() > 0)
                                {
                                    selectDocument(((UBDocumentProxyTreeItem*)groupItem->child(0))->proxy());
                                    break;
                                }
                            }
                        }
                        else
                            proxyTi->parent()->setSelected(true);
                    }

                    proxyTi->parent()->removeChild(proxyTi);

                    UBPersistenceManager::persistenceManager()->deleteDocument(proxyTi->proxy());

                    refreshDocumentThumbnailsView();
                }
                else
                {
                    // Move document to trash
                    QString oldGroupName = proxyTi->proxy()->metaData(UBSettings::documentGroupName).toString();
                    proxyTi->proxy()->setMetaData(UBSettings::documentGroupName, UBSettings::trashedDocumentGroupNamePrefix + oldGroupName);
                    UBPersistenceManager::persistenceManager()->persistDocumentMetadata(proxyTi->proxy());

                    proxyTi->parent()->removeChild(proxyTi);
                    mTrashTi->addChild(proxyTi);
                    proxyTi->setFlags(proxyTi->flags() ^ Qt::ItemIsEditable);
                }
            }
        }
        else if (groupTi)
        {
            if (groupTi == mTrashTi)
            {
                if(UBApplication::mainWindow->yesNoQuestion(tr("Empty Trash"), tr("Are you sure you want to empty trash?")))
                {
                    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                    QList<UBDocumentProxyTreeItem*> toBeDeleted;

                    for (int i = 0; i < groupTi->childCount(); i++)
                    {
                        UBDocumentProxyTreeItem* proxyTi = dynamic_cast<UBDocumentProxyTreeItem*>(groupTi->child(i));
                        if (proxyTi && proxyTi->proxy())
                            toBeDeleted << proxyTi;
                    }

                    UBApplication::showMessage(tr("Emptying trash"));

                    for (int i = 0; i < toBeDeleted.count(); i++)
                    {
                        UBDocumentProxyTreeItem* proxyTi = toBeDeleted.at(i);

                        proxyTi->parent()->removeChild(proxyTi);
                        UBPersistenceManager::persistenceManager()->deleteDocument(proxyTi->proxy());
                    }

                    UBApplication::showMessage(tr("Emptied trash"));

                    QApplication::restoreOverrideCursor();
                    mMainWindow->actionDelete->setEnabled(false);
                }
            }
            else
            {
                if(UBApplication::mainWindow->yesNoQuestion(tr("Remove Folder"), tr("Are you sure you want to remove the folder '%1' and all its content?").arg(groupTi->groupName())))
                {
                    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

                    bool changeCurrentDocument = false;
                    for (int i = 0; i < groupTi->childCount(); i++)
                    {
                        UBDocumentProxyTreeItem* proxyTi  = dynamic_cast<UBDocumentProxyTreeItem*>(groupTi->child(i));
                        if (proxyTi && proxyTi->proxy() && proxyTi->proxy() == mBoardController->activeDocument())
                        {
                            changeCurrentDocument = true;
                            break;
                        }
                    }

                    if (changeCurrentDocument)
                    {
                        for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
                        {
                            QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);
                            UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
                            if (groupItem != groupTi && groupItem->childCount() > 0)
                            {
                                selectDocument(((UBDocumentProxyTreeItem*)groupItem->child(0))->proxy());
                                break;
                            }
                        }
                    }

                    QList<UBDocumentProxyTreeItem*> toBeDeleted;

                    for (int i = 0; i < groupTi->childCount(); i++)
                    {
                        UBDocumentProxyTreeItem* proxyTi = dynamic_cast<UBDocumentProxyTreeItem*>(groupTi->child(i));
                        if (proxyTi && proxyTi->proxy())
                            toBeDeleted << proxyTi;
                    }

                    for (int i = 0; i < toBeDeleted.count(); i++)
                    {
                        UBDocumentProxyTreeItem* proxyTi = toBeDeleted.at(i);

                        UBApplication::showMessage(QString("Deleting %1").arg(proxyTi->proxy()->metaData(UBSettings::documentName).toString()));
                        // Move document to trash
                        QString oldGroupName = proxyTi->proxy()->metaData(UBSettings::documentGroupName).toString();
                        proxyTi->proxy()->setMetaData(UBSettings::documentGroupName, UBSettings::trashedDocumentGroupNamePrefix + oldGroupName);
                        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(proxyTi->proxy());

                        groupTi->removeChild(proxyTi);
                        mTrashTi->addChild(proxyTi);
                        proxyTi->setFlags(proxyTi->flags() ^ Qt::ItemIsEditable);

                        UBApplication::showMessage(QString("%1 deleted").arg(groupTi->groupName()));
                    }

                    // dont remove default group
                    if (!groupTi->isDefaultFolder())
                    {
                        int index = mDocumentUI->documentTreeWidget->indexOfTopLevelItem(groupTi);

                        if (index >= 0)
                        {
                            mDocumentUI->documentTreeWidget->takeTopLevelItem(index);
                        }
                    }

                    refreshDocumentThumbnailsView();

                    QApplication::restoreOverrideCursor();
                }
            }
        }
    }
}


void UBDocumentController::exportDocument()
{
    QAction *currentExportAction = qobject_cast<QAction *>(sender());
    QVariant actionData = currentExportAction->data();
    UBExportAdaptor* selectedExportAdaptor = UBDocumentManager::documentManager()->supportedExportAdaptors()[actionData.toInt()];

    UBDocumentProxy* proxy = selectedDocumentProxy();

    if (proxy)
    {
        selectedExportAdaptor->persist(proxy);
        emit exportDone();
    }
    else
    {
       UBApplication::showMessage(tr("No document selected!"));
    }
}


void UBDocumentController::documentZoomSliderValueChanged (int value)
{
    mDocumentUI->thumbnailWidget->setThumbnailWidth(value);

    UBSettings::settings()->documentThumbnailWidth->set(value);
}


UBGraphicsScene* UBDocumentController::activeScene()
{
    if (mBoardController->activeDocument())
    {
        return mBoardController->activeScene();
    }
    else
    {
        return 0;
    }
}


void UBDocumentController::loadDocumentProxies()
{
    QList<QPointer<UBDocumentProxy> > proxies = UBPersistenceManager::persistenceManager()->documentProxies;

    QStringList emptyGroupNames = UBSettings::settings()->value("Document/EmptyGroupNames", QStringList()).toStringList();

    mDocumentUI->documentTreeWidget->clear();

    QMap<QString, UBDocumentGroupTreeItem*> groupNamesMap;

    UBDocumentGroupTreeItem* emptyGroupNameTi = 0;

    mTrashTi = new UBDocumentGroupTreeItem(0, false); // deleted by the tree widget
    mTrashTi->setGroupName(UBSettings::documentTrashGroupName);
    mTrashTi->setIcon(0, QIcon(":/images/trash.png"));

    foreach (QPointer<UBDocumentProxy> proxy, proxies)
    {
        if (proxy)
        {
            QString docGroup = proxy->metaData(UBSettings::documentGroupName).toString();

            bool isEmptyGroupName = false;
            bool isInTrash = false;

            if (docGroup.isEmpty()) // #see https://trac.assembla.com/uniboard/ticket/426
            {
                docGroup = UBSettings::defaultDocumentGroupName;
                isEmptyGroupName = true;
            }
            else if (docGroup.startsWith(UBSettings::trashedDocumentGroupNamePrefix))
            {
                isInTrash = true;
            }

            QString docName = proxy->metaData(UBSettings::documentName).toString();

            if (emptyGroupNames.contains(docGroup))
                emptyGroupNames.removeAll(docGroup);

            if (!groupNamesMap.contains(docGroup) && !isInTrash)
            {
                UBDocumentGroupTreeItem* docGroupItem = new UBDocumentGroupTreeItem(0, !isEmptyGroupName); // deleted by the tree widget
                groupNamesMap.insert(docGroup, docGroupItem);
                docGroupItem->setGroupName(docGroup);

                if (isEmptyGroupName)
                    emptyGroupNameTi = docGroupItem;
            }

            UBDocumentGroupTreeItem* docGroupItem;
            if (isInTrash)
                docGroupItem = mTrashTi;
            else
                docGroupItem = groupNamesMap.value(docGroup);

            QTreeWidgetItem* docItem = new UBDocumentProxyTreeItem(docGroupItem, proxy, !isInTrash);
            docItem->setText(0, docName);

            if (mBoardController->activeDocument() == proxy)
            {
                mDocumentUI->documentTreeWidget->expandItem(docGroupItem);
                mDocumentUI->documentTreeWidget->setCurrentItem(docGroupItem);
            }
        }
    }

    foreach (const QString emptyGroupName, emptyGroupNames)
    {
        UBDocumentGroupTreeItem* docGroupItem = new UBDocumentGroupTreeItem(0); // deleted by the tree widget
        groupNamesMap.insert(emptyGroupName, docGroupItem);
        docGroupItem->setGroupName(emptyGroupName);
    }

    QList<QString> groupNamesList = groupNamesMap.keys();
    qSort(groupNamesList);

    foreach (const QString groupName, groupNamesList)
    {
        UBDocumentGroupTreeItem* ti = groupNamesMap.value(groupName);

        if (ti != emptyGroupNameTi)
            mDocumentUI->documentTreeWidget->addTopLevelItem(ti);
    }

    if (emptyGroupNameTi)
        mDocumentUI->documentTreeWidget->addTopLevelItem(emptyGroupNameTi);

    mDocumentUI->documentTreeWidget->addTopLevelItem(mTrashTi);
}


void UBDocumentController::itemClicked(QTreeWidgetItem * item, int column )
{
    Q_UNUSED(item);
    Q_UNUSED(column);

    selectDocument(selectedDocumentProxy(), false);
    itemSelectionChanged();
}


void UBDocumentController::itemChanged(QTreeWidgetItem * item, int column)
{
    UBDocumentProxyTreeItem* proxyItem = dynamic_cast<UBDocumentProxyTreeItem*>(item);

    disconnect(UBPersistenceManager::persistenceManager(), SIGNAL(documentMetadataChanged(UBDocumentProxy*))
            , this, SLOT(updateDocumentInTree(UBDocumentProxy*)));

    if (proxyItem)
    {
        if (proxyItem->proxy()->metaData(UBSettings::documentName).toString() != item->text(column))
        {
            proxyItem->proxy()->setMetaData(UBSettings::documentName, item->text(column));
            UBPersistenceManager::persistenceManager()->persistDocumentMetadata(proxyItem->proxy());
        }
    }
    else
    {
        // it is a group
        UBDocumentGroupTreeItem* editedGroup = dynamic_cast<UBDocumentGroupTreeItem*>(item);
        if (editedGroup)
        {
            for (int i = 0; i < item->childCount(); i++)
            {
                UBDocumentProxyTreeItem* childItem = dynamic_cast<UBDocumentProxyTreeItem*>(item->child(i));

                if (childItem)
                {
                    QString groupName;
                    if (0 != (item->flags() & Qt::ItemIsEditable))
                    {
                        childItem->proxy()->setMetaData(UBSettings::documentGroupName, item->text(column));
                        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(childItem->proxy());
                    }
                }
            }
        }
    }

    connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentMetadataChanged(UBDocumentProxy*)),
            this, SLOT(updateDocumentInTree(UBDocumentProxy*)));
}


void UBDocumentController::importFile()
{
    UBDocumentGroupTreeItem* group = selectedDocumentGroupTreeItem();
    UBDocumentManager *docManager = UBDocumentManager::documentManager();

    if (group)
    {
        QString defaultPath = UBSettings::settings()->lastImportFilePath->get().toString();
        QString filePath = QFileDialog::getOpenFileName(mParentWidget, tr("Open Supported File"),
                defaultPath, docManager->importFileFilter());

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QApplication::processEvents();
        QFileInfo fileInfo(filePath);
        UBSettings::settings()->lastImportFilePath->set(QVariant(fileInfo.absolutePath()));

        if (filePath.length() > 0)
        {
            UBDocumentProxy* createdDocument = 0;
            QApplication::processEvents();
            QFile selectedFile(filePath);

            QString groupName = group->groupName();

            if (groupName == UBSettings::defaultDocumentGroupName)
                groupName = "";

            UBApplication::showMessage(tr("Importing file %1...").arg(fileInfo.baseName()), true);

            createdDocument = docManager->importFile(selectedFile, groupName);

            if (createdDocument)
            {
                selectDocument(createdDocument, false);
            }
            else
            {
                UBApplication::showMessage(tr("Failed to import file ... "));
            }
        }

        QApplication::restoreOverrideCursor();
    }
}

void UBDocumentController::addFolderOfImages()
{
    UBDocumentProxy* document = selectedDocumentProxy();

    if (document)
    {
        QString defaultPath = UBSettings::settings()->lastImportFolderPath->get().toString();

        QString imagesDir = QFileDialog::getExistingDirectory(mParentWidget, tr("Import all Images from Folder"), defaultPath);
        QDir parentImageDir(imagesDir);
        parentImageDir.cdUp();

        UBSettings::settings()->lastImportFolderPath->set(QVariant(parentImageDir.absolutePath()));

        if (imagesDir.length() > 0)
        {
            QDir dir(imagesDir);

            int importedImageNumber
                  = UBDocumentManager::documentManager()->addImageDirToDocument(dir, document);

            if (importedImageNumber == 0)
            {
                UBApplication::showMessage(tr("Folder does not contain any image files!"));
                UBApplication::applicationController->showDocument();
            }
            else
            {
                document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
                UBMetadataDcSubsetAdaptor::persist(document);
                mDocumentThumbs = UBThumbnailAdaptor::load(selectedDocumentProxy());
                refreshDocumentThumbnailsView();
            }
        }
    }
}


void UBDocumentController::addFileToDocument()
{
    UBDocumentProxy* document = selectedDocumentProxy();

    if (document)
    {
         addFileToDocument(document);
         mDocumentThumbs = UBThumbnailAdaptor::load(document);
         refreshDocumentThumbnailsView();
    }
}


bool UBDocumentController::addFileToDocument(UBDocumentProxy* document)
{
    QString defaultPath = UBSettings::settings()->lastImportFilePath->get().toString();
    QString filePath = QFileDialog::getOpenFileName(mParentWidget, tr("Open Supported File")
            , defaultPath, UBDocumentManager::documentManager()->importFileFilter());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();

    QFileInfo fileInfo(filePath);
    UBSettings::settings()->lastImportFilePath->set(QVariant(fileInfo.absolutePath()));

    bool success = false;

    if (filePath.length() > 0)
    {
        QApplication::processEvents();
        QFile selectedFile(filePath);

        UBApplication::showMessage(tr("Importing file %1...").arg(fileInfo.baseName()), true);

        success = UBDocumentManager::documentManager()->addFileToDocument(document, selectedFile);

        if (success)
        {
            document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(document);
        }
        else
        {
            UBApplication::showMessage(tr("Failed to import file ... "));
        }
    }

    QApplication::restoreOverrideCursor();

    return success;
}


void UBDocumentController::moveSceneToIndex(UBDocumentProxy* proxy, int source, int target)
{
    UBPersistenceManager::persistenceManager()->moveSceneToIndex(proxy, source, target);

    proxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
    UBMetadataDcSubsetAdaptor::persist(proxy);
    mDocumentThumbs.insert(target, mDocumentThumbs.takeAt(source));
    refreshDocumentThumbnailsView();
    mDocumentUI->thumbnailWidget->hightlightItem(target);
}


void UBDocumentController::thumbnailViewResized()
{
    int maxWidth = qMin(UBSettings::maxThumbnailWidth, mDocumentUI->thumbnailWidget->width());

    mDocumentUI->documentZoomSlider->setMaximum(maxWidth);
}


void UBDocumentController::pageSelectionChanged()
{
    if (mIsClosing)
        return;

    bool pageSelected = mDocumentUI->thumbnailWidget->selectedItems().count() > 0;

    if (pageSelected)
        mSelectionType = Page;
    else
        mSelectionType = None;

    selectionChanged();
}


void UBDocumentController::selectionChanged()
{
    if (mIsClosing)
        return;

    int pageCount = -1;

    UBDocumentProxyTreeItem* proxyTi = selectedDocumentProxyTreeItem();

    if (proxyTi && proxyTi->proxy())
        pageCount = proxyTi->proxy()->pageCount();

    bool pageSelected = (mSelectionType == Page);
    bool groupSelected = (mSelectionType == Folder);
    bool docSelected = (mSelectionType == Document);

    bool trashSelected = false;
    if (groupSelected && selectedDocumentGroupTreeItem())
        trashSelected = selectedDocumentGroupTreeItem()->isTrashFolder();

    if ((docSelected || pageSelected) && proxyTi)
        trashSelected = dynamic_cast<UBDocumentGroupTreeItem*>(proxyTi->parent())->isTrashFolder();

    bool defaultGroupSelected = false;
    if (groupSelected && selectedDocumentGroupTreeItem())
        defaultGroupSelected = selectedDocumentGroupTreeItem()->isDefaultFolder();

    mMainWindow->actionNewDocument->setEnabled((groupSelected || docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionExport->setEnabled((docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionDuplicate->setEnabled((docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionOpen->setEnabled((docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionRename->setEnabled((groupSelected || docSelected) && !trashSelected && !defaultGroupSelected);

    mMainWindow->actionAddToWorkingDocument->setEnabled(pageSelected
            && !(selectedDocumentProxy() == mBoardController->activeDocument()) && !trashSelected);

    bool deleteEnabled = false;
    if (trashSelected)
    {
        if (docSelected)
            deleteEnabled = true;
        else if (groupSelected && selectedDocumentGroupTreeItem())
        {
            if (selectedDocumentGroupTreeItem()->childCount() > 0)
                deleteEnabled = true;
        }
    }
    else
    {
        deleteEnabled = groupSelected || docSelected || pageSelected;
    }

    if (pageSelected && (pageCount == mDocumentUI->thumbnailWidget->selectedItems().count()))
    {
        deleteEnabled = false;
    }

    mMainWindow->actionDelete->setEnabled(deleteEnabled);

    if (trashSelected)
    {
        if (docSelected)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/toolbar/deleteDocument.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        else
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash.png"));
            mMainWindow->actionDelete->setText(tr("Empty"));
        }
    }
    else
    {
        mMainWindow->actionDelete->setIcon(QIcon(":/images/trash.png"));
        mMainWindow->actionDelete->setText(tr("Trash"));
    }

    mMainWindow->actionDocumentAdd->setEnabled((docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionImport->setEnabled(!trashSelected);

}


void UBDocumentController::documentSceneChanged(UBDocumentProxy* proxy, int pSceneIndex)
{
    Q_UNUSED(pSceneIndex);

    if (proxy == selectedDocumentProxy())
    {
        refreshDocumentThumbnailsView();
    }
}


void UBDocumentController::pageDoubleClicked(QGraphicsItem* item, int index)
{
    Q_UNUSED(item);
    Q_UNUSED(index);

    bool pageSelected = (mSelectionType == Page);
    bool groupSelected = (mSelectionType == Folder);
    bool docSelected = (mSelectionType == Document);

    bool trashSelected = false;
    if (groupSelected && selectedDocumentGroupTreeItem())
        trashSelected = selectedDocumentGroupTreeItem()->isTrashFolder();
    UBDocumentProxyTreeItem* proxyTi = selectedDocumentProxyTreeItem();
    if ((docSelected || pageSelected) && proxyTi)
        trashSelected = dynamic_cast<UBDocumentGroupTreeItem*>(proxyTi->parent())->isTrashFolder();
    if (trashSelected) return;

    openSelectedItem();
}


void UBDocumentController::pageClicked(QGraphicsItem* item, int index)
{
    Q_UNUSED(item);
    Q_UNUSED(index);

    pageSelectionChanged();
}


void UBDocumentController::closing()
{
    mIsClosing = true;

    QStringList emptyGroups;

    for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);

        if (item->childCount() == 0)
        {
            UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
            if (groupItem)
            {
                QString groupName = groupItem->groupName();
                if (!emptyGroups.contains(groupName) && groupName != UBSettings::documentTrashGroupName)
                    emptyGroups << groupName;
            }
        }
    }

    UBSettings::settings()->setValue("Document/EmptyGroupNames", emptyGroups);

}

void UBDocumentController::addToDocument()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();

    if (selectedItems.count() > 0)
    {
        int oldActiveSceneIndex = mBoardController->activeSceneIndex();

        QList<QPair<UBDocumentProxy*, int> > pageInfoList;

        foreach (QGraphicsItem* item, selectedItems)
        {
            UBSceneThumbnailPixmap* thumb = dynamic_cast<UBSceneThumbnailPixmap*> (item);

            if (thumb &&  thumb->proxy())
            {
                QPair<UBDocumentProxy*, int> pageInfo(thumb->proxy(), thumb->sceneIndex());
                pageInfoList << pageInfo;
            }
        }

        for (int i = 0; i < pageInfoList.length(); i++)
        {
            mBoardController->addScene(pageInfoList.at(i).first, pageInfoList.at(i).second, true);
        }

        int newActiveSceneIndex = selectedItems.count() == mBoardController->activeDocument()->pageCount() ? 0 : oldActiveSceneIndex + 1;
        mDocumentUI->thumbnailWidget->selectItemAt(newActiveSceneIndex, false);
        selectDocument(mBoardController->activeDocument());
        mBoardController->activeDocument()->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBMetadataDcSubsetAdaptor::persist(mBoardController->activeDocument());

        UBApplication::applicationController->showBoard();
    }

    QApplication::restoreOverrideCursor();
}


void UBDocumentController::addDocumentInTree(UBDocumentProxy* pDocument)
{
    QString documentName = pDocument->name();
    QString documentGroup = pDocument->groupName();
    if (documentGroup.isEmpty())
    {
        documentGroup = UBSettings::defaultDocumentGroupName;
    }
    UBDocumentGroupTreeItem* group = 0;
    if (documentGroup.startsWith(UBSettings::trashedDocumentGroupNamePrefix))
    {
        group = mTrashTi;
    }
    else
    {
        for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);
            UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
            if (groupItem->groupName() == documentGroup)
            {
                group = groupItem;
                break;
            }
        }
    }

    if (group == 0)
    {
        group = new UBDocumentGroupTreeItem(0); // deleted by the tree widget
        group->setGroupName(documentGroup);
        mDocumentUI->documentTreeWidget->addTopLevelItem(group);
    }

    UBDocumentProxyTreeItem *ti = new UBDocumentProxyTreeItem(group, pDocument, !group->isTrashFolder());
    ti->setText(0, documentName);
}


void UBDocumentController::updateDocumentInTree(UBDocumentProxy* pDocument)
{
    QTreeWidgetItemIterator it(mDocumentUI->documentTreeWidget);
    while (*it)
    {
        UBDocumentProxyTreeItem* pi = dynamic_cast<UBDocumentProxyTreeItem*>((*it));

        if (pi && pi->proxy() == pDocument)
        {
            pi->setText(0, pDocument->name());
            break;
        }
        ++it;
    }
}


QStringList UBDocumentController::allGroupNames()
{
    QStringList result;

    for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);
        UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
        result << groupItem->groupName();
    }

    return result;
}


void UBDocumentController::renameSelectedItem()
{
    if (mDocumentUI->documentTreeWidget->selectedItems().count() > 0)
        mDocumentUI->documentTreeWidget->editItem(mDocumentUI->documentTreeWidget->selectedItems().at(0));
}


bool UBDocumentController::isOKToOpenDocument(UBDocumentProxy* proxy)
{
    //check version
    QString docVersion = proxy->metaData(UBSettings::documentVersion).toString();

    if (docVersion.isEmpty() || docVersion.startsWith("4.1") || docVersion.startsWith("4.2")
            || docVersion.startsWith("4.3") || docVersion.startsWith("4.4") || docVersion.startsWith("4.5")
            || docVersion.startsWith("4.6")) // TODO UB 4.7 update if necessary
    {
        return true;
    }
    else
    {
        if (UBApplication::mainWindow->yesNoQuestion(tr("Open Document"),
                tr("The document '%1' has been generated with a newer version of Sankore (%2). By opening it, you may lose some information. Do you want to proceed?")
                    .arg(proxy->metaData(UBSettings::documentName).toString())
                    .arg(docVersion)))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}


void UBDocumentController::showMessage(const QString& message, bool showSpinningWheel)
{
    int margin = UBSettings::boardMargin;

    QRect newSize = mDocumentUI->thumbnailWidget->geometry();

    if (mMessageWindow)
    {
        mMessageWindow->move(margin, newSize.height() - mMessageWindow->height() - margin);
        mMessageWindow->showMessage(message, showSpinningWheel);
    }
}


void UBDocumentController::hideMessage()
{
    if (mMessageWindow)
        mMessageWindow->hideMessage();
}


void UBDocumentController::addImages()
{
    UBDocumentProxy* document = selectedDocumentProxy();

    if (document)
    {
        QString defaultPath = UBSettings::settings()->lastImportFolderPath->get().toString();

        QString extensions;

        foreach (QString ext, UBSettings::settings()->imageFileExtensions)
        {
            extensions += " *.";
            extensions += ext;
        }

        QStringList images = QFileDialog::getOpenFileNames(mParentWidget, tr("Add all Images to Document"),
                defaultPath, tr("All Images (%1)").arg(extensions));

        if (images.length() > 0)
        {
            QFileInfo firstImage(images.at(0));

            UBSettings::settings()->lastImportFolderPath->set(QVariant(firstImage.absoluteDir().absolutePath()));

            int importedImageNumber
                  = UBDocumentManager::documentManager()->addImageAsPageToDocument(images, document);

            if (importedImageNumber == 0)
            {
                UBApplication::showMessage(tr("Selection does not contain any image files!"));
                UBApplication::applicationController->showDocument();
            }
            else
            {
                document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
                UBMetadataDcSubsetAdaptor::persist(document);
                mDocumentThumbs = UBThumbnailAdaptor::load(selectedDocumentProxy());
                refreshDocumentThumbnailsView();
            }
        }
    }
}


void UBDocumentController::toggleDocumentToolsPalette()
{
    if (!mToolsPalette->isVisible() && !mToolsPalettePositionned)
    {
        mToolsPalette->adjustSizeAndPosition();
        int left = controlView()->width() - 20 - mToolsPalette->width();
        int top = (controlView()->height() - mToolsPalette->height()) / 2;

        mToolsPalette->setCustomPosition(true);
        mToolsPalette->move(left, top);

        mToolsPalettePositionned = true;
    }

    bool visible = mToolsPalette->isVisible();
    mToolsPalette->setVisible(!visible);
}


void UBDocumentController::cut()
{
    // TODO - implemented me
}


void UBDocumentController::copy()
{
    // TODO - implemented me
}


void UBDocumentController::paste()
{
    // TODO - implemented me
}


void UBDocumentController::focusChanged(QWidget *old, QWidget *current)
{
    Q_UNUSED(old);

    if (current == mDocumentUI->thumbnailWidget)
    {
        if (mDocumentUI->thumbnailWidget->selectedItems().count() > 0)
            mSelectionType = Page;
        else
            mSelectionType = None;
    }
    else if (current == mDocumentUI->documentTreeWidget)
    {
        if (selectedDocumentProxy())
            mSelectionType = Document;
        else if (selectedDocumentGroupTreeItem())
            mSelectionType = Folder;
        else
            mSelectionType = None;
    }
    else if (current == mDocumentUI->documentZoomSlider)
    {
        if (mDocumentUI->thumbnailWidget->selectedItems().count() > 0)
            mSelectionType = Page;
        else
            mSelectionType = None;
    }
    else
    {
        if (old != mDocumentUI->thumbnailWidget &&
            old != mDocumentUI->documentTreeWidget &&
            old != mDocumentUI->documentZoomSlider)
        {
            mSelectionType = None;
        }
    }

    selectionChanged();
}

void UBDocumentController::deletePages(QList<QGraphicsItem *> itemsToDelete)
{
    if (itemsToDelete.count() > 0)
    {
        QList<int> sceneIndexes;
        UBDocumentProxy* proxy = 0;

        foreach (QGraphicsItem* item, itemsToDelete)
        {
            UBSceneThumbnailPixmap* thumb = dynamic_cast<UBSceneThumbnailPixmap*> (item);

            if (thumb)
            {
                proxy = thumb->proxy();
                if (proxy)
                {
                    sceneIndexes.append(thumb->sceneIndex());
                }
            }
        }

        if(UBApplication::mainWindow->yesNoQuestion(tr("Remove Page"), tr("Are you sure you want to remove %n page(s) from the selected document '%1'?", "", sceneIndexes.count()).arg(proxy->metaData(UBSettings::documentName).toString())))
        {
            UBPersistenceManager::persistenceManager()->deleteDocumentScenes(proxy, sceneIndexes);
            proxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(proxy);

            int offset = 0;
            foreach(int index, sceneIndexes)
            {
                mDocumentThumbs.removeAt(index - offset);
                offset++;
            }
            refreshDocumentThumbnailsView();

            int minIndex = proxy->pageCount() - 1;

            foreach (int i, sceneIndexes)
                 minIndex = qMin(i, minIndex);

            mDocumentUI->thumbnailWidget->selectItemAt(minIndex);
        }
    }
}

int UBDocumentController::getSelectedItemIndex()
{
    QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();

    if (selectedItems.count() > 0)
    {
        UBSceneThumbnailPixmap* thumb = dynamic_cast<UBSceneThumbnailPixmap*> (selectedItems.last());
        return thumb->sceneIndex();
    }
    else return -1;
}

bool UBDocumentController::pageCanBeMovedUp(int page)
{
	if(UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool())
		return page >= 2;
	else
		return page >= 1;
}

bool UBDocumentController::pageCanBeMovedDown(int page)
{
	if(UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool())
		return page != 0 && page < mCurrentDocument->pageCount() - 1;
	else
		return page < mCurrentDocument->pageCount() - 1;
}

bool UBDocumentController::pageCanBeDuplicated(int page)
{
	return page != 0;
}

bool UBDocumentController::pageCanBeDeleted(int page)
{
	return page != 0;
}
