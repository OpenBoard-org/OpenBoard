/*
 * UBImportAdaptor.cpp
 *
 *  Created on: Feb 11, 2009
 *      Author: julienbachmann
 */

#include "UBImportAdaptor.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "document/UBDocumentProxy.h"

#include "core/memcheck.h"

UBImportAdaptor::UBImportAdaptor(QObject *parent)
    :QObject(parent)
{
    // NOOP
}

UBImportAdaptor::~UBImportAdaptor()
{
    // NOOP
}

UBDocumentProxy* UBImportAdaptor::importFile(const QFile& pFile, const QString& pGroup)
{
    QString documentName = QFileInfo(pFile.fileName()).baseName();

    UBDocumentProxy* newDocument = UBPersistenceManager::persistenceManager()->createDocument(pGroup, documentName);

    bool result = addFileToDocument(newDocument, pFile);

    if (result)
    {
        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(newDocument);
    }
    else
    {
        UBPersistenceManager::persistenceManager()->deleteDocument(newDocument);
        newDocument = 0;
    }

    return newDocument;
}
