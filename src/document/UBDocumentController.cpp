/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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
#include "core/UBMimeData.h"
#include "core/UBForeignObjectsHandler.h"

#include "adaptors/UBExportPDF.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBWidgetUpgradeAdaptor.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBDrawingController.h"


#include "gui/UBThumbnailView.h"
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

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
typedef Qt::SplitBehaviorFlags SplitBehavior;
#else
typedef QString::SplitBehavior SplitBehavior;
#endif

static bool lessThan(UBDocumentTreeNode *lValue, UBDocumentTreeNode *rValue)
{
    if (lValue->nodeType() == UBDocumentTreeNode::Catalog) {
        if (rValue->nodeType() == UBDocumentTreeNode::Catalog) {
            return lValue->nodeName() < rValue->nodeName();
        } else {
            return true;
        }
    } else {
        if (rValue->nodeType() == UBDocumentTreeNode::Catalog) {
            return false;
        } else {
            Q_ASSERT(lValue->proxyData());
            Q_ASSERT(rValue->proxyData());

            //N/C - NNE - 20140402 : Default order set to alphabetical order
            //return lValue->nodeName() < rValue->nodeName();

            QDateTime lTime = lValue->proxyData()->documentDate();
            QDateTime rTime = rValue->proxyData()->documentDate();

            return lTime > rTime;
        }
    }

    return false;
}



UBDocumentReplaceDialog::UBDocumentReplaceDialog(const QString &pIncommingName, const QStringList &pFileList, QWidget *parent, Qt::WindowFlags pFlags)
    : QDialog(parent, pFlags)
    , mFileNameList(pFileList)
    , mIncommingName(pIncommingName)
    , acceptText(tr("Accept"))
    , replaceText(tr("Replace"))
    , cancelText(tr("Cancel"))
    , mLabelText(0)
{
    this->setStyleSheet("background:white;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QVBoxLayout *labelLayout = new QVBoxLayout();

    mLabelText = new QLabel(labelTextWithName(pIncommingName), this);
    mLineEdit = new QLineEdit(this);
    mLineEdit->setText(pIncommingName);
    mLineEdit->selectedText();

    labelLayout->addWidget(mLabelText);
    labelLayout->addWidget(mLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    acceptButton = new QPushButton(acceptText, this);
    QPushButton *cancelButton = new QPushButton(cancelText, this);
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(labelLayout);
    mainLayout->addLayout(buttonLayout);

    acceptButton->setEnabled(false);

    connect(acceptButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(mLineEdit, SIGNAL(textEdited(QString)), this, SLOT(reactOnTextChanged(QString)));

    reactOnTextChanged(mIncommingName);
}

void UBDocumentReplaceDialog::setRegexp(const QRegularExpression pRegExp)
{
    mValidator->setRegularExpression(pRegExp);
}
bool UBDocumentReplaceDialog::validString(const QString &pStr)
{
    Q_UNUSED(pStr);
    return mLineEdit->hasAcceptableInput();
}

void UBDocumentReplaceDialog::setFileNameAndList(const QString &fileName, const QStringList &pLst)
{
    mFileNameList = pLst;
    mIncommingName = fileName;
    mLabelText->setText(labelTextWithName(fileName));
    mLineEdit->setText(fileName);
    mLineEdit->selectAll();
    mLineEdit->selectedText();
}

QString UBDocumentReplaceDialog::labelTextWithName(const QString &documentName) const
{
    return tr("The name %1 is allready used.\nKeeping this name will replace the document.\nProviding a new name will create a new document.")
            .arg(documentName);
}

void UBDocumentReplaceDialog::accept()
{
    QDialog::accept();
}
void UBDocumentReplaceDialog::reject()
{
    mLineEdit->clear();
    emit closeDialog();

    QDialog::reject();
}

void UBDocumentReplaceDialog::reactOnTextChanged(const QString &pStr)
{
//     if !mFileNameList.contains(pStr.trimmed(), Qt::CaseSensitive)

    if (!validString(pStr)) {
        acceptButton->setEnabled(false);
        mLineEdit->setStyleSheet("background:#FFB3C8;");
        acceptButton->setEnabled(false);

    } else if (mFileNameList.contains(pStr.trimmed(), Qt::CaseSensitive)) {
        acceptButton->setEnabled(true);
        mLineEdit->setStyleSheet("background:#FFB3C8;");
        acceptButton->setText(replaceText);

    } else {
        acceptButton->setEnabled(true);
        mLineEdit->setStyleSheet("background:white;");
        acceptButton->setText(acceptText);
    }
}

UBDocumentTreeNode::UBDocumentTreeNode(Type pType, const QString &pName, const QString &pDisplayName, UBDocumentProxy *pProxy ) :
    mType(pType)
  , mName(pName)
  , mDisplayName(pDisplayName)
  , mProxy(pProxy)
{
    if (pDisplayName.isEmpty()) {
        mDisplayName = mName;
    }
    mParent = 0;
}

void UBDocumentTreeNode::addChild(UBDocumentTreeNode *pChild)
{
    if (pChild) {
        mChildren += pChild;
        pChild->mParent = this;
    }
}

void UBDocumentTreeNode::insertChild(int pIndex, UBDocumentTreeNode *pChild)
{
    if (pChild) {
        mChildren.insert(pIndex, pChild);
        pChild->mParent = this;
    }
}

void UBDocumentTreeNode::moveChild(UBDocumentTreeNode *child, int index, UBDocumentTreeNode *newParent)
{
    int childIndex = mChildren.indexOf(child);
    if (childIndex == -1) {
        return;
    }

    newParent->insertChild(index, child);
    mChildren.removeAt(childIndex);
}

void UBDocumentTreeNode::removeChild(int index)
{
    if (index < 0 || index > mChildren.count() - 1) {
        return;
    }

    UBDocumentTreeNode *curChild = mChildren[index];
    while (curChild->mChildren.count()) {
        curChild->removeChild(0);
    }

    mChildren.removeAt(index);
    delete curChild;
}

UBDocumentTreeNode *UBDocumentTreeNode::clone()
{
    return new UBDocumentTreeNode(this->mType
                                  , this->mName
                                  , this->mDisplayName
                                  , this->mProxy ? new UBDocumentProxy(*this->mProxy)
                                                 : 0);
}

QString UBDocumentTreeNode::dirPathInHierarchy()
{
    QString result;
    UBDocumentTreeNode *curNode = this;
    //protect the 2nd level items
    while (curNode->parentNode() && !curNode->isTopLevel()) {
        result.prepend(curNode->parentNode()->nodeName() + "/");
        curNode = curNode->parentNode();
    }

    if (result.endsWith("/")) {
        result.truncate(result.count() - 1);
    }

    return result;
}

UBDocumentTreeNode::~UBDocumentTreeNode()
{
    foreach (UBDocumentTreeNode *curChildren, mChildren) {
        delete(curChildren);
        curChildren = 0;
    }
    if (mProxy)
        delete mProxy;
}

//issue 1629 - NNE - 20131105
bool UBDocumentTreeNode::findNode(UBDocumentTreeNode *node)
{
    UBDocumentTreeNode *parent = node->parentNode();

    bool hasFound = false;

    while(parent){
        if(parent == this){
            hasFound = true;
            break;
        }

        parent = parent->parentNode();
    }

    return hasFound;
}

UBDocumentTreeNode *UBDocumentTreeNode::nextSibling()
{
    UBDocumentTreeNode *parent = this->parentNode();
    UBDocumentTreeNode *nextSibling = NULL;

    int myIndex = parent->children().indexOf(this);
    int indexOfNextSibling = myIndex + 1;

    if(indexOfNextSibling < parent->children().size()){
        nextSibling = parent->children().at(indexOfNextSibling);
    }

    return nextSibling;
}

UBDocumentTreeNode *UBDocumentTreeNode::previousSibling()
{
    UBDocumentTreeNode *parent = this->parentNode();
    UBDocumentTreeNode *previousSibling = NULL;

    int myIndex = parent->children().indexOf(this);
    int indexOfPreviousSibling = myIndex - 1;

    if(indexOfPreviousSibling >= 0){
        previousSibling = parent->children().at(indexOfPreviousSibling);
    }

    return previousSibling;
}

//issue 1629 - NNE - 20131105 : END

UBDocumentTreeModel::UBDocumentTreeModel(QObject *parent) :
    QAbstractItemModel(parent)
  , mRootNode(0)
  , mCurrentNode(nullptr)
{
    UBDocumentTreeNode *rootNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, "root");

    QString trashName = UBSettings::trashedDocumentGroupNamePrefix;

    UBDocumentTreeNode *myDocsNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, UBPersistenceManager::myDocumentsName, tr("My documents"));
    rootNode->addChild(myDocsNode);
    //UBDocumentTreeNode *modelsNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, UBPersistenceManager::modelsName, tr("Models"));
    //rootNode->addChild(modelsNode);
    UBDocumentTreeNode *trashNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, trashName, tr("Trash"));
    rootNode->addChild(trashNode);
    //UBDocumentTreeNode *untitledDocumentsNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, UBPersistenceManager::untitledDocumentsName, tr("Untitled documents"));
    //myDocsNode->addChild(untitledDocumentsNode);

    setRootNode(rootNode);

    mRoot = index(0, 0, QModelIndex());
    mMyDocuments =  index(0, 0, QModelIndex());
    mTrash =  index(1, 0, QModelIndex());
    mUntitledDocuments = index(0, 0, mMyDocuments);
    mAscendingOrder = true;
}

QModelIndex UBDocumentTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!mRootNode || row < 0 || column < 0) {
        return QModelIndex();
    }

    UBDocumentTreeNode *nodeParent = nodeFromIndex(parent);
    if (!nodeParent || row > nodeParent->children().count() - 1) {
        return QModelIndex();
    }

    UBDocumentTreeNode *requiredNode = nodeParent->children().at(row);
    if(!requiredNode) {
        return QModelIndex();
    }

    QModelIndex resIndex = createIndex(row, column, requiredNode);

    return resIndex;
}

QModelIndex UBDocumentTreeModel::parent(const QModelIndex &child) const
{
    UBDocumentTreeNode *nodeChild = nodeFromIndex(child);
    if (!nodeChild) {
        return QModelIndex();
    }

    UBDocumentTreeNode *nodeParent = nodeChild->parentNode();
    if (!nodeParent) {
        return QModelIndex();
    }

    UBDocumentTreeNode *nodePreParent = nodeParent->parentNode();
    if (!nodePreParent) {
        return QModelIndex();
    }

    int row = nodePreParent->children().indexOf(nodeParent);

    QModelIndex resIndex = createIndex(row, 0, nodeParent);

    return resIndex;
}

int UBDocumentTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    UBDocumentTreeNode *nodeParent = nodeFromIndex(parent);
    if (!nodeParent) {
        return 0;
    }

    return nodeParent->children().count();
}

int UBDocumentTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    //N/C - NNE - 20140403
    return 3;
}

QVariant UBDocumentTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    UBDocumentTreeNode *dataNode = nodeFromIndex(index);

    if (!dataNode)
        return QVariant();


    if(role == Qt::DisplayRole){
        if(index.column() == 0){
            return dataNode->displayName();
        }else{
            UBDocumentProxy *proxy = proxyForIndex(index);

            QString displayText = "";

            if(proxy){
                QDateTime d;

                if(index.column() == 1){
                    d = proxy->documentDate();
                }else if(index.column() == 2){
                    d = proxy->lastUpdate();
                }

                displayText = d.toString("dd/MM/yyyy hh:mm");
            }

            return displayText;

        }
    }

    if(role == UBDocumentTreeModel::CreationDate){
        return findNodeDate(dataNode, UBSettings::documentDate);
    }

    if(role == UBDocumentTreeModel::UpdateDate){
        return findNodeDate(dataNode, UBSettings::documentUpdatedAt);
    }

    if(role == Qt::BackgroundRole){
        if (isConstant(index)) {
            return QBrush(0xD9DFEB);
        }

        if (mHighLighted.isValid() && index == mHighLighted) {
            return QBrush(0x6682B5);
        }
    }

    if(role == Qt::UserRole +1){
        return QVariant::fromValue(dataNode);
    }

    if (index.column() == 0) {
        switch (role) {
        case (Qt::DecorationRole) :
            if (mCurrentNode && mCurrentNode == dataNode) {
                return QIcon(":images/currentDocument.png");
            } else {
                if (index == trashIndex()) {
                    return QIcon(":images/trash.png");
                } else if (isConstant(index)) {
                    return QIcon(":images/libpalette/ApplicationsCategory.svg");
                }
                switch (static_cast<int>(dataNode->nodeType()))
                {
                    case UBDocumentTreeNode::Catalog :
                        return QIcon(":images/folder.png");
                    case UBDocumentTreeNode::Document :
                        return QIcon(":images/toolbar/board.png");
                }
            }
            break;
        case (Qt::FontRole) :
            if (isConstant(index)) {
                QFont font;
                font.setBold(true);
                return font;
            }
            break;
        case (Qt::ForegroundRole) :
            if (isConstant(index)) {
                return QColor(Qt::darkGray);
            }
            break;
        }
    }

    return QVariant();
}

