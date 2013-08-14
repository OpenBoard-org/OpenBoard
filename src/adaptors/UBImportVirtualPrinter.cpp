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




#include "UBImportVirtualPrinter.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBDocumentManager.h"

#include "document/UBDocumentProxy.h"

#include "UBIniFileParser.h"

#include <windows.h>

#include "core/memcheck.h"

QString UBImportVirtualPrinter::sOriginalDefaultPrintername = "";

QPointer<UBDocumentProxy> UBImportVirtualPrinter::pendingDocument;


UBImportVirtualPrinter::UBImportVirtualPrinter(QObject* parent)
    : UBImportAdaptor(parent)
{
    // NOOP
}


UBImportVirtualPrinter::~UBImportVirtualPrinter()
{
    // NOOP
}


QStringList UBImportVirtualPrinter::supportedExtentions()
{
    return QStringList("ini");
}


QString UBImportVirtualPrinter::importFileFilter()
{
    // we don't want user import manually ini file.
    return "";
}


QString UBImportVirtualPrinter::pdfFileName(const QFile& pFile)
{

    UBIniFileParser iniParser(pFile.fileName());

    // check if we have 1 PDF file to import
    QString totalPdfString = iniParser.getStringValue("PDF", "Count");
    bool ok;
    int totalPdf = totalPdfString.toInt(&ok);
    QString pdfFilename;

    if (ok && totalPdf > 0)
    {
        pdfFilename = iniParser.getStringValue("PDF", "File0");
        if (pdfFilename == "")
        {
            qWarning() << "Unable to retreive PDF file name from ini file " << pFile.fileName();
        }
    }

    return pdfFilename;
}


QStringList UBImportVirtualPrinter::emfFileNames(const QFile& pFile)
{

    UBIniFileParser iniParser(pFile.fileName());

    // check if we have 1 PDF file to import
    QString totalEmfString = iniParser.getStringValue("EMF", "Count");
    bool ok;
    int totalEmf = totalEmfString.toInt(&ok);
    QStringList emfFilenames;

    if (ok)
    {
        for(int i = 0; i < totalEmf; i++)
        {
            emfFilenames << iniParser.getStringValue("EMF", QString("File%1").arg(i));
        }
    }

    return emfFilenames;
}



void UBImportVirtualPrinter::cleanUp(const QFile& pFile, const QString& pPdfFileName, QStringList pEmfFilenames)
{

    //restore default printer if needed
    if (UBImportVirtualPrinter::sOriginalDefaultPrintername != "")
    {
        LPTSTR wDefaultPrinterName = new TCHAR[255];
        int i = UBImportVirtualPrinter::sOriginalDefaultPrintername.toWCharArray(wDefaultPrinterName);
        wDefaultPrinterName[i] = 0;
        SetDefaultPrinter(wDefaultPrinterName);
        UBImportVirtualPrinter::sOriginalDefaultPrintername = "";
        delete[] wDefaultPrinterName;
    }

    // delete tmp ini file
    if (!QFile::remove(pFile.fileName()))
    {
        qWarning() << "Unable to remove file " << pFile.fileName();
    }
    // delete tmp PDF file
    if (pPdfFileName != "" && !QFile::remove(pPdfFileName))
    {
        qWarning() << "Unable to remove file " << pPdfFileName;
    }

    foreach(QString f, pEmfFilenames)
    {
        QFile::remove(f);
    }
}


UBDocumentProxy* UBImportVirtualPrinter::importFile(const QFile& pFile, const QString& pGroup)
{
    UBApplication::showMessage(tr("Importing Sankore printer file ..."));
    UBDocumentProxy *document = 0;

    QString pdfFilename = pdfFileName(pFile);

    if (pdfFilename.size() > 0)
    {
        QFile pdfFile(pdfFilename);

                document = UBDocumentManager::documentManager()->importFile(pdfFile, pGroup);

                UBIniFileParser iniParser(pFile.fileName());

        QString documentName = iniParser.getStringValue("Document", "Name");

        // we must update the document name otherwise the imported document will have the name of the tmp PDF file.
        if (document)
        {
            if (documentName != "")
                document->setMetaData(UBSettings::documentName, documentName);

            UBPersistenceManager::persistenceManager()->persistDocumentMetadata(document);
        }
    }

    if (!document)
    {
        UBApplication::showMessage(tr("Error while importing Sankore printer file."));
    }

    cleanUp(pFile, pdfFilename, emfFileNames(pFile));

    return document;
}


bool UBImportVirtualPrinter::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
    UBApplication::showMessage(tr("Importing Sankore printer file ..."));
    bool result = false;

    QString pdfFilename = pdfFileName(pFile);

    if (pdfFilename.size() > 0)
    {
        QFile pdfFile(pdfFilename);
        result = UBDocumentManager::documentManager()->addFileToDocument(pDocument, pdfFile);
    }

    if (!result)
    {
        UBApplication::showMessage(tr("Error while importing Sankore printer file."));
    }

    cleanUp(pFile, pdfFilename, emfFileNames(pFile));

    return result;
}

