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


#include "UBExportDocumentSetAdaptor.h"
#include "UBExportDocument.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBDocumentManager.h"
#include "core/UBApplication.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "globals/UBGlobals.h"
#include "core/UBPersistenceManager.h"
#include "core/UBForeignObjectsHandler.h"

#ifdef Q_OS_OSX
    #include <quazip.h>
    #include <quazipfile.h>
#else
    #include "quazip.h"
    #include "quazipfile.h"
#endif

#include "core/memcheck.h"

UBExportDocumentSetAdaptor::UBExportDocumentSetAdaptor(QObject *parent)
    : UBExportAdaptor(parent)
{

}

UBExportDocumentSetAdaptor::~UBExportDocumentSetAdaptor()
{
    // NOOP
}

void UBExportDocumentSetAdaptor::persist(UBDocumentProxy* pDocumentProxy)
{
    QModelIndex treeViewParentIndex;
    UBPersistenceManager *persistenceManager = UBPersistenceManager::persistenceManager();
    UBDocumentTreeModel *treeModel = persistenceManager->mDocumentTreeStructureModel;
    QString filename;

    if (pDocumentProxy) {
        treeViewParentIndex = treeModel->indexForProxy(pDocumentProxy);
        if (!treeViewParentIndex.isValid()) {
            qDebug() << "failed to export";
            UBApplication::showMessage(tr("Failed to export..."));
            return;
        }
        filename = askForFileName(pDocumentProxy, tr("Export as UBX File"));

    } else {
        treeViewParentIndex = UBApplication::documentController->firstSelectedTreeIndex();
        if (!treeViewParentIndex.isValid()) {
            qDebug() << "failed to export";
            UBApplication::showMessage(tr("Failed to export..."));
            return;
        }

        UBDocumentTreeNode* node = treeModel->nodeFromIndex(treeViewParentIndex);
        UBDocumentProxy proxy;
        proxy.setMetaData(UBSettings::documentName,node->displayName());
        filename = askForFileName(&proxy, tr("Export as UBX File"));
    }

    if (filename.length() > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        if (mIsVerbose)
            UBApplication::showMessage(tr("Exporting document..."));

         if (persistData(treeViewParentIndex, filename)) {
             if (mIsVerbose) {
                 UBApplication::showMessage(tr("Export successful."));
             }
         } else {
             if (mIsVerbose) {
                 UBApplication::showMessage(tr("Export failed."));
             }
         }

         QApplication::restoreOverrideCursor();
    }
}

bool UBExportDocumentSetAdaptor::persistData(const QModelIndex &pRootIndex, QString filename)
{
    UBPersistenceManager *persistenceManager = UBPersistenceManager::persistenceManager();
    UBDocumentTreeModel *treeModel = persistenceManager->mDocumentTreeStructureModel;

    QModelIndex index = pRootIndex;

    if (!index.isValid()) {
        return false;
    }

    QuaZip zip(filename);
    zip.setFileNameCodec("UTF-8");

    if(!zip.open(QuaZip::mdCreate))
    {
        qWarning("Export failed. Cause: zip.open(): %d", zip.getZipError());
        return false;
    }

    if (!addDocumentToZip(pRootIndex, treeModel, zip)) {
        zip.close();
        return false;
    }

    zip.close();
    UBPlatformUtils::setFileType(filename, 0x5542647A /* UBdz */);

    return true;
}

QString UBExportDocumentSetAdaptor::exportExtention()
{
    return QString(".ubx");
}

QString UBExportDocumentSetAdaptor::exportName()
{
    return tr("Export to OpenBoard UBX Format");
}

bool UBExportDocumentSetAdaptor::addDocumentToZip(const QModelIndex &pIndex, UBDocumentTreeModel *model, QuaZip &zip)
{
    static int i = 0;
    i++;

    QModelIndex parentIndex = pIndex;
    if (!parentIndex.isValid()) {
        return false;
    }

    UBDocumentProxy *pDocumentProxy = model->proxyForIndex(parentIndex);
    if (pDocumentProxy) {

//        Q_ASSERT(QFileInfo(pDocumentProxy->persistencePath()).exists());
//        UBForeighnObjectsHandler cleaner;
//        cleaner.cure(pDocumentProxy->persistencePath());

        //UniboardSankoreTransition document;
        QString documentPath(pDocumentProxy->persistencePath());
        //document.checkDocumentDirectory(documentPath);

        QDir documentDir = QDir(pDocumentProxy->persistencePath());
        QuaZipFile zipFile(&zip);
        UBFileSystemUtils::compressDirInZip(documentDir, QFileInfo(documentPath).fileName() + "/", &zipFile, false);

        if(zip.getZipError() != 0)
        {
            qWarning("Export failed. Cause: zip.close(): %d", zip.getZipError());
        }
    }

    for (int i = 0; i < model->rowCount(parentIndex); ++i) {
        QModelIndex curIndex = model->index(i, 0, parentIndex);
        if (!addDocumentToZip(curIndex, model, zip)) {
            return false;
        }
    }

    return true;
}

bool UBExportDocumentSetAdaptor::associatedActionactionAvailableFor(const QModelIndex &selectedIndex)
{
    const UBDocumentTreeModel *docModel = qobject_cast<const UBDocumentTreeModel*>(selectedIndex.model());
    if (!selectedIndex.isValid() || docModel->isDocument(selectedIndex)) {
        return false;
    }

    return true;
}