bool UBDocumentTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    switch (role) {
    case Qt::EditRole:
        if (!index.isValid() || value.toString().isEmpty()) {
            return false;
        }

        QString fullNewName = value.toString();
        if (isCatalog(index))
        {
            fullNewName.replace('/', '-');
        }

        setNewName(index, fullNewName);
        return true;
    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags UBDocumentTreeModel::flags (const QModelIndex &index) const
{
    Qt::ItemFlags resultFlags = QAbstractItemModel::flags(index);
    UBDocumentTreeNode *indexNode = nodeFromIndex(index);

    if ( index.isValid() ) {
        if (!indexNode->isRoot() && !isConstant(index)) {
            if (!inTrash(index)) {
                resultFlags |= Qt::ItemIsEditable;
            }
            resultFlags |= Qt::ItemIsDragEnabled;
        }
        if (indexNode->nodeType() == UBDocumentTreeNode::Catalog) {
            resultFlags |= Qt::ItemIsDropEnabled;
        }
    }

    return resultFlags;
}

//N/C - NNE -20140407
QDateTime UBDocumentTreeModel::findNodeDate(UBDocumentTreeNode *node, QString type) const
{
    if(type == UBSettings::documentDate){
        return findCatalogCreationDate(node);
    }else if(type == UBSettings::documentUpdatedAt){
        return findCatalogUpdatedDate(node);
    }

    return QDateTime();
}

QDateTime UBDocumentTreeModel::findCatalogUpdatedDate(UBDocumentTreeNode *node) const
{
    UBDocumentProxy *proxy = node->proxyData();

    if(proxy){
        return proxy->metaData(UBSettings::documentUpdatedAt).toDateTime();
    }else if(node->children().size() > 0){
        QDateTime d = findCatalogUpdatedDate(node->children().at(0));

        for(int i = 1; i < node->children().size(); i++){
            QDateTime dChild = findCatalogUpdatedDate(node->children().at(i));

            if(dChild != QDateTime()){
                if(mAscendingOrder){
                    d = qMin(d, dChild);
                }else{
                    d = qMax(d, dChild);
                }
            }

        }

        return d;
    }

    return QDateTime();
}

QDateTime UBDocumentTreeModel::findCatalogCreationDate(UBDocumentTreeNode *node) const
{
    UBDocumentProxy *proxy = node->proxyData();

    if(proxy){
        return proxy->metaData(UBSettings::documentDate).toDateTime();
    }else if(node->children().size() > 0){
        QDateTime d = findCatalogCreationDate(node->children().at(0));

        for(int i = 1; i < node->children().size(); i++){
            QDateTime dChild = findCatalogCreationDate(node->children().at(i));

            if(dChild != QDateTime()){
                if(mAscendingOrder){
                    d = qMin(d, dChild);
                }else{
                    d = qMax(d, dChild);
                }
            }

        }

        return d;
    }

    return QDateTime();
}
//N/C - NNE -20140407 : END

QStringList UBDocumentTreeModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list" << "image/png" << "image/tiff" << "image/gif" << "image/jpeg";
    return types;
}

QMimeData *UBDocumentTreeModel::mimeData (const QModelIndexList &indexes) const
{
    UBDocumentTreeMimeData *mimeData = new UBDocumentTreeMimeData();
    QList <QModelIndex> indexList;
    QList<QUrl> urlList;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            indexList.append(index);
            urlList.append(QUrl());
        }
    }

#if defined(Q_OS_OSX)
    #if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
        if (QOperatingSystemVersion::current().majorVersion() == 10 && QOperatingSystemVersion::current().minorVersion() < 15) /* <= Mojave */
            mimeData->setUrls(urlList);
    #endif
#else
    mimeData->setUrls(urlList);
#endif
    mimeData->setIndexes(indexList);

    return mimeData;
}

bool UBDocumentTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction) {
        return false;
    }

    if (data->hasFormat(UBApplication::mimeTypeUniboardPage)) {
        UBDocumentTreeNode *curNode = nodeFromIndex(index(row - 1, column, parent));
        UBDocumentProxy *targetDocProxy = curNode->proxyData();
        const UBMimeData *ubMime = qobject_cast <const UBMimeData*>(data);
        if (!targetDocProxy || !ubMime || !ubMime->items().count()) {
            qDebug() << "an error ocured while parsing " << UBApplication::mimeTypeUniboardPage;
            return false;
        }

//        int count = 0;
        int total = ubMime->items().size();

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        foreach (UBMimeDataItem sourceItem, ubMime->items())
        {
            UBDocumentProxy *fromProxy = sourceItem.documentProxy();
            int fromIndex = sourceItem.sceneIndex();
            int toIndex = targetDocProxy->pageCount();

            UBPersistenceManager::persistenceManager()->copyDocumentScene(fromProxy, fromIndex,
                                                                          targetDocProxy, toIndex);
        }

        QApplication::restoreOverrideCursor();

        UBApplication::applicationController->showMessage(tr("%1 pages copied", "", total).arg(total), false);

        return true;
    }

    const UBDocumentTreeMimeData *mimeData = qobject_cast<const UBDocumentTreeMimeData*>(data);
    if (!mimeData) {
        qDebug() << "Incorrect mimeData, only internal one supported";
        return false;
    }

    if (!parent.isValid()) {
        return false;
    }

    UBDocumentTreeNode *newParentNode = nodeFromIndex(parent);

    if (!newParentNode) {
        qDebug() << "incorrect incoming parent node;";
        return false;
    }

    QList<QModelIndex> incomingIndexes = mimeData->indexes();

    foreach (QModelIndex curIndex, incomingIndexes)
    {
        //Issue N/C - NNE - 20140528 : use just the index on the first column
        if(curIndex.column() == 0){
            QModelIndex clonedTopLevel = copyIndexToNewParent(curIndex, parent, action == Qt::MoveAction ? aReference : aContentCopy);
            if (nodeFromIndex(curIndex) == mCurrentNode && action == Qt::MoveAction) {
                emit currentIndexMoved(clonedTopLevel, curIndex);
            }
        }
    }

    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    return true;
}

bool UBDocumentTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row + count > rowCount(parent))
        return false;

    beginRemoveRows( parent, row, row + count - 1);

    UBDocumentTreeNode *parentNode = nodeFromIndex(parent);
    for (int i = row; i < row + count; i++) {
        UBDocumentTreeNode *curChildNode = parentNode->children().at(i);
        QModelIndex curChildIndex = parent.model()->index(i, 0, parent);
        if (curChildNode) {
            if (rowCount(curChildIndex)) {
                while (rowCount(curChildIndex)) {
                    removeRows(0, 1, curChildIndex);
                }
            }
        }
        mNewDocuments.removeAll(curChildNode->proxyData());
        parentNode->removeChild(i);

    }

    endRemoveRows();
    return true;
}

bool UBDocumentTreeModel::containsDocuments(const QModelIndex &index)
{
    for (int i = 0; i < rowCount(index); i++)
    {
        QModelIndex child = this->index(i, 0, index);
        if (isCatalog(child))
        {
            if (containsDocuments(child))
            {
                return true;
            }
        }
        else if (isDocument(child))
        {
           return true;
        }
        else
        {
            qDebug() << "Who the hell are you ?";
        }
    }

    return false;
}

QModelIndex UBDocumentTreeModel::indexForNode(UBDocumentTreeNode *pNode) const
{
    if (pNode == 0) {
        return QModelIndex();
    }

    return pIndexForNode(QModelIndex(), pNode);
}

QPersistentModelIndex UBDocumentTreeModel::persistentIndexForNode(UBDocumentTreeNode *pNode)
{
    return QPersistentModelIndex(indexForNode(pNode));
}

UBDocumentTreeNode *UBDocumentTreeModel::findProxy(UBDocumentProxy *pSearch, UBDocumentTreeNode *pParent) const
{
    foreach (UBDocumentTreeNode *curNode, pParent->children())
    {
        if (UBDocumentTreeNode::Catalog != curNode->nodeType())
        {
            if (curNode->proxyData()->theSameDocument(pSearch))
                return curNode;
        }
        else if (curNode->children().count())
        {
            UBDocumentTreeNode *recursiveDescendResult = findProxy(pSearch, curNode);
            if (recursiveDescendResult)
                return findProxy(pSearch, curNode);
        }
    }

    return 0;
}

QModelIndex UBDocumentTreeModel::pIndexForNode(const QModelIndex &parent, UBDocumentTreeNode *pNode) const
{
    for (int i = 0; i < rowCount(parent); i++) {
        QModelIndex curIndex = index(i, 0, parent);
        if (curIndex.internalPointer() == pNode) {
            return curIndex;
        } else if (rowCount(curIndex) > 0) {
            QModelIndex recursiveDescendIndex = pIndexForNode(curIndex, pNode);
            if (recursiveDescendIndex.isValid()) {
                return recursiveDescendIndex;
            }
        }
    }
    return QModelIndex();
}

//N/C - NNE - 20140411
void UBDocumentTreeModel::copyIndexToNewParent(const QModelIndexList &list, const QModelIndex &newParent, eCopyMode pMode)
{
    for(int i = 0; i < list.size(); i++){
        if(list.at(i).column() == 0){
            copyIndexToNewParent(list.at(i), newParent, pMode);
        }
    }
}
//N/C - NNE - 20140411 : END

QPersistentModelIndex UBDocumentTreeModel::copyIndexToNewParent(const QModelIndex &source, const QModelIndex &newParent, eCopyMode pMode)
{
    UBDocumentTreeNode *nodeParent = nodeFromIndex(newParent);
    UBDocumentTreeNode *nodeSource = nodeFromIndex(source);

    if (!nodeParent || !nodeSource) {
        return QModelIndex();
    }

    //beginInsertRows(newParent, rowCount(newParent), rowCount(newParent));

    UBDocumentTreeNode *clonedNodeSource = 0;
    switch (static_cast<int>(pMode)) {
    case aReference:
        clonedNodeSource = nodeSource->clone();
        if (mNewDocuments.contains(nodeSource->proxyData())) { //update references for session documents
            mNewDocuments << clonedNodeSource->proxyData();

            UBPersistenceManager::persistenceManager()->reassignDocProxy(clonedNodeSource->proxyData(), nodeSource->proxyData());
        }
        break;

    case aContentCopy:
        UBDocumentProxy* duplicatedProxy = 0;
        if (nodeSource->nodeType() == UBDocumentTreeNode::Document && nodeSource->proxyData()) {
            duplicatedProxy = UBPersistenceManager::persistenceManager()->duplicateDocument(nodeSource->proxyData());
            duplicatedProxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(duplicatedProxy);
        }
        clonedNodeSource = new UBDocumentTreeNode(nodeSource->nodeType()
                                                  , nodeSource->nodeName()
                                                  , nodeSource->displayName()
                                                  , duplicatedProxy);
        break;
    }

    // Determine whether to provide a name with postfix if the name in current level already exists
    QString newName = clonedNodeSource->nodeName();
    if ((source.parent() != newParent
            || pMode != aReference)
            && (newParent != trashIndex() || !inTrash(newParent))) {
        newName = adjustNameForParentIndex(newName, newParent);
        clonedNodeSource->setNodeName(newName);
    }

    if (clonedNodeSource->proxyData()) {
        clonedNodeSource->proxyData()->setMetaData(UBSettings::documentGroupName, virtualPathForIndex(newParent));
        clonedNodeSource->proxyData()->setMetaData(UBSettings::documentName, newName);
        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(clonedNodeSource->proxyData());
    }

    addNode(clonedNodeSource, newParent);
    //endInsertRows();

    QPersistentModelIndex newParentIndex = createIndex(rowCount(newParent), 0, clonedNodeSource);

    if (rowCount(source)) {
        for (int i = 0; i < rowCount(source); i++) {
            QModelIndex curNewParentIndexChild = source.model()->index(i, 0, source);
            copyIndexToNewParent(curNewParentIndexChild, newParentIndex, pMode);
        }
    }

    return newParentIndex;
}

//N/C - NNE - 20140409
void UBDocumentTreeModel::moveIndexes(const QModelIndexList &source, const QModelIndex &destination)
{
    //Issue N/C - NNE - 20140528
    QModelIndex destinationParent = destination;
    while(!isCatalog(destinationParent)){
        destinationParent = destinationParent.parent();
    }

    UBDocumentTreeNode *newParentNode = nodeFromIndex(destinationParent);

    bool hasOneInsertion = false;

    for(int i = 0; i < source.size(); i++){
        UBDocumentTreeNode *sourceNode = nodeFromIndex(source.at(i));
        QModelIndex s = source.at(i);

        if(newParentNode == sourceNode->parentNode() || sourceNode->findNode(newParentNode))
            continue;

        if(s.internalId() != destinationParent.internalId()){
            int sourceIndex = source.at(i).row();
            int destIndex = positionForParent(sourceNode, newParentNode);

            beginMoveRows(s.parent(), sourceIndex, sourceIndex, destinationParent, destIndex);
            fixNodeName(s, destinationParent);
            sourceNode->parentNode()->moveChild(sourceNode, destIndex, newParentNode);
            updateIndexNameBindings(sourceNode);
            hasOneInsertion = true;
        }
    }

    if(hasOneInsertion)
        endMoveRows();
}

//N/C - NNE - 20140409

void UBDocumentTreeModel::moveIndex(const QModelIndex &what, const QModelIndex &destination)
{
    QModelIndexList list;
    list.push_back(what);
    moveIndexes(list, destination);
}

void UBDocumentTreeModel::setCurrentDocument(UBDocumentProxy *pDocument)
{
    UBDocumentTreeNode *testCurNode = findProxy(pDocument, mRootNode);

    if (testCurNode) {
        setCurrentNode(testCurNode);
    }
}

