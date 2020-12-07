/*
 * Copyright (C) 2015-2020 Département de l'Instruction Publique (DIP-SEM)
 *
 */

#include <QFile>
#include <QDebug>
#include <QProcess>

#include "globals/UBGlobals.h"

#include "UBExportDocumentCleaner.h"
#include "document/UBDocumentProxy.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "frameworks/UBPlatformUtils.h"

#ifdef USE_XPDF
    THIRD_PARTY_WARNINGS_DISABLE
    #include <xpdf/Object.h>
    #include <xpdf/GlobalParams.h>
    #include <xpdf/SplashOutputDev.h>
    #include <xpdf/PDFDoc.h>
    THIRD_PARTY_WARNINGS_ENABLE
#else
    #include <poppler/Object.h>
    #include <poppler/GlobalParams.h>
    #include <poppler/SplashOutputDev.h>
    #include <poppler/PDFDoc.h>
#endif

UBExportDocumentCleaner::UBExportDocumentCleaner()
{

}

bool UBExportDocumentCleaner::StripeDocument(UBDocumentProxy* pDocumentProxy,
                                             const QString &filename)
{
    //qDebug() << "UBExportDocumentCleaner::StripeDocument pDocumentProxy=" << pDocumentProxy->objectName() << "filename=" << filename;
    QMap<QString, QVariant> const metaDatas = pDocumentProxy->metaDatas();
    //qDebug() << "UBExportDocumentCleaner::StripeDocument metaDatas=" << metaDatas;

    // Find every PDF in this dir. For every PDF, build a list of pages to export.
    QMap<QString, QList<int>> pdfPages;
    QDirIterator it(filename, QStringList() << "*.svg", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString svgFile = it.next();
        QFile file(svgFile);
        file.open(QFile::ReadOnly);
        QXmlStreamReader xmlReader(&file);

        // Xml decoding as done by 'UBSvgSubsetAdaptor::UBSvgSubsetReader::loadScene'.
        while (!xmlReader.atEnd())
        {
            xmlReader.readNext();
            if (xmlReader.isStartElement())
            {
                if (xmlReader.name() == "foreignObject")
                {
                    QString href = xmlReader.attributes().value(UBSvgSubsetAdaptor::nsXLink, "href").toString();
                    if (href.contains(".pdf"))
                    {
                        QString href = xmlReader.attributes().value(UBSvgSubsetAdaptor::nsXLink, "href").toString();
                        QStringList parts = href.split("#page=");
                        if (parts.count() != 2)
                        {
                            qWarning() << "invalid pdf href value" << href;
                        } else {
                            QString pdfPath = parts[0];
                            QUuid uuid(QFileInfo(pdfPath).baseName());
                            int pageNumber = parts[1].toInt();
                            QString finalPath = QString("%1/%2").arg(filename).arg(pdfPath);
                            QList<int> &pages = pdfPages[finalPath];

                            // Don't insert twice the same page. We don't know if this a problem for the tool, but
                            // filtering is easy.
                            if (pages.indexOf(pageNumber) == -1)
                            {
                                pages.push_back(pageNumber);
                            }
                        }
                    }
                }
            }
        }
    }

    bool stripeSuccess = true;
    for (QMap<QString, QList<int>>::iterator i = pdfPages.begin(); i != pdfPages.end(); i++)
    {
        stripeSuccess &= StripePdf(i.key(), i.value());
    }
    return stripeSuccess;
}

bool UBExportDocumentCleaner::StripePdf(QString const &originalFile, QList<int> const &pagesToKeep)
{
    //qDebug() << "UBExportDocumentCleaner::StripePdf originalFile=" << originalFile << "pagesToKeep=" << pagesToKeep;
    Q_ASSERT(pagesToKeep.size() != 0);

    int totalPages = 0;
    {
        // Scope, necessary because the pdf file remains opens as long as the PDFDoc exist.
#ifdef USE_XPDF
        PDFDoc pdfDocument(new GString(originalFile.toLocal8Bit()), 0, 0, 0); // the filename GString is deleted on PDFDoc desctruction
#else
        PDFDoc pdfDocument(new GooString(originalFile.toLocal8Bit()), 0, 0, 0); // the filename GString is deleted on PDFDoc desctruction
#endif

        totalPages = pdfDocument.getNumPages();
    }

    if (totalPages <= 0)
        return false;

    Q_ASSERT(pagesToKeep.size() <= totalPages);

    if (pagesToKeep.size() == totalPages)
    {
        // If the totality of the file has to be kept, then we have nothing to stripe.
        Q_ASSERT(pagesToKeep[pagesToKeep.size()-1] == totalPages); // If all pages are there, the last one nbr is also the total.
        return true;
    }

    QString const tempName = originalFile+"_temp";
    QFile::rename(originalFile, tempName);

    QString const relaseEmptyFileName = UBPlatformUtils::applicationResourcesDirectory() + "/etc/empty.pdf";
    QString pdfEmptyFileName = relaseEmptyFileName;
    if (!QFile::exists(pdfEmptyFileName))
    {
        // No empty file in the expected place? Try a dev one.
        pdfEmptyFileName = "../../../../../OpenBoard/resources/etc/empty.pdf";
    }

    if (!QFile::exists(pdfEmptyFileName))
    {
        qWarning() << "The file '" << relaseEmptyFileName << "' was not found. Therefore, the following qpdf stripe operation is likely to fail.";
    }

    // Build a string list with all required pages.
    QString commandLinePagesString;
    for (int i = 1; i <= totalPages; i++)
    {
        if (commandLinePagesString.size() > 0)
            commandLinePagesString += " ";

        if (pagesToKeep.indexOf(i) == -1)
        {
            // The empty.pdf has only 1 page. We need to insert it as many times as required.
            commandLinePagesString += QString("\"%1\" 1").arg(pdfEmptyFileName);
        } else {
            commandLinePagesString += QString("\"%1\" %2").arg(tempName).arg(QString::number(i));
        }
    }

    // Try the 'release' config first. The binary is either side by side of OB, or in the system path.
    QString command = QString("\"%1\" --empty --pages %2 -- \"%3\"").arg(QPDF_BINARY).arg(commandLinePagesString).arg(originalFile);
    int result = QProcess::execute(command);
    if (result != 0)
    {
        // 'qpdf' not on the path? Then try for a debug configuration, so it ease debugging.
        command = QString("\"%1/%2\" --empty --pages %3 -- \"%4\"").arg(QPDF_DEBUG_BIN_DIR).arg(QPDF_BINARY).arg(commandLinePagesString).arg(originalFile);
        result = QProcess::execute(command);
        //qDebug() << command << "result=" << result;
    }

    if (result != 0) {
        // Can't stripe? Recover the original file.
        QFile::remove(originalFile);
        QFile::rename(tempName, originalFile);
    }

    QFile::remove(tempName);

    //qDebug() << "UBExportDocumentCleaner::StripePdf result=" << result;
    return (result == 0);
}
