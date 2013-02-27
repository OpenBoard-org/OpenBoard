/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBExportDocument.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBDocumentManager.h"
#include "core/UBApplication.h"

#include "document/UBDocumentProxy.h"

#include "globals/UBGlobals.h"

THIRD_PARTY_WARNINGS_DISABLE
#include "quazip.h"
#include "quazipfile.h"
THIRD_PARTY_WARNINGS_ENABLE

#include "transition/UniboardSankoreTransition.h"

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
    if (!pDocumentProxy)
        return;

    QString filename = askForFileName(pDocumentProxy, tr("Export as UBZ File"));

    if (filename.length() > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        if (mIsVerbose)
            UBApplication::showMessage(tr("Exporting document..."));

        persistsDocument(pDocumentProxy, filename);

        if (mIsVerbose)
            UBApplication::showMessage(tr("Export successful."));

        QApplication::restoreOverrideCursor();
    }
}


void UBExportDocument::persistsDocument(UBDocumentProxy* pDocumentProxy, QString filename)
{
    UniboardSankoreTransition document;
    QString documentPath(pDocumentProxy->persistencePath());
    document.checkDocumentDirectory(documentPath);

    QuaZip zip(filename);
    zip.setFileNameCodec("UTF-8");
    if(!zip.open(QuaZip::mdCreate))
    {
        qWarning("Export failed. Cause: zip.open(): %d", zip.getZipError());
        return;
    }

    QDir documentDir = QDir(pDocumentProxy->persistencePath());

    QuaZipFile outFile(&zip);
    UBFileSystemUtils::compressDirInZip(documentDir, "", &outFile, true, this);

    if(zip.getZipError() != 0)
    {
        qWarning("Export failed. Cause: zip.close(): %d", zip.getZipError());
    }

    zip.close();

    UBPlatformUtils::setFileType(filename, 0x5542647A /* UBdz */);

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
    return tr("Export to Sankore Format");
}