QModelIndex UBDocumentTreeModel::indexForProxy(UBDocumentProxy *pSearch) const
{
    UBDocumentTreeNode *proxy = findProxy(pSearch, mRootNode);
    if (!proxy) {
        return QModelIndex();
    }

    return indexForNode(proxy);
}

void UBDocumentTreeModel::setRootNode(UBDocumentTreeNode *pRoot)
{
    mRootNode = pRoot;
    //reset();
}

UBDocumentProxy *UBDocumentTreeModel::proxyForIndex(const QModelIndex &pIndex) const
{
    UBDocumentTreeNode *node = nodeFromIndex(pIndex);
    if (!node) {
        return 0;
    }

    return node->proxyData();
}

QString UBDocumentTreeModel::virtualDirForIndex(const QModelIndex &pIndex) const
{
    QString result;
    UBDocumentTreeNode *curNode = nodeFromIndex(pIndex);
    //protect the 2nd level items
    while (curNode->parentNode() && !curNode->isTopLevel()) {
        result.prepend(curNode->parentNode()->nodeName() + "/");
        curNode = curNode->parentNode();
    }

    if (result.endsWith("/")) {
        result.truncate(result.count() - 1);
    }

    return result;
}

QString UBDocumentTreeModel::virtualPathForIndex(const QModelIndex &pIndex) const
{
    UBDocumentTreeNode *curNode = nodeFromIndex(pIndex);
    Q_ASSERT(curNode);

    return virtualDirForIndex(pIndex) + "/" + curNode->nodeName();
}

QList<UBDocumentTreeNode*> UBDocumentTreeModel::nodeChildrenFromIndex(const QModelIndex &pIndex) const
{
    UBDocumentTreeNode *node = nodeFromIndex(pIndex);

    if (node)
        return node->children();
    else
        return QList<UBDocumentTreeNode*>();
}

QStringList UBDocumentTreeModel::nodeNameList(const QModelIndex &pIndex, bool distinctNodeType) const
{
    QStringList result;

    UBDocumentTreeNode *catalog = nodeFromIndex(pIndex);
    if (catalog->nodeType() != UBDocumentTreeNode::Catalog) {
        return QStringList();
    }

    foreach (UBDocumentTreeNode *curNode, catalog->children())
    {
        if (distinctNodeType)
        {
            if (curNode->nodeType() == UBDocumentTreeNode::Catalog)
            {
                result << "folder - " + curNode->nodeName();
            }
            else
            {
                result << curNode->nodeName();
            }
        }
        else
        {
            result << curNode->nodeName();
        }
    }

    return result;
}

bool UBDocumentTreeModel::newNodeAllowed(const QModelIndex &pSelectedIndex)  const
{
    if (!pSelectedIndex.isValid()) {
        return false;
    }

    if (inTrash(pSelectedIndex) || pSelectedIndex == trashIndex()) {
        return false;
    }

    return true;
}

QModelIndex UBDocumentTreeModel::goTo(const QString &dir)
{
    QStringList pathList = dir.split("/", SplitBehavior::SkipEmptyParts);

    if (pathList.isEmpty()) {
        return untitledDocumentsIndex();
    }

    if (pathList.first() != UBPersistenceManager::myDocumentsName
            && pathList.first() != UBSettings::trashedDocumentGroupNamePrefix
            && pathList.first() != UBPersistenceManager::modelsName) {
        pathList.prepend(UBPersistenceManager::myDocumentsName);
    }

    QModelIndex parentIndex;

    bool searchingNode = true;
    while (!pathList.isEmpty())
    {
        QString curLevelName = pathList.takeFirst();
        if (searchingNode) {
            searchingNode = false;
            int irowCount = rowCount(parentIndex);
            for (int i = 0; i < irowCount; ++i) {
                QModelIndex curChildIndex = index(i, 0, parentIndex);
                UBDocumentTreeNode* currentNode = nodeFromIndex(curChildIndex);
                if (currentNode->nodeName() == curLevelName && currentNode->nodeType() == UBDocumentTreeNode::Catalog)
                {
                    searchingNode = true;
                    parentIndex = curChildIndex;
                    break;
                }
            }
        }

        if (!searchingNode) {
            UBDocumentTreeNode *newChild = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, curLevelName);
            parentIndex = addNode(newChild, parentIndex);
        }
    }

    return parentIndex;
}

bool UBDocumentTreeModel::inTrash(const QModelIndex &index) const
{
    return isDescendantOf(index, trashIndex());
}

bool UBDocumentTreeModel::inUntitledDocuments(const QModelIndex &index) const
{
    return isDescendantOf(index, untitledDocumentsIndex());
}

//N/C - NNE - 20140408
bool UBDocumentTreeModel::inMyDocuments(const QModelIndex &index) const
{
    return isDescendantOf(index, myDocumentsIndex());
}
//N/C - NNE - 20140408 : END

void UBDocumentTreeModel::addDocument(UBDocumentProxy *pProxyData, const QModelIndex &pParent)
{
    if (!pProxyData) {
        return;
    }
    QString docName = pProxyData->metaData(UBSettings::documentName).toString();
    QString docGroupName = pProxyData->metaData(UBSettings::documentGroupName).toString();

    if (docName.isEmpty()) {
        return;
    }

    QModelIndex lParent = pParent;
    UBDocumentTreeNode *freeNode = new UBDocumentTreeNode(UBDocumentTreeNode::Document
                                                          , docName
                                                          , QString()
                                                          , pProxyData);
    if (!pParent.isValid()) {
        lParent = goTo(docGroupName);
    }

    addNode(freeNode, lParent);
}

void UBDocumentTreeModel::addNewDocument(UBDocumentProxy *pProxyData, const QModelIndex &pParent)
{
    addDocument(pProxyData, pParent);
    mNewDocuments << pProxyData;
}

QModelIndex UBDocumentTreeModel::addCatalog(const QString &pName, const QModelIndex &pParent)
{
    if (pName.isEmpty() || !pParent.isValid()) {
        return QModelIndex();
    }

    UBDocumentTreeNode *catalogNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, pName);
    return addNode(catalogNode, pParent);
}

void UBDocumentTreeModel::setNewName(const QModelIndex &index, const QString &newName)
{
    if (!index.isValid()) {
        return;
    }

    UBDocumentTreeNode *indexNode = nodeFromIndex(index);

    QString magicSeparator = "+!##s";
    if (isCatalog(index)) {
        QString fullNewName = newName;
        if (!newName.contains(magicSeparator)) {
            indexNode->setNodeName(fullNewName);
            QString virtualDir = virtualDirForIndex(index);
            fullNewName.prepend(virtualDir.isEmpty() ? "" : virtualDir + magicSeparator);
        }
        for (int i = 0; i < rowCount(index); i++) {
            QModelIndex subIndex = this->index(i, 0, index);
            setNewName(subIndex, fullNewName + magicSeparator + subIndex.data().toString());
        }

    } else if (isDocument(index)) {
        Q_ASSERT(indexNode->proxyData());

        int prefixIndex = newName.lastIndexOf(magicSeparator);
        if (prefixIndex != -1) {
            QString newDocumentGroupName = newName.left(prefixIndex).replace(magicSeparator, "/");
            indexNode->proxyData()->setMetaData(UBSettings::documentGroupName, newDocumentGroupName);
        } else {
            indexNode->setNodeName(newName);
            indexNode->proxyData()->setMetaData(UBSettings::documentName, newName);
        }

        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(indexNode->proxyData());
    }
}

QString UBDocumentTreeModel::adjustNameForParentIndex(const QString &pName, const QModelIndex &pIndex)
{
    int i = 0;
    QString newName = pName;
    QStringList siblingNames = nodeNameList(pIndex);
    while (siblingNames.contains(newName)) {
        newName = pName + " " + QVariant(++i).toString();
    }

    return newName;
}

void UBDocumentTreeModel::fixNodeName(const QModelIndex &source, const QModelIndex &dest)
{
    // Determine whether to provide a name with postfix if the name in current level allready exists
    UBDocumentTreeNode *srcNode = nodeFromIndex(source);
    Q_ASSERT(srcNode);

    QString newName = srcNode->nodeName();
    if (source.parent() != dest
            && (dest != trashIndex()
            || !inTrash(dest))) {
        newName = adjustNameForParentIndex(newName, dest);
        srcNode->setNodeName(newName);
        nodeFromIndex(source)->setNodeName(newName);
    }
}

void UBDocumentTreeModel::updateIndexNameBindings(UBDocumentTreeNode *nd)
{
    Q_ASSERT(nd);

    if (nd->nodeType() == UBDocumentTreeNode::Catalog) {
        foreach (UBDocumentTreeNode *lnd, nd->children()) {
            updateIndexNameBindings(lnd);
        }
    } else if (nd->proxyData()) {
        nd->proxyData()->setMetaData(UBSettings::documentGroupName, virtualPathForIndex(indexForNode(nd->parentNode())));
        nd->proxyData()->setMetaData(UBSettings::documentName, nd->nodeName());
        nd->proxyData()->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(nd->proxyData());
    }
}

bool UBDocumentTreeModel::isDescendantOf(const QModelIndex &pPossibleDescendant, const QModelIndex &pPossibleAncestor) const
{
    if (!pPossibleDescendant.isValid()) {
        return false;
    }

    QModelIndex ancestor = pPossibleDescendant;
    while (ancestor.parent().isValid()) {
        ancestor = ancestor.parent();
        if (ancestor == pPossibleAncestor) {
            return true;
        }
    }

    return false;
}

QModelIndex UBDocumentTreeModel::addNode(UBDocumentTreeNode *pFreeNode, const QModelIndex &pParent, eAddItemMode pMode)
{
    UBDocumentTreeNode *tstParent = nodeFromIndex(pParent);

    if (!pParent.isValid() || tstParent->nodeType() != UBDocumentTreeNode::Catalog) {
        return QModelIndex();
    }
    int newIndex = pMode == aDetectPosition ? positionForParent(pFreeNode, tstParent): tstParent->children().size();
    beginInsertRows(pParent, newIndex, newIndex);
    tstParent->insertChild(newIndex, pFreeNode);
    endInsertRows();

    return createIndex(newIndex, 0, pFreeNode);
}

int UBDocumentTreeModel::positionForParent(UBDocumentTreeNode *pFreeNode, UBDocumentTreeNode *pParentNode)
{
    Q_ASSERT(pFreeNode);
    Q_ASSERT(pParentNode);
    Q_ASSERT(pParentNode->nodeType() == UBDocumentTreeNode::Catalog);

    int c = -1;
    int childCount = pParentNode->children().count();
    while (c <= childCount) {
        if (++c == childCount || lessThan(pFreeNode, pParentNode->children().at(c))) {
            break;
        }
    }
    return c == -1 ? childCount : c;
}

UBDocumentTreeNode *UBDocumentTreeModel::nodeFromIndex(const QModelIndex &pIndex) const
{
    if (pIndex.isValid()) {
        return static_cast<UBDocumentTreeNode*>(pIndex.internalPointer());
    } else {
        return mRootNode;
    }
}

bool UBDocumentTreeModel::nodeLessThan(const UBDocumentTreeNode *firstIndex, const UBDocumentTreeNode *secondIndex)
{
    return firstIndex->nodeName() < secondIndex->nodeName();
}

UBDocumentTreeModel::~UBDocumentTreeModel()
{
    delete mRootNode;
}

UBDocumentTreeView::UBDocumentTreeView(QWidget *parent) : QTreeView(parent)
{
    setObjectName("UBDocumentTreeView");
    setRootIsDecorated(true);
    setSelectionBehavior(SelectRows);
}

void UBDocumentTreeView::setSelectedAndExpanded(const QModelIndex &pIndex, bool pExpand, bool pEdit)
{
    if (!pIndex.isValid()) {
        return;
    }

    QModelIndex indexCurrentDoc = pIndex;
    clearSelection();

    UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(model());

    QItemSelectionModel::SelectionFlags sel = pExpand
                                                ? QItemSelectionModel::Select
                                                : QItemSelectionModel::Deselect;

    setCurrentIndex(indexCurrentDoc);

    selectionModel()->setCurrentIndex(proxy->mapFromSource(indexCurrentDoc), QItemSelectionModel::SelectCurrent);

    selectionModel()->select(proxy->mapFromSource(indexCurrentDoc), QItemSelectionModel::Rows | sel);

    while (indexCurrentDoc.parent().isValid()) {
        setExpanded(indexCurrentDoc.parent(), pExpand);
        indexCurrentDoc = indexCurrentDoc.parent();
    }

    scrollTo(proxy->mapFromSource(pIndex));

    if (pEdit)
        edit(proxy->mapFromSource(pIndex));
}

void UBDocumentTreeView::onModelIndexChanged(const QModelIndex &pNewIndex, const QModelIndex &pOldIndex)
{
    Q_UNUSED(pOldIndex)

    //N/C - NNE - 20140407
    QModelIndex indexSource = mapIndexToSource(pNewIndex);

    setSelectedAndExpanded(indexSource, true);
}

void UBDocumentTreeView::hSliderRangeChanged(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);

    QScrollBar *hScroller = horizontalScrollBar();
    if (hScroller)
    {
        hScroller->triggerAction(QAbstractSlider::SliderToMaximum);
    }
}

void UBDocumentTreeView::mousePressEvent(QMouseEvent *event)
{
    QTreeView::mousePressEvent(event);
    UBApplication::documentController->clearThumbnailsSelection();
}

void UBDocumentTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeView::dragEnterEvent(event);
    event->accept();
    event->acceptProposedAction();
}

void UBDocumentTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);

    UBDocumentTreeModel *docModel = 0;

    UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(model());

    if(proxy){
        docModel = dynamic_cast<UBDocumentTreeModel*>(proxy->sourceModel());
    }else{
        docModel =  dynamic_cast<UBDocumentTreeModel*>(model());
    }

    docModel->setHighLighted(QModelIndex());
    update();
}

void UBDocumentTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    QModelIndex index;
    if (selectedIndexes().count() > 0)
    {
        index = selectedIndexes().first();
    }

    bool acceptIt = isAcceptable(index, indexAt(event->pos()));

    if (event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage)) {
        UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(model());

        UBDocumentTreeModel *docModel = 0;

        if(proxy){
            docModel = dynamic_cast<UBDocumentTreeModel*>(proxy->sourceModel());
        }else{
            docModel =  dynamic_cast<UBDocumentTreeModel*>(model());
        }

        QModelIndex targetIndex = mapIndexToSource(indexAt(event->pos()));

        if (!docModel || !docModel->isDocument(targetIndex) || docModel->inTrash(targetIndex)) {
            event->ignore();
            event->setDropAction(Qt::IgnoreAction);
            docModel->setHighLighted(QModelIndex());
            acceptIt = false;
        } else {
            docModel->setHighLighted(targetIndex);
            acceptIt = true;
        }
        updateIndexEnvirons(indexAt(event->pos()));
    }
    QTreeView::dragMoveEvent(event);

    event->setAccepted(acceptIt);
}

void UBDocumentTreeView::dropEvent(QDropEvent *event)
{
    event->ignore();
    event->setDropAction(Qt::IgnoreAction);
    UBDocumentTreeModel *docModel = 0;

    //N/C - NNE - 20140408
    UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(model());
    if(proxy){
        docModel = dynamic_cast<UBDocumentTreeModel*>(proxy->sourceModel());
    }

    QModelIndex targetIndex = mapIndexToSource(indexAt(event->pos()));
    QModelIndexList dropIndex = mapIndexesToSource(selectedIndexes());

    //clear the selection right after
    //selectionModel()->clearSelection();

    bool isUBPage = event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage);

    //issue 1629 - NNE - 20131212
    bool targetIsInTrash = docModel->inTrash(targetIndex) || docModel->trashIndex() == targetIndex;
    bool targetIsInMyDocuments = docModel->inMyDocuments(targetIndex) || docModel->myDocumentsIndex() == targetIndex;

    if (!targetIsInMyDocuments && !targetIsInTrash)
        return;

    if (isUBPage)
    {
        UBDocumentProxy *targetDocProxy = docModel->proxyData(targetIndex);

        const UBMimeData *ubMime = qobject_cast <const UBMimeData*>(event->mimeData());
        if (!targetDocProxy || !ubMime || !ubMime->items().count()) {
            qDebug() << "an error ocured while parsing " << UBApplication::mimeTypeUniboardPage;
            QTreeView::dropEvent(event);
            return;
        }

        int count = 0;
        int total = ubMime->items().size();
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        foreach (UBMimeDataItem sourceItem, ubMime->items())
        {
            UBDocumentProxy *fromProxy = sourceItem.documentProxy();
            int fromIndex = sourceItem.sceneIndex();
            int toIndex = targetDocProxy->pageCount();            

            count++;

            UBApplication::applicationController->showMessage(tr("Copying page %1/%2").arg(count).arg(total), true);

            // TODO UB 4.x Move following code to some controller class
            UBGraphicsScene *scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(sourceItem.documentProxy(), sourceItem.sceneIndex());
            if (scene)
            {
                UBGraphicsScene* sceneClone = scene->sceneDeepCopy();

                UBDocumentProxy *targetDocProxy = docModel->proxyForIndex(targetIndex);

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

                QString thumbTmp(fromProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", fromIndex));
                QString thumbTo(targetDocProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", toIndex));

                QFile::remove(thumbTo);
                QFile::copy(thumbTmp, thumbTo);

                Q_ASSERT(QFileInfo(thumbTmp).exists());
                Q_ASSERT(QFileInfo(thumbTo).exists());

                auto pix = std::make_shared<QPixmap>(thumbTmp);
                UBApplication::documentController->insertExistingThumbPage(toIndex, pix);
                if (UBApplication::documentController->selectedDocument() == targetDocProxy)
                {
                    UBApplication::documentController->reloadThumbnails();
                }
                if (UBApplication::boardController->selectedDocument() == targetDocProxy)
                {
                    UBApplication::boardController->insertThumbPage(toIndex);
                }
            }

            QApplication::restoreOverrideCursor();

            docModel->setHighLighted(QModelIndex());
        }

        UBApplication::applicationController->showMessage(tr("%1 pages copied", "", total).arg(total), false);
    }
    else
    {
        if(targetIsInTrash)
        {
            if (!UBApplication::mainWindow->yesNoQuestion(tr("Remove Item"), tr("Are you sure you want to remove the selected item(s) ?")))
                return;

            UBApplication::documentController->moveIndexesToTrash(dropIndex, docModel);
        }else{
            docModel->moveIndexes(dropIndex, targetIndex);
        }
    }

    expand(proxy->mapFromSource(targetIndex));

    QTreeView::dropEvent(event);

    UBApplication::documentController->pageSelectionChanged();
}

void UBDocumentTreeView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);
}

void UBDocumentTreeView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QTreeView::rowsAboutToBeRemoved(parent, start, end);
}

bool UBDocumentTreeView::isAcceptable(const QModelIndex &dragIndex, const QModelIndex &atIndex)
{
    QModelIndex dragIndexSource = mapIndexToSource(dragIndex);
    QModelIndex atIndexSource = mapIndexToSource(atIndex);

    if (!dragIndexSource.isValid()) {
        return false;
    }

    return true;
}

Qt::DropAction UBDocumentTreeView::acceptableAction(const QModelIndex &dragIndex, const QModelIndex &atIndex)
{    
    return Qt::MoveAction;
}

void UBDocumentTreeView::updateIndexEnvirons(const QModelIndex &index)
{
    QRect updateRect = visualRect(index);
    const int multipler = 3;
    updateRect.adjust(0, -updateRect.height() * multipler, 0, updateRect.height() * multipler);
    update(updateRect);
}

//N/C - NNE - 20140404
QModelIndex UBDocumentTreeView::mapIndexToSource(const QModelIndex &index)
{
    UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(model());

    if(proxy){
        return proxy->mapToSource(index);
    }

    return index;
}

QModelIndexList UBDocumentTreeView::mapIndexesToSource(const QModelIndexList &indexes)
{
    UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(model());

    if(proxy){
        QModelIndexList list;

        for(int i = 0; i < indexes.size(); i++){
            list.push_back(proxy->mapToSource(indexes.at(i)));
        }

        return list;
    }

    return indexes;
}
//N/C - NNE - 20140404 : END

UBDocumentTreeItemDelegate::UBDocumentTreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void UBDocumentTreeItemDelegate::commitAndCloseEditor()
{
    QLineEdit *lineEditor = dynamic_cast<QLineEdit*>(sender());
    if (lineEditor)
    {
        if (lineEditor->hasAcceptableInput())
        {
            emit commitData(lineEditor);
        //emit closeEditor(lineEditor);
        }

        emit UBApplication::documentController->reorderDocumentsRequested();
    }
}

void UBDocumentTreeItemDelegate::processChangedText(const QString &str) const
{
    QLineEdit *editor = qobject_cast<QLineEdit*>(sender());
    if (editor)
    {
        if (editor->validator())
        {
            int pos = 0;
            if (editor->validator()->validate(const_cast<QString&>(str), pos) != QValidator::Acceptable)
            {
                editor->setStyleSheet("background-color: #FFB3C8;");
            }
            else
            {
                editor->setStyleSheet("background-color: #FFFFFF;");
            }
        }
    }
}

bool UBDocumentTreeItemDelegate::validateString(const QString &str) const
{
    return !mExistingFileNames.contains(str);
}

QWidget *UBDocumentTreeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    //N/C - NNE - 20140407 : Add the test for the index column.
    if(index.column() == 0){
        mExistingFileNames.clear();
        const UBDocumentTreeModel *docModel = 0;

        const UBSortFilterProxyModel *proxy = dynamic_cast<const UBSortFilterProxyModel*>(index.model());

        if(proxy){
            docModel = dynamic_cast<UBDocumentTreeModel*>(proxy->sourceModel());
        }else{
            docModel =  dynamic_cast<const UBDocumentTreeModel*>(index.model());
        }

        QModelIndex sourceIndex = proxy->mapToSource(index);

        if (docModel)
        {
            mExistingFileNames = docModel->nodeNameList(sourceIndex.parent());
            mExistingFileNames.removeOne(sourceIndex.data().toString());

            UBDocumentTreeNode* sourceNode = docModel->nodeFromIndex(sourceIndex);

            if (sourceNode)
            {
                QLineEdit *nameEditor = new QLineEdit(parent);
                QList<UBDocumentTreeNode*> nodeChildren = docModel->nodeChildrenFromIndex(sourceIndex.parent());
                nodeChildren.removeOne(sourceNode);

                UBValidator* validator = new UBValidator(nodeChildren, sourceNode->nodeType());
                nameEditor->setValidator(validator);
                connect(nameEditor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
                connect(nameEditor, SIGNAL(textChanged(QString)), this, SLOT(processChangedText(QString)));

                return nameEditor;
            }
        }

        return nullptr;
    }

    //N/C - NNe - 20140407 : the other column are not editable.
    return 0;
}

void UBDocumentTreeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 0) {
        QLineEdit *lineEditor = qobject_cast<QLineEdit*>(editor);
        lineEditor->setText(index.data().toString());
        lineEditor->selectAll();
    }
}

void UBDocumentTreeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEditor = qobject_cast<QLineEdit*>(editor);
    if (lineEditor)
    {
        int pos;
        QString input = lineEditor->text();
        if (lineEditor->validator())
        {
            if (lineEditor->validator()->validate(input, pos) == QValidator::Acceptable)
            {
                model->setData(index, input);
            }
        }
    }
}

void UBDocumentTreeItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

UBDocumentController::UBDocumentController(UBMainWindow* mainWindow)
   : UBDocumentContainer(mainWindow->centralWidget())
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
   , mDocumentTrashGroupName(tr("Trash"))
   , mDefaultDocumentGroupName(tr("Untitled Documents"))
   , mCurrentTreeDocument(0)
   , mCurrentIndexMoved(false)
{

    setupViews();
    setupToolbar();
    connect(this, SIGNAL(exportDone()), mMainWindow, SLOT(onExportDone()));
    connect(this, SIGNAL(documentThumbnailsUpdated(UBDocumentContainer*)), this, SLOT(refreshDocumentThumbnailsView(UBDocumentContainer*)));
    //connect(this, SIGNAL(documentPageInserted(int)), this, SLOT(insertThumbnail(int)));
    connect(this, SIGNAL(documentPageUpdated(int)), this, SLOT(updateThumbnail(int)));
    connect(this, SIGNAL(documentPageRemoved(int)), this, SLOT(removeThumbnail(int)));
    connect(this, SIGNAL(documentPageMoved(int, int)), this, SLOT(moveThumbnail(int, int)));
    connect(this, SIGNAL(reorderDocumentsRequested()), this, SLOT(reorderDocuments()));
}

UBDocumentController::~UBDocumentController()
{
   if (mDocumentUI)
       delete mDocumentUI;
}

void UBDocumentController::createNewDocument()
{
    UBPersistenceManager *pManager = UBPersistenceManager::persistenceManager();
    UBDocumentTreeModel *docModel = pManager->mDocumentTreeStructureModel;
    QModelIndex selectedIndex = firstSelectedTreeIndex();

    if (!selectedIndex.isValid())
        selectedIndex = docModel->myDocumentsIndex();

    QString groupName = docModel->isCatalog(selectedIndex)
                ? docModel->virtualPathForIndex(selectedIndex)
                : docModel->virtualDirForIndex(selectedIndex);


    QDateTime now = QDateTime::currentDateTime();
    QString documentName = "";
    if (docModel->isCatalog(selectedIndex))
    {
        documentName = docModel->adjustNameForParentIndex(now.toString(Qt::SystemLocaleShortDate), selectedIndex);
    }
    else
    {
        documentName = docModel->adjustNameForParentIndex(now.toString(Qt::SystemLocaleShortDate), selectedIndex.parent());
    }


    UBDocumentProxy *document = pManager->createDocument(groupName, documentName);

    selectDocument(document, true, false, true);

    if (document)
        pManager->mDocumentTreeStructureModel->markDocumentAsNew(document);

    pageSelectionChanged();
}

void UBDocumentController::selectDocument(UBDocumentProxy* proxy, bool setAsCurrentDocument, const bool onImport, const bool editMode)
{
    if (proxy==NULL)
    {
        setDocument(NULL);
        return;
    }

    if (setAsCurrentDocument)
    {
        UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->setCurrentDocument(proxy);
        QModelIndex indexCurrentDoc = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->indexForProxy(proxy);
        if (indexCurrentDoc.isValid())
        {
            mDocumentUI->documentTreeView->setSelectedAndExpanded(indexCurrentDoc, true, editMode);

            if (proxy != mBoardController->selectedDocument()) // only if wanted Document is different from document actually on Board,  // ALTI/AOU - 20140217
            {
                //issue 1629 - NNE - 20131105 : When set a current document, change in the board controller
                mBoardController->setActiveDocumentScene(proxy, 0, true, onImport);
            }
        }
        else
        {
            qWarning() << "an issue occured while trying to select current index in document tree";
        }
    }

    setDocument(proxy);
}

