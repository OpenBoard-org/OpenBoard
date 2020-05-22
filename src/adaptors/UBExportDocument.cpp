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




#include "UBExportDocument.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBDocumentManager.h"
#include "core/UBApplication.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "globals/UBGlobals.h"

#ifdef Q_OS_OSX
    #include <quazip.h>
    #include <quazipfile.h>
#else
    #include "quazip.h"
    #include "quazipfile.h"
#endif

#include "core/memcheck.h"

UBExportDocument::UBExportDocument(QObject *parent)
    : UBExportAdaptor(parent)
{
        UBExportDocument::tr("Page"); // dummy slot for translation
}

UBExportDocument::~UBExportDocument()
{
    // NOOP
}

void UBExportDocument::persist(UBDocumentProxy* pDocumentProxy)
{
    persistLocally(pDocumentProxy, tr("Export as UBZ File"));
}


bool UBExportDocument::persistsDocument(UBDocumentProxy* pDocumentProxy, const QString &filename)
{
    QuaZip zip(filename);
    zip.setFileNameCodec("UTF-8");
    if(!zip.open(QuaZip::mdCreate))
    {
        qWarning("Export failed. Cause: zip.open(): %d", zip.getZipError());
        return false;
    }

    QDir documentDir = QDir(pDocumentProxy->persistencePath());

    QuaZipFile outFile(&zip);
    UBFileSystemUtils::compressDirInZip(documentDir, "", &outFile, true, this);

    zip.close();

    if(zip.getZipError() != 0)
    {
        qWarning("Export failed. Cause: zip.close(): %d", zip.getZipError());
        return false;
    }


    UBPlatformUtils::setFileType(filename, 0x5542647A /* UBdz */);

    return true;
}


void UBExportDocument::processing(const QString& pObjectName, int pCurrent, int pTotal)
{
    QString localized = UBExportDocument::trUtf8(pObjectName.toUtf8());

    if (mIsVerbose)
        UBApplication::showMessage(tr("Exporting %1 %2 of %3").arg(localized).arg(pCurrent).arg(pTotal));
}



QString UBExportDocument::exportExtention()
{
    return QString(".ubz");
}

QString UBExportDocument::exportName()
{
    return tr("Export to OpenBoard Format");
}

bool UBExportDocument::associatedActionactionAvailableFor(const QModelIndex &selectedIndex)
{
    const UBDocumentTreeModel *docModel = qobject_cast<const UBDocumentTreeModel*>(selectedIndex.model());
    if (!selectedIndex.isValid() || docModel->isCatalog(selectedIndex)) {
        return false;
    }

    return true;
}
