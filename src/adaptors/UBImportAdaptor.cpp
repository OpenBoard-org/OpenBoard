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