void UBDocumentController::createNewDocumentGroup()
{
    UBPersistenceManager *pManager = UBPersistenceManager::persistenceManager();
    UBDocumentTreeModel *docModel = pManager->mDocumentTreeStructureModel;
    QModelIndex selectedIndex = firstSelectedTreeIndex();
    if (!selectedIndex.isValid()) {
        selectedIndex = docModel->myDocumentsIndex();
    }
    QModelIndex parentIndex = docModel->isCatalog(selectedIndex)
            ? selectedIndex
            : selectedIndex.parent();

    QString newFolderName = docModel->adjustNameForParentIndex(tr("New Folder"), parentIndex);

    QModelIndex newIndex = docModel->addCatalog(newFolderName, parentIndex);
    mDocumentUI->documentTreeView->setSelectedAndExpanded(newIndex, true, true);

    pageSelectionChanged();
}


UBDocumentProxy* UBDocumentController::selectedDocumentProxy()
{
    return UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->proxyForIndex(firstSelectedTreeIndex());
}

QList<UBDocumentProxy*> UBDocumentController::selectedProxies()
{
    QList<UBDocumentProxy*> result;

    foreach (QModelIndex curIndex, mapIndexesToSource(mDocumentUI->documentTreeView->selectionModel()->selectedIndexes())) {
        result << UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->proxyForIndex(curIndex);
    }

    return result;
}

QModelIndexList UBDocumentController::selectedTreeIndexes()
{
    return mapIndexesToSource(mDocumentUI->documentTreeView->selectionModel()->selectedRows(0));
}

UBDocumentProxy* UBDocumentController::firstSelectedTreeProxy()
{
    return selectedProxies().count() ? selectedProxies().first() : 0;
}

void UBDocumentController::TreeViewSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    QModelIndex current_index = mapIndexToSource(current);

    //N/C - NNE  - 20140414
    //if the selection contains more than one object, don't show the thumbnail.
    //We have just to pass a null proxy to disable the display of thumbnail
    UBDocumentProxy *currentDocumentProxy = 0;

    if(current_index.isValid() && mDocumentUI->documentTreeView->selectionModel()->selectedRows(0).size() == 1)
    {
        currentDocumentProxy = docModel->proxyData(current_index);
        setDocument(currentDocumentProxy, false);
    }
    //N/C - NNE  - 20140414 : END


    if (mCurrentIndexMoved) {
        if (docModel->isDocument(current_index)) {
            docModel->setCurrentDocument(currentDocumentProxy);
        } else if (docModel->isCatalog(current_index)) {
            docModel->setCurrentDocument(0);
        }
        mCurrentIndexMoved = false;
    }
}

//N/C - NNE - 20140402 : workaround for using a proxy model
QModelIndex UBDocumentController::mapIndexToSource(const QModelIndex &index)
{
    UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(mDocumentUI->documentTreeView->model());

    if(proxy){
        return proxy->mapToSource(index);
    }

    return index;
}

QModelIndexList UBDocumentController::mapIndexesToSource(const QModelIndexList &indexes)
{
    UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(mDocumentUI->documentTreeView->model());

    if(proxy){
        QModelIndexList list;

        for(int i = 0; i < indexes.size(); i++){
            list.push_back(proxy->mapToSource(indexes.at(i)));
        }

        return list;
    }

    return indexes;
}

//N/C - NNE - 20140402 : END

void UBDocumentController::TreeViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    int nbIndexes = selected.indexes().count();

    if (nbIndexes) {
        QModelIndexList list = mDocumentUI->documentTreeView->selectionModel()->selectedRows();

        //if multi-selection
        if(list.count() > 1)
        {
            for (int i=0; i < list.count() ; i++)
            {
                QModelIndex newSelectedRow = list.at(i);
                QModelIndex parent = newSelectedRow.parent();
                bool isParentIsSelected = false;
                while(parent.isValid())
                {
                    isParentIsSelected |= (list.indexOf(parent) != -1);

                    if(isParentIsSelected)
                        break;

                    parent = parent.parent();
                }

                if(!isParentIsSelected)
                    TreeViewSelectionChanged(newSelectedRow, QModelIndex());
                else
                    mDocumentUI->documentTreeView->selectionModel()->select(newSelectedRow, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);

            }
        }
        else
        {
            TreeViewSelectionChanged(selected.indexes().at(0), QModelIndex());
        }
    }
}

void UBDocumentController::itemSelectionChanged(LastSelectedElementType newSelection)
{
    mSelectionType = newSelection;
    updateActions();
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
            adaptor->setAssociatedAction(currentExportAction);
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

#ifdef Q_OS_OSX
        mMainWindow->actionDelete->setShortcut(QKeySequence(Qt::Key_Backspace));
#else
        mMainWindow->actionDelete->setShortcut(QKeySequence(Qt::Key_Delete));
#endif

        connect(mMainWindow->actionDelete, SIGNAL(triggered()), this, SLOT(deleteSelectedItem()));
        connect(mMainWindow->actionDuplicate, SIGNAL(triggered()), this, SLOT(duplicateSelectedItem()));
        connect(mMainWindow->actionRename, SIGNAL(triggered()), this, SLOT(renameSelectedItem()));
        connect(mMainWindow->actionAddToWorkingDocument, SIGNAL(triggered()), this, SLOT(addToDocument()));

        UBDocumentTreeModel *model = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

        mSortFilterProxyModel = new UBSortFilterProxyModel();

        mSortFilterProxyModel->setSourceModel(model);

        // sort documents according to preferences
        int sortKind  = UBSettings::settings()->documentSortKind->get().toInt();
        int sortOrder = UBSettings::settings()->documentSortOrder->get().toInt();

        // update icon and button
        mDocumentUI->sortKind->setCurrentIndex(sortKind);
        if (sortOrder == UBDocumentController::DESC)
            mDocumentUI->sortOrder->setChecked(true);

        mDocumentUI->documentTreeView->setModel(mSortFilterProxyModel);

        mDocumentUI->documentTreeView->setItemDelegate(new UBDocumentTreeItemDelegate(this));
        mDocumentUI->documentTreeView->setDragEnabled(true);
        mDocumentUI->documentTreeView->setAcceptDrops(true);
        mDocumentUI->documentTreeView->viewport()->setAcceptDrops(true);
        mDocumentUI->documentTreeView->setDropIndicatorShown(true);
        mDocumentUI->documentTreeView->header()->setStretchLastSection(false);
        mDocumentUI->documentTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        mDocumentUI->documentTreeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        mDocumentUI->documentTreeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

        const int splitterLeftSize = UBSettings::settings()->documentSplitterLeftSize->get().toInt();
        const int splitterRightSize = UBSettings::settings()->documentSplitterRightSize->get().toInt();
        QList<int> splitterSizes;
        splitterSizes.append(splitterLeftSize);
        splitterSizes.append(splitterRightSize);
        mDocumentUI->splitter->setSizes(splitterSizes);

        //mDocumentUI->documentTreeView->hideColumn(1);
        mDocumentUI->documentTreeView->hideColumn(2);

        sortDocuments(sortKind, sortOrder);

        connect(mDocumentUI->sortKind, SIGNAL(activated(int)), this, SLOT(onSortKindChanged(int)));
        connect(mDocumentUI->sortOrder, SIGNAL(toggled(bool)), this, SLOT(onSortOrderChanged(bool)));

        connect(mDocumentUI->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(onSplitterMoved(int, int)));

        connect(mDocumentUI->documentTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(TreeViewSelectionChanged(QItemSelection,QItemSelection)));
        connect(UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel, SIGNAL(indexChanged(QModelIndex,QModelIndex))
                ,mDocumentUI->documentTreeView, SLOT(onModelIndexChanged(QModelIndex,QModelIndex)));
        connect(UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel, SIGNAL(currentIndexMoved(QModelIndex,QModelIndex))
                ,this, SLOT(currentIndexMoved(QModelIndex,QModelIndex)));

        connect(mDocumentUI->thumbnailWidget, SIGNAL(sceneDropped(UBDocumentProxy*, int, int)), this, SLOT(moveSceneToIndex ( UBDocumentProxy*, int, int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(resized()), this, SLOT(thumbnailViewResized()));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseDoubleClick(QGraphicsItem*,int)), this, SLOT(thumbnailPageDoubleClicked(QGraphicsItem*,int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseClick(QGraphicsItem*, int)), this, SLOT(pageClicked(QGraphicsItem*, int)));

        mDocumentUI->thumbnailWidget->setBackgroundBrush(UBSettings::documentViewLightColor);

        #ifdef Q_WS_MACX
            mMessageWindow = new UBMessageWindow(NULL);
        #else
            mMessageWindow = new UBMessageWindow(mDocumentUI->thumbnailWidget);
        #endif

        mMessageWindow->setCustomPosition(true);
        mMessageWindow->hide();
    }
}

void UBDocumentController::refreshDateColumns()
{
    if (UBSettings::settings()->documentSortKind->get().toInt() == UBDocumentController::Alphabetical)
    {
        if (!UBSettings::settings()->showDateColumnOnAlphabeticalSort->get().toBool())
        {
            mDocumentUI->documentTreeView->hideColumn(1);
            mDocumentUI->documentTreeView->hideColumn(2);
        }
        else
        {
            mDocumentUI->documentTreeView->showColumn(1);
            mDocumentUI->documentTreeView->hideColumn(2);
        }
    }
}

void UBDocumentController::reorderDocuments()
{
   int kindIndex = mDocumentUI->sortKind->currentIndex();
   int orderIndex = mDocumentUI->sortOrder->isChecked() ? UBDocumentController::DESC : UBDocumentController::ASC;

   sortDocuments(kindIndex, orderIndex);
}

void UBDocumentController::sortDocuments(int kind, int order)
{
    Qt::SortOrder sortOrder = Qt::AscendingOrder;
    if(order == UBDocumentController::DESC)
        sortOrder = Qt::DescendingOrder;

    if(kind == UBDocumentController::CreationDate){
        mSortFilterProxyModel->setSortRole(UBDocumentTreeModel::CreationDate);
        mSortFilterProxyModel->sort(1, sortOrder);
        mDocumentUI->documentTreeView->showColumn(1);
        mDocumentUI->documentTreeView->hideColumn(2);
    }else if(kind == UBDocumentController::UpdateDate){
        mSortFilterProxyModel->setSortRole(UBDocumentTreeModel::UpdateDate);
        mSortFilterProxyModel->sort(2, sortOrder);
        mDocumentUI->documentTreeView->hideColumn(1);
        mDocumentUI->documentTreeView->showColumn(2);
    }else{
        mSortFilterProxyModel->setSortRole(Qt::DisplayRole);
        mSortFilterProxyModel->sort(0, sortOrder);
        if (!UBSettings::settings()->showDateColumnOnAlphabeticalSort->get().toBool())
        {
            mDocumentUI->documentTreeView->hideColumn(1);
            mDocumentUI->documentTreeView->hideColumn(2);
        }
    }

    mDocumentUI->documentTreeView->setSelectedAndExpanded(firstSelectedTreeIndex(), true);
}

void UBDocumentController::onSortOrderChanged(bool order)
{
    int kindIndex = mDocumentUI->sortKind->currentIndex();
    int orderIndex = order ? UBDocumentController::DESC : UBDocumentController::ASC;

    sortDocuments(kindIndex, orderIndex);

    UBSettings::settings()->documentSortOrder->setInt(orderIndex);
}

void UBDocumentController::onSortKindChanged(int index)
{
    int orderIndex = mDocumentUI->sortOrder->isChecked() ? UBDocumentController::DESC : UBDocumentController::ASC;

    sortDocuments(index, orderIndex);

    UBSettings::settings()->documentSortKind->setInt(index);
}

void UBDocumentController::onSplitterMoved(int size, int index)
{
    Q_UNUSED(index);
    UBSettings::settings()->documentSplitterLeftSize->setInt(size);
    UBSettings::settings()->documentSplitterRightSize->setInt(controlView()->size().width()-size);
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
    selectDocument(mBoardController->selectedDocument());

    reorderDocuments();

    pageSelectionChanged();

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
            UBDocumentProxy* proxy = thumb->documentProxy();

            if (proxy && isOKToOpenDocument(proxy))
            {
                mBoardController->setActiveDocumentScene(proxy, thumb->sceneIndex());
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
                UBDocumentProxy *proxy = thumb->documentProxy();

                if (proxy)
                {
                    int sceneIndex = thumb->sceneIndex();
                    selectedSceneIndexes << sceneIndex;
                }
            }
        }
        if (selectedSceneIndexes.count() > 0)
        {
            duplicatePages(selectedSceneIndexes);
            if (selectedDocument() == selectedDocumentProxy())
            {
                reloadThumbnails();
            }
            selectedDocument()->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(selectedDocument());
            int selectedThumbnail = selectedSceneIndexes.last() + selectedSceneIndexes.size();
            mDocumentUI->thumbnailWidget->selectItemAt(selectedThumbnail);
            int sceneCount = selectedSceneIndexes.count();
            showMessage(tr("duplicated %1 page","duplicated %1 pages",sceneCount).arg(sceneCount), false);

            if (selectedDocument() == mBoardController->selectedDocument())
            {
                mBoardController->setActiveDocumentScene(selectedThumbnail);
                mBoardController->reloadThumbnails();
            }
        }
    }
    else
    {
        UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
        QModelIndex selectedIndex = firstSelectedTreeIndex();

        Q_ASSERT(!docModel->isConstant(selectedIndex) && !docModel->inTrash(selectedIndex));

        showMessage(tr("Duplicating Document %1").arg(""), true);

        docModel->copyIndexToNewParent(selectedIndex, selectedIndex.parent(), UBDocumentTreeModel::aContentCopy);

        showMessage(tr("Document %1 copied").arg(""), false);
    }

    emit reorderDocumentsRequested();
    pageSelectionChanged();
}

