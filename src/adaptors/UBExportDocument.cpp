/*
 * UBExportDocument.cpp
 *
 *  Created on: Feb 10, 2009
 *      Author: julienbachmann
 */

#include "UBExportDocument.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBDocumentManager.h"
#include "core/UBApplication.h"

#include "document/UBDocumentProxy.h"

#include "quazip.h"
#include "quazipfile.h"

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