void UBDocumentController::deleteSelectedItem()
{
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    if (!UBApplication::mainWindow->yesNoQuestion(tr("Remove Item"), tr("Are you sure you want to remove the selected item(s) ?")))
        return;

    QModelIndexList indexes = selectedTreeIndexes();

    if (indexes.size() > 1)
    {
        deleteMultipleItems(indexes, docModel);
    }
    else if (indexes.size() == 1)
    {
        deleteSingleItem(indexes.at(0), docModel);
    }

    pageSelectionChanged();
}

void UBDocumentController::deleteMultipleItems(QModelIndexList indexes, UBDocumentTreeModel* docModel)
{
    DeletionType deletionForSelection = deletionTypeForSelection(mSelectionType, indexes.at(0), docModel);

    switch (deletionForSelection)
    {
        case DeletePage:
        {
            deletePages(mDocumentUI->thumbnailWidget->selectedItems());
            break;
        }
        case MoveToTrash:
        {
            moveIndexesToTrash(indexes, docModel);
            break;
        }
        case CompleteDelete:
        {
            for (int i =0; i < indexes.size(); i++)
            {
                deleteIndexAndAssociatedData(indexes.at(i));
            }
            emit documentThumbnailsUpdated(this);
            break;
        }
        case EmptyFolder:
        {
            for (int i =0; i < indexes.size(); i++)
            {
                QModelIndex currentIndex = indexes.at(i);
                if (currentIndex == docModel->myDocumentsIndex()) { //Emptying "My documents". Keeping Untitled Documents
                    int startInd = 0;
                    while (docModel->rowCount(currentIndex)) {
                        QModelIndex testSubINdecurrentIndex = docModel->index(startInd, 0, currentIndex);
                        if (testSubINdecurrentIndex == docModel->untitledDocumentsIndex()) {
                            emptyFolder(testSubINdecurrentIndex, MoveToTrash);
                            startInd++;
                            continue;
                        }
                        if (!testSubINdecurrentIndex.isValid()) {
                            break;
                        }
                        docModel->moveIndex(testSubINdecurrentIndex, docModel->trashIndex());
                    }
                    //issue 1629 - NNE - 20131105
                    //Here, we are sure that the current scene has been deleted
                    createNewDocumentInUntitledFolder();
                } else {
                    //issue 1629 - NNE - 20131105
                    //Check if we will delete the current scene
                    UBDocumentTreeNode *currentNode = docModel->nodeFromIndex(currentIndex);
                    bool deleteCurrentScene = currentNode->findNode(docModel->nodeFromIndex(docModel->currentIndex()));

                    emptyFolder(currentIndex, MoveToTrash); //Empty constant folder

                    if(deleteCurrentScene) createNewDocumentInUntitledFolder();
                }
            }

            break;
        }
        case EmptyTrash:
        {
            for (int i=0; i < indexes.size(); i++)
            {
                emptyFolder(indexes.at(i), CompleteDelete); // Empty trash folder
            }

            break;
        }
    }
}

void UBDocumentController::deleteSingleItem(QModelIndex currentIndex, UBDocumentTreeModel* docModel)
{
    DeletionType deletionForSelection = deletionTypeForSelection(mSelectionType, currentIndex, docModel);

    switch (deletionForSelection)
    {
        case DeletePage:
        {
            deletePages(mDocumentUI->thumbnailWidget->selectedItems());
            break;
        }
        case MoveToTrash:
        {
            moveToTrash(currentIndex, docModel);
            break;
        }
        case CompleteDelete:
        {
            deleteIndexAndAssociatedData(currentIndex);
            emit documentThumbnailsUpdated(this);
            break;
        }
        case EmptyFolder:
        {
            if (currentIndex == docModel->myDocumentsIndex()) { //Emptying "My documents". Keeping Untitled Documents
                int startInd = 0;
                while (docModel->rowCount(currentIndex)) {
                    QModelIndex testSubINdecurrentIndex = docModel->index(startInd, 0, currentIndex);
                    if (testSubINdecurrentIndex == docModel->untitledDocumentsIndex()) {
                        emptyFolder(testSubINdecurrentIndex, MoveToTrash);
                        startInd++;
                        continue;
                    }
                    if (!testSubINdecurrentIndex.isValid()) {
                        break;
                    }
                    docModel->moveIndex(testSubINdecurrentIndex, docModel->trashIndex());
                }
                //issue 1629 - NNE - 20131105
                //Here, we are sure that the current scene has been deleted
                createNewDocumentInUntitledFolder();
            } else {
                //issue 1629 - NNE - 20131105
                //Check if we will delete the current scene
                UBDocumentTreeNode *currentNode = docModel->nodeFromIndex(currentIndex);
                bool deleteCurrentScene = currentNode->findNode(docModel->nodeFromIndex(docModel->currentIndex()));

                emptyFolder(currentIndex, MoveToTrash); //Empty constant folder

                if(deleteCurrentScene) createNewDocumentInUntitledFolder();
            }

            break;
        }
        case EmptyTrash:
        {
            emptyFolder(currentIndex, CompleteDelete); // Empty trash folder
            break;
        }
    }
}

//N/C - NNE - 20140410
void UBDocumentController::moveIndexesToTrash(const QModelIndexList &list, UBDocumentTreeModel *docModel)
{
    QModelIndex currentScene = docModel->indexForNode(docModel->currentNode());

    //check if the current scene is selected
    QItemSelectionModel *selectionModel = mDocumentUI->documentTreeView->selectionModel();
    bool deleteCurrentScene = selectionModel->isSelected(mSortFilterProxyModel->mapFromSource(currentScene));

    //check if the current scene is in the hierarchy
    if(!deleteCurrentScene){
        for(int i = 0; i < list.size(); i++){
            deleteCurrentScene = docModel->isDescendantOf(currentScene, list.at(i));

            if(deleteCurrentScene){
                break;
            }
        }

    }

    QModelIndex proxyMapCurentScene = mSortFilterProxyModel->mapFromSource(currentScene);

    if(deleteCurrentScene){
        QModelIndex sibling = findPreviousSiblingNotSelected(proxyMapCurentScene, selectionModel);

        if(sibling.isValid()){
            QModelIndex sourceSibling = mSortFilterProxyModel->mapToSource(sibling);

            UBDocumentProxy *proxy = docModel->proxyForIndex(sourceSibling);

            if (proxy)
            {
                selectDocument(proxy,true);

                deleteCurrentScene = false;
            }
        }else{
            sibling = findNextSiblingNotSelected(proxyMapCurentScene, selectionModel);

            if(sibling.isValid()){
                QModelIndex sourceSibling = mSortFilterProxyModel->mapToSource(sibling);

                UBDocumentProxy *proxy = docModel->proxyForIndex(sourceSibling);

                if (proxy)
                {
                    selectDocument(proxy,true);

                    deleteCurrentScene = false;
                }
            }
        }
    }
    else
    {
        UBDocumentProxy* proxy = docModel->proxyForIndex(currentScene);
        selectDocument(proxy, true);
    }

    docModel->moveIndexes(list, docModel->trashIndex());

    if(deleteCurrentScene){
        createNewDocumentInUntitledFolder();
    }

    //selectionModel->clearSelection();
}
//N/C - NNE - 20140410 : END

QModelIndex UBDocumentController::findPreviousSiblingNotSelected(const QModelIndex &index, QItemSelectionModel *selectionModel)
{
    QModelIndex sibling = index.sibling(index.row() - 1, 0);

    if(sibling.isValid())
    {
        if(!parentIsSelected(sibling, selectionModel)
                && !selectionModel->isSelected(sibling))
        {
            QModelIndex model = mSortFilterProxyModel->mapToSource(sibling);

            if(UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->isCatalog(model))
            {
                return findPreviousSiblingNotSelected(sibling, selectionModel);
            }
            else
            {
                return sibling;
            }
        }
        else
        {
            return findPreviousSiblingNotSelected(sibling, selectionModel);
        }
    }else{
        //if the parent exist keep searching, else stop the search
        QModelIndex parent = index.model()->parent(index);

        if(parent.isValid())
        {
            return findPreviousSiblingNotSelected(parent, selectionModel);
        }
        else
        {
            return QModelIndex();
        }
    }
}

QModelIndex UBDocumentController::findNextSiblingNotSelected(const QModelIndex &index, QItemSelectionModel *selectionModel)
{
    QModelIndex sibling = index.sibling(index.row() + 1, 0);

    if(sibling.isValid())
    {
        if(!parentIsSelected(sibling, selectionModel)
            && !selectionModel->isSelected(sibling))
        {
            QModelIndex model = mSortFilterProxyModel->mapToSource(sibling);

            if(UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->isCatalog(model))
            {
                return findNextSiblingNotSelected(sibling, selectionModel);
            }
            else
            {
                return sibling;
            }
        }
        else
        {
            return findNextSiblingNotSelected(sibling, selectionModel);
        }
    }
    else
    {
        //if the parent exist keep searching, else stop the search
        QModelIndex parent = index.parent();

        if(parent.isValid())
        {
            return findNextSiblingNotSelected(parent, selectionModel);
        }
        else
        {
            return QModelIndex();
        }
    }
}

bool UBDocumentController::parentIsSelected(const QModelIndex& child, QItemSelectionModel *selectionModel)
{
    QModelIndex parent = child.parent();

    while(parent.isValid()){
        if(selectionModel->isSelected(parent)){
            return true;
        }

        parent = parent.parent();
    }

    return false;
}

//issue 1629 - NNE - 20131212
void UBDocumentController::moveToTrash(QModelIndex &index, UBDocumentTreeModel* docModel)
{
    QModelIndexList list;
    list.push_back(index);
    moveIndexesToTrash(list, docModel);
}
//issue 1629 - NNE - 20131212 : END

void UBDocumentController::deleteDocumentsInFolderOlderThan(const QModelIndex &index, const int days)
{
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    QModelIndexList list;
    for (int i = 0; i < docModel->rowCount(index); i++)
    {
        list << docModel->index(i, 0, index);
    }

    foreach (QModelIndex child, list)
    {
        UBDocumentProxy *documentProxy= docModel->proxyForIndex(child);

        if (documentProxy)
        {
            if (documentProxy->lastUpdate().date() < QDateTime::currentDateTime().addDays(-days).date())
            {
                UBPersistenceManager::persistenceManager()->deleteDocument(documentProxy);
            }
        }
        else
        {
            if (docModel->isCatalog(child))
            {
                deleteDocumentsInFolderOlderThan(child, days);
            }
        }
    }
}

void UBDocumentController::deleteEmptyFolders(const QModelIndex &index)
{
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    QModelIndexList list;
    for (int i = 0; i < docModel->rowCount(index); i++)
    {
        list << docModel->index(i, 0, index);
    }

    if (list.length() > 0)
    {
        foreach (QModelIndex child, list)
        {
            if (docModel->isCatalog(child))
            {
                if (!docModel->containsDocuments(child))
                {
                    deleteIndexAndAssociatedData(child);
                }
            }
        }
    }
    else
    {
        if (docModel->isCatalog(index))
        {
            deleteIndexAndAssociatedData(index);
        }
    }
}

void UBDocumentController::emptyFolder(const QModelIndex &index, DeletionType pDeletionType)
{
    // Issue NC - CFA - 20131029 : ajout d'une popup de confirmation pour la suppression definitive
//    if(pDeletionType == CompleteDelete && !UBApplication::mainWindow->yesNoQuestion(tr("Empty the trash"),tr("You're about to empty the trash.") +"\n\n" + tr("Are you sure ?")))
//        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    if (!docModel->isCatalog(index)) {
        return;
    }
    while (docModel->rowCount(index)) {
        QModelIndex subIndex = docModel->index(0, 0, index);
        switch (pDeletionType) {
        case MoveToTrash :
            docModel->moveIndex(subIndex, docModel->trashIndex());
            break;

        case CompleteDelete :
            deleteIndexAndAssociatedData(subIndex);
            break;
        default:
            break;
        }

    }

    QApplication::restoreOverrideCursor();
    // Fin issue NC - CFA - 20131029
}

void UBDocumentController::deleteIndexAndAssociatedData(const QModelIndex &pIndex)
{
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    while (docModel->rowCount(pIndex)) {
        QModelIndex subIndex = docModel->index(0, 0, pIndex);
        deleteIndexAndAssociatedData(subIndex);
    }

    //N/C - NNE - 20140408
    UBDocumentProxy *proxyData = nullptr;
    if(pIndex.column() == 0)
    {
        if (docModel->isDocument(pIndex)) {
            proxyData = docModel->proxyData(pIndex);

            if (selectedDocument() == proxyData)
            {
                setDocument(nullptr);
            }

            if (proxyData) {
                UBPersistenceManager::persistenceManager()->deleteDocument(proxyData);
            }
        }
    }

    if (proxyData)
    {
        // need to recall indexForProxy as rows could have changed when performing a multiple deletion
        QModelIndex indexForProxy = docModel->indexForProxy(proxyData);
        if (!docModel->removeRow(indexForProxy.row(), indexForProxy.parent()))
        {
            qDebug() << "could not remove row (r:" << indexForProxy.row() << "p:" << indexForProxy.parent() << ")";
        }
    }
    else
    {
        if (docModel->isCatalog(pIndex))
        {
            if (!docModel->removeRow(pIndex.row(), pIndex.parent()))
            {
                qDebug() << "could not remove row (r:" << pIndex.row() << "p:" << pIndex.parent() << ")";
            }
        }
    }
}


void UBDocumentController::exportDocument()
{
    QAction *currentExportAction = qobject_cast<QAction *>(sender());
    QVariant actionData = currentExportAction->data();
    UBExportAdaptor* selectedExportAdaptor = UBDocumentManager::documentManager()->supportedExportAdaptors()[actionData.toInt()];

    UBDocumentProxy* proxy = firstSelectedTreeProxy();

    selectedExportAdaptor->persist(proxy);
    emit exportDone();

}

void UBDocumentController::exportDocumentSet()
{

}

void UBDocumentController::documentZoomSliderValueChanged (int value)
{
    mDocumentUI->thumbnailWidget->setThumbnailWidth(value);

    UBSettings::settings()->documentThumbnailWidth->set(value);
}

void UBDocumentController::importFile()
{
    UBDocumentManager *docManager = UBDocumentManager::documentManager();

    QString defaultPath = UBSettings::settings()->lastImportFilePath->get().toString();
    if(defaultPath.isDetached())
        defaultPath = UBSettings::settings()->userDocumentDirectory();
    QString filePath = QFileDialog::getOpenFileName(mParentWidget, tr("Open Supported File"),
                                                    defaultPath, docManager->importFileFilter());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();
    QFileInfo fileInfo(filePath);

    if (fileInfo.suffix().toLower() == "ubx") {
        UBPersistenceManager::persistenceManager()->createDocumentProxiesStructure(docManager->importUbx(filePath, UBSettings::userDocumentDirectory()), true);

        emit documentThumbnailsUpdated(this); // some documents might have been overwritten while not having the same page count

    } else {
        UBSettings::settings()->lastImportFilePath->set(QVariant(fileInfo.absolutePath()));

        if (filePath.length() > 0)
        {
            UBDocumentProxy* createdDocument = 0;
            QApplication::processEvents();
            QFile selectedFile(filePath);

            UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

            QModelIndex selectedIndex = firstSelectedTreeIndex();
            QString groupName = "";
            if (selectedIndex.isValid())
            {
                groupName = docModel->isCatalog(selectedIndex)
                    ? docModel->virtualPathForIndex(selectedIndex)
                    : docModel->virtualDirForIndex(selectedIndex);
            }

            showMessage(tr("Importing file %1...").arg(fileInfo.baseName()), true);

            createdDocument = docManager->importFile(selectedFile, groupName);

            if (createdDocument) {
                selectDocument(createdDocument, true, true, true);
                pageSelectionChanged();

            } else {
                showMessage(tr("Failed to import file ... "));
            }
        }
    }

    QApplication::restoreOverrideCursor();

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
                showMessage(tr("Folder does not contain any image files"));
                UBApplication::applicationController->showDocument();
            }
            else
            {
                document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
                UBMetadataDcSubsetAdaptor::persist(document);
                reloadThumbnails();
                if (selectedDocument() == UBApplication::boardController->selectedDocument())
                    UBApplication::boardController->reloadThumbnails();

                pageSelectionChanged();
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
    }
}


bool UBDocumentController::addFileToDocument(UBDocumentProxy* document)
{
    QString defaultPath = UBSettings::settings()->lastImportFilePath->get().toString();
    QString filePath = QFileDialog::getOpenFileName(mParentWidget, tr("Open Supported File"), defaultPath, UBDocumentManager::documentManager()->importFileFilter(true));

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();

    QFileInfo fileInfo(filePath);
    UBSettings::settings()->lastImportFilePath->set(QVariant(fileInfo.absolutePath()));

    bool success = false;

    if (filePath.length() > 0)
    {
        QApplication::processEvents(); // NOTE: We performed this just a few lines before. Is it really necessary to do it again here??
        QFile selectedFile(filePath);

        showMessage(tr("Importing file %1...").arg(fileInfo.baseName()), true);

        QStringList fileNames;
        fileNames << filePath;
        success = UBDocumentManager::documentManager()->addFilesToDocument(document, fileNames);

        if (success)
        {
            document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(document);
            reloadThumbnails();
            if (selectedDocument() == UBApplication::boardController->selectedDocument())
                UBApplication::boardController->reloadThumbnails();

            pageSelectionChanged();
        }
        else
        {
            showMessage(tr("Failed to import file ... "));
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
    //mBoardController->reloadThumbnails();

    UBDocumentContainer::moveThumbPage(source, target);
    mDocumentUI->thumbnailWidget->hightlightItem(target);

    //mBoardController->setActiveDocumentScene(target);
}

void UBDocumentController::insertThumbnail(int index, const QPixmap& pix)
{
    QGraphicsPixmapItem *newThumbnail = new UBSceneThumbnailPixmap(pix, selectedDocument(), index); // deleted by the tree widget

    mDocumentUI->thumbnailWidget->insertThumbnail(index, newThumbnail);
}

void UBDocumentController::updateThumbnail(int index)
{
    auto pix = UBApplication::boardController->pageAt(index);

    mDocumentUI->thumbnailWidget->updateThumbnailPixmap(index, *pix);
}


void UBDocumentController::removeThumbnail(int index)
{
    mDocumentUI->thumbnailWidget->removeThumbnail(index);
}

void UBDocumentController::moveThumbnail(int from, int to)
{
    mDocumentUI->thumbnailWidget->moveThumbnail(from, to);
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

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    bool pageSelected = mDocumentUI->thumbnailWidget->selectedItems().count() > 0;
    bool docSelected = docModel->isDocument(firstSelectedTreeIndex());
    bool folderSelected = docModel->isCatalog(firstSelectedTreeIndex());

    if (pageSelected)
        itemSelectionChanged(Page);
    else if (docSelected)
        itemSelectionChanged(Document);
    else if (folderSelected)
        itemSelectionChanged(Folder);
    else
        itemSelectionChanged(None);
}

void UBDocumentController::documentSceneChanged(UBDocumentProxy* proxy, int pSceneIndex)
{
    Q_UNUSED(pSceneIndex);
    QModelIndexList sel = mDocumentUI->documentTreeView->selectionModel()->selectedRows(0);

    QModelIndex selection;
    if(sel.count() > 0){
        selection = sel.first();
    }

    TreeViewSelectionChanged(selection, QModelIndex());
}

void UBDocumentController::thumbnailPageDoubleClicked(QGraphicsItem* item, int index)
{
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    QModelIndex selectedIndex = firstSelectedTreeIndex();

    if (selectedIndex.isValid()) {
        if (docModel->inTrash(selectedIndex)) {
            return;
        }
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    UBSceneThumbnailPixmap* thumb = qgraphicsitem_cast<UBSceneThumbnailPixmap*> (item);

    if (thumb) {
        UBDocumentProxy* proxy = thumb->documentProxy();
        if (proxy && isOKToOpenDocument(proxy)) {
            mBoardController->setActiveDocumentScene(proxy, index);
            UBApplication::applicationController->showBoard();
        }
    }

    QApplication::restoreOverrideCursor();
}


void UBDocumentController::pageClicked(QGraphicsItem* item, int index)
{
    Q_UNUSED(item);
    Q_UNUSED(index);

    pageSelectionChanged();
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

            if (thumb &&  thumb->documentProxy())
            {
                QPair<UBDocumentProxy*, int> pageInfo(thumb->documentProxy(), thumb->sceneIndex());
                pageInfoList << pageInfo;
            }
        }

        for (int i = 0; i < pageInfoList.length(); i++)
        {
            mBoardController->addScene(pageInfoList.at(i).first, pageInfoList.at(i).second, false);
        }

        int newActiveSceneIndex = selectedItems.count() == mBoardController->selectedDocument()->pageCount() ? 0 : oldActiveSceneIndex + 1;
        mDocumentUI->thumbnailWidget->selectItemAt(newActiveSceneIndex, false);
        selectDocument(mBoardController->selectedDocument());
        mBoardController->selectedDocument()->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBMetadataDcSubsetAdaptor::persist(mBoardController->selectedDocument());
        //mBoardController->reloadThumbnails();

        UBApplication::applicationController->showBoard();

        mBoardController->setActiveDocumentScene(newActiveSceneIndex);
    }

    QApplication::restoreOverrideCursor();
}

void UBDocumentController::renameSelectedItem()
{
    if (mDocumentUI->documentTreeView->currentIndex().isValid()) {
        mDocumentUI->documentTreeView->edit(mDocumentUI->documentTreeView->currentIndex());
    }
}

bool UBDocumentController::isOKToOpenDocument(UBDocumentProxy* proxy)
{
    static UBWidgetUpgradeAdaptor widgetUpgradeAdaptor;

    //check version
    QString docVersion = proxy->metaData(UBSettings::documentVersion).toString();

    if (docVersion.isEmpty() || docVersion.startsWith("4.1") || docVersion.startsWith("4.2")
            || docVersion.startsWith("4.3") || docVersion.startsWith("4.4") || docVersion.startsWith("4.5")
            || docVersion.startsWith("4.6") || docVersion.startsWith("4.8")) // TODO UB 4.7 update if necessary
    {
        // Invoke widget upgrader
        widgetUpgradeAdaptor.upgradeWidgets(proxy);
        return true;
    }
    else
    {
        if (UBApplication::mainWindow->yesNoQuestion(tr("Open Document"),
                tr("The document '%1' has been generated with a newer version of OpenBoard (%2). By opening it, you may lose some information. Do you want to proceed?")
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
    if (mMessageWindow)
    {
        int margin = UBSettings::boardMargin;

        QRect newSize = mDocumentUI->thumbnailWidget->geometry();

        #ifdef Q_WS_MACX
            QPoint point(newSize.left() + margin, newSize.bottom() - mMessageWindow->height() - margin);
            mMessageWindow->move(mDocumentUI->thumbnailWidget->mapToGlobal(point));
        #else
            mMessageWindow->move(margin, newSize.height() - mMessageWindow->height() - margin);
        #endif

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
                = UBDocumentManager::documentManager()->addFilesToDocument(document, images);

            if (importedImageNumber == 0)
            {
                UBApplication::showMessage(tr("Selection does not contain any image files!"));
                UBApplication::applicationController->showDocument();
            }
            else
            {
                document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
                UBMetadataDcSubsetAdaptor::persist(document);
                reloadThumbnails();
                if (selectedDocument() == UBApplication::boardController->selectedDocument())
                    UBApplication::boardController->reloadThumbnails();

                pageSelectionChanged();
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
    UBDocumentTreeModel *treeModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    if (current == mDocumentUI->thumbnailWidget)
    {
        if (mDocumentUI->thumbnailWidget->selectedItems().count() > 0)
            mSelectionType = Page;
        else
            mSelectionType = None;
    }
    else if (current == mDocumentUI->documentTreeView)
    {
        if (treeModel->isDocument(firstSelectedTreeIndex()))
            mSelectionType = Document;
        else if (treeModel->isCatalog(firstSelectedTreeIndex()))
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
}

void UBDocumentController::updateActions()
{
    if (mIsClosing)
        return;

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    //N/C - NNE - 20140408
    QModelIndexList list = mDocumentUI->documentTreeView->selectionModel()->selectedRows(0);

    //if in multi selection, juste activate the actionDelete
    if(list.count() > 1){
        mMainWindow->actionNewDocument->setEnabled(false);
        mMainWindow->actionNewFolder->setEnabled(false);
        mMainWindow->actionExport->setEnabled(false);
        mMainWindow->actionDuplicate->setEnabled(false);
        mMainWindow->actionOpen->setEnabled(false);
        mMainWindow->actionRename->setEnabled(false);

        mMainWindow->actionAddToWorkingDocument->setEnabled(false);
        mMainWindow->actionDocumentAdd->setEnabled(false);
        mMainWindow->actionImport->setEnabled(false);
        mMainWindow->actionDelete->setEnabled(true);

        return;
    }
    //N/C - NNE - 20140408 : END

#ifdef Q_OS_OSX
        mMainWindow->actionDelete->setShortcut(QKeySequence(Qt::Key_Backspace));
#else
        mMainWindow->actionDelete->setShortcut(QKeySequence(Qt::Key_Delete));
#endif

    QModelIndex selectedIndex = firstSelectedTreeIndex();
    UBDocumentProxy *selectedProxy = docModel->proxyData(selectedIndex);
    int pageCount = -1;
    if (selectedProxy) {
        pageCount = selectedProxy->pageCount();
    }

    bool pageSelected = false;
    bool groupSelected = false;
    bool docSelected = false;

    if (mSelectionType == Page) {
        pageSelected = true;
    } else {
        if (docModel->isDocument(firstSelectedTreeIndex())) {
            docSelected = true;
        } else if (docModel->isCatalog(firstSelectedTreeIndex())) {
            groupSelected = true;
        }
    }

    bool trashSelected = docModel->inTrash(selectedIndex) || selectedIndex == docModel->trashIndex()  ? true : false;

    mMainWindow->actionNewDocument->setEnabled(docModel->newNodeAllowed(selectedIndex));
    mMainWindow->actionNewFolder->setEnabled(docModel->newNodeAllowed(selectedIndex));
    mMainWindow->actionExport->setEnabled((docSelected || pageSelected || groupSelected) && !trashSelected);
    updateExportSubActions(selectedIndex);

    bool firstSceneSelected = false;
    bool everyPageSelected = false;

    if (docSelected) {
        mMainWindow->actionDuplicate->setEnabled(!trashSelected);

    } else if (pageSelected) {
        QList<QGraphicsItem*> selection = mDocumentUI->thumbnailWidget->selectedItems();
        if(pageCount == 1) {
            mMainWindow->actionDuplicate->setEnabled(!trashSelected && pageCanBeDuplicated(UBDocumentContainer::pageFromSceneIndex(0)));

        } else {
            for (int i = 0; i < selection.count() && !firstSceneSelected; i += 1) {
                if (qgraphicsitem_cast<UBSceneThumbnailPixmap*>(selection.at(i))->sceneIndex() == 0) {
                    mMainWindow->actionDuplicate->setEnabled(!trashSelected && pageCanBeDuplicated(UBDocumentContainer::pageFromSceneIndex(0)));
                    firstSceneSelected = true;
                    break;
                }
            }
            if (!firstSceneSelected) {
                mMainWindow->actionDuplicate->setEnabled(!trashSelected);
            }
        }

    } else {
        mMainWindow->actionDuplicate->setEnabled(false);
    }

    mMainWindow->actionOpen->setEnabled((docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionRename->setEnabled(docModel->isOkToRename(selectedIndex));

    mMainWindow->actionAddToWorkingDocument->setEnabled(pageSelected
            && !(selectedProxy == mBoardController->selectedDocument()) && !trashSelected);

    DeletionType deletionForSelection = deletionTypeForSelection(mSelectionType, selectedIndex, docModel);
    mMainWindow->actionDelete->setEnabled(deletionForSelection != NoDeletion);

    switch (static_cast<int>(deletionForSelection)) {
    case MoveToTrash :
        if (mSelectionType == Folder)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-folder.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        else if (mSelectionType == Document)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-document.png"));
            mMainWindow->actionDelete->setText(tr("Trash"));
        }
        else if (mSelectionType == Page)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-document-page.png"));
            mMainWindow->actionDelete->setText(tr("Trash"));
        }
        else
        {//can happen ?
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash.png"));
            mMainWindow->actionDelete->setText(tr("Trash"));
        }
        break;
    case DeletePage :
        mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-document-page.png"));
        mMainWindow->actionDelete->setText(tr("Trash"));
        break;
    case CompleteDelete :
        if (mSelectionType == Folder)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-delete-folder.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        else
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-delete-document.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        break;
    case EmptyFolder :
        if (firstSelectedTreeIndex() == docModel->myDocumentsIndex())
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-my-documents.png"));
            mMainWindow->actionDelete->setText(tr("Empty"));
        }
        else
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-folder.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        break;
    case EmptyTrash :
        mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-empty.png"));
        mMainWindow->actionDelete->setText(tr("Empty"));
        break;
    case NoDeletion :
    default:
        if (mSelectionType == Folder)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-delete-folder.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        else if (mSelectionType == Document)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-delete-document.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        else if (mSelectionType == Page)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash-document-page.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        break;
    }

    mMainWindow->actionDocumentAdd->setEnabled((docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionImport->setEnabled(!trashSelected);

}

void UBDocumentController::updateExportSubActions(const QModelIndex &selectedIndex)
{
    UBDocumentManager *documentManager = UBDocumentManager::documentManager();
    for (int i = 0; i < documentManager->supportedExportAdaptors().length(); i++)
    {
        UBExportAdaptor* adaptor = documentManager->supportedExportAdaptors()[i];
        if (adaptor->associatedAction()) {
            adaptor->associatedAction()->setEnabled(adaptor->associatedActionactionAvailableFor(selectedIndex));
        }
    }
}

void UBDocumentController::currentIndexMoved(const QModelIndex &newIndex, const QModelIndex &PreviousIndex)
{
    Q_UNUSED(newIndex);
    Q_UNUSED(PreviousIndex);

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    UBDocumentProxy *newProxy = docModel->proxyData(newIndex);
    if (newProxy) {
        UBDocumentProxy *cp = new UBDocumentProxy(*newProxy); // we cannot use newProxy because it will be destroyed later
        pureSetDocument(cp);
        mBoardController->pureSetDocument(cp);
        mBoardController->pureSetDocument(newProxy);
    }
    mCurrentIndexMoved = true;
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
                proxy = thumb->documentProxy();
                if (proxy)
                {
                    sceneIndexes.append(thumb->sceneIndex());
                }

            }
        }
        UBDocumentContainer::deletePages(sceneIndexes);
        if (mBoardController->selectedDocument() == selectedDocument())
        {
            std::sort(sceneIndexes.begin(), sceneIndexes.end(), std::greater<>());
            for (auto index : sceneIndexes)
                emit mBoardController->removeThumbnailRequired(index);
        }

        proxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBMetadataDcSubsetAdaptor::persist(proxy);

        int minIndex = proxy->pageCount() - 1;
        foreach (int i, sceneIndexes)
             minIndex = qMin(i, minIndex);

        if (mBoardController->activeSceneIndex() > minIndex)
        {
            mBoardController->setActiveSceneIndex(minIndex);
        }

        mDocumentUI->thumbnailWidget->selectItemAt(minIndex);

        mBoardController->setActiveDocumentScene(minIndex);
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
    return page >= 1;
}

bool UBDocumentController::pageCanBeMovedDown(int page)
{
    return page < selectedDocument()->pageCount() - 1;
}

bool UBDocumentController::pageCanBeDuplicated(int page)
{
    return page != 0;
}

bool UBDocumentController::pageCanBeDeleted(int page)
{
    return page != 0;
}

void UBDocumentController::setDocument(UBDocumentProxy *document, bool forceReload)
{
    UBDocumentContainer::setDocument(document, forceReload);
}

QModelIndex UBDocumentController::firstSelectedTreeIndex()
{
    return selectedTreeIndexes().count() ? selectedTreeIndexes().first() : QModelIndex();
}

UBDocumentController::DeletionType
UBDocumentController::deletionTypeForSelection(LastSelectedElementType pTypeSelection
                                               , const QModelIndex &selectedIndex
                                               , UBDocumentTreeModel *docModel) const
{

    if (pTypeSelection == Page)
    {
        if (everySceneSelected())
        {
            return NoDeletion;
        }
        if (!firstAndOnlySceneSelected()) {
            return DeletePage;
        }
    } else if (docModel->isConstant(selectedIndex)) {
        if (selectedIndex == docModel->trashIndex()) {
            if (docModel->rowCount(selectedIndex) > 0)
                return EmptyTrash;
            else
                return NoDeletion;
        }

        if (selectedIndex.isValid())
            return EmptyFolder;
        else
            return NoDeletion;
    } else if (pTypeSelection != None) {
        if (docModel->inTrash(selectedIndex)) {
            return CompleteDelete;
        } else {
            return MoveToTrash;
        }
    }

    return NoDeletion;
}

bool UBDocumentController::everySceneSelected() const
{
    QList<QGraphicsItem*> selection = mDocumentUI->thumbnailWidget->selectedItems();
    if (selection.count() > 0)
    {
        UBSceneThumbnailPixmap* p = dynamic_cast<UBSceneThumbnailPixmap*>(selection.at(0));
        if (p)
        {
            return (selection.count() == p->documentProxy()->pageCount());
        }
    }
    return false;
}

bool UBDocumentController::firstAndOnlySceneSelected() const
{
    QList<QGraphicsItem*> selection = mDocumentUI->thumbnailWidget->selectedItems();
    for(int i = 0; i < selection.count(); i += 1)
    {
        UBSceneThumbnailPixmap* p = dynamic_cast<UBSceneThumbnailPixmap*>(selection.at(i));
        if (p)
        {
            int pageCount = p->documentProxy()->pageCount();
            if (pageCount > 1) //not the only scene
            {
                return false;
            }
            else
            {
                if (p->sceneIndex() == 0)
                {
                    return true; //the first and only scene
                }
            }
        }
    }

    return false;
}

void UBDocumentController:: refreshDocumentThumbnailsView(UBDocumentContainer* source)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    UBDocumentProxy *currentDocumentProxy = selectedDocument();

    QModelIndex current = docModel->indexForProxy(currentDocumentProxy);

    if (!current.isValid())
    {
        mDocumentUI->thumbnailWidget->setGraphicsItems(QList<QGraphicsItem*>()
                                                       , QList<QUrl>()
                                                       , QStringList()
                                                       , UBApplication::mimeTypeUniboardPage);

        QApplication::restoreOverrideCursor();
        return;
    }

    if (currentDocumentProxy)
    {
        UBThumbnailAdaptor::load(currentDocumentProxy, documentThumbs());
    }

    QList<QGraphicsItem*> items;
    QList<QUrl> itemsPath;

    QGraphicsPixmapItem *selection = 0;

    QStringList labels;

    if (currentDocumentProxy)
    {
        for (int i = 0; i < currentDocumentProxy->pageCount(); i++)
        {
            UBApplication::showMessage(tr("Refreshing Document Thumbnails View (%1/%2)").arg(i+1).arg(source->selectedDocument()->pageCount()));

            auto pix = documentThumbs().at(i);
            QGraphicsPixmapItem *pixmapItem = new UBSceneThumbnailPixmap(*pix, currentDocumentProxy, i); // deleted by the tree widget

            if (currentDocumentProxy == mBoardController->selectedDocument() && mBoardController->activeSceneIndex() == i)
            {
                selection = pixmapItem;
            }

            items << pixmapItem;
            int pageIndex = pageFromSceneIndex(i);
            if(pageIndex)
                labels << tr("Page %1").arg(pageIndex);
            else
                labels << tr("Title page");

            itemsPath.append(QUrl::fromLocalFile(currentDocumentProxy->persistencePath() + QString("/pages/%1").arg(UBDocumentContainer::pageFromSceneIndex(i))));
        }
    }

    mDocumentUI->thumbnailWidget->setGraphicsItems(items, itemsPath, labels, UBApplication::mimeTypeUniboardPage);

    if (docModel->inTrash(current)) {
        mDocumentUI->thumbnailWidget->setDragEnabled(false);
    } else {
        mDocumentUI->thumbnailWidget->setDragEnabled(true);
    }

    mDocumentUI->thumbnailWidget->ensureVisible(0, 0, 10, 10);

    if (selection)
    {
        UBSceneThumbnailPixmap *currentSceneThumbnailPixmap = dynamic_cast<UBSceneThumbnailPixmap*>(selection);
        if (currentSceneThumbnailPixmap)
            mDocumentUI->thumbnailWidget->hightlightItem(currentSceneThumbnailPixmap->sceneIndex());
    }

    QApplication::restoreOverrideCursor();
}

void UBDocumentController::createNewDocumentInUntitledFolder()
{
    UBPersistenceManager *pManager = UBPersistenceManager::persistenceManager();
    UBDocumentTreeModel *docModel = pManager->mDocumentTreeStructureModel;

    QString groupName = docModel->virtualPathForIndex(docModel->untitledDocumentsIndex());

    UBDocumentProxy *document = pManager->createDocument(groupName);
    selectDocument(document);

    if (document)
        pManager->mDocumentTreeStructureModel->markDocumentAsNew(document);

    pageSelectionChanged();
}

void UBDocumentController::collapseAll()
{
    //disable the animations because the view port will be in a invalid state
    mDocumentUI->documentTreeView->setAnimated(false);

    mDocumentUI->documentTreeView->collapseAll();

    QPersistentModelIndex untiltedDocumentIndex = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->untitledDocumentsIndex();
    QPersistentModelIndex myDocumentIndex = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->myDocumentsIndex();

    UBSortFilterProxyModel *proxy = dynamic_cast<UBSortFilterProxyModel*>(mDocumentUI->documentTreeView->model());

    if(proxy){
        mDocumentUI->documentTreeView->setExpanded(proxy->mapFromSource(myDocumentIndex), true);
        mDocumentUI->documentTreeView->setExpanded(proxy->mapFromSource(untiltedDocumentIndex), true);
    }else{
        mDocumentUI->documentTreeView->setExpanded(myDocumentIndex, true);
        mDocumentUI->documentTreeView->setExpanded(untiltedDocumentIndex, true);
    }

    mDocumentUI->documentTreeView->setAnimated(true);
}

//N/C - NNE - 20140513
void UBDocumentController::expandAll()
{
    //disable the animations because the view port will be in a invalid state
    mDocumentUI->documentTreeView->setAnimated(false);

    mDocumentUI->documentTreeView->expandAll();

    mDocumentUI->documentTreeView->setAnimated(true);
}

void UBDocumentController::clearThumbnailsSelection()
{
    mDocumentUI->thumbnailWidget->clearSelection();
    pageSelectionChanged();
}
