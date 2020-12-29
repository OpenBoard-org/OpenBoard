/*
 * Copyright (C) 2015-2020 Département de l'Instruction Publique (DIP-SEM)
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

#include <QFile>
#include <QDebug>
#include <QProcess>

#include "globals/UBGlobals.h"

#include "UBExportDocumentCleaner.h"
#include "UBExportDocumentCleanerQPDF.h"
#include "document/UBDocumentProxy.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "frameworks/UBPlatformUtils.h"

UBExportDocumentCleaner::UBExportDocumentCleaner()
{
}

UBExportDocumentCleaner::~UBExportDocumentCleaner()
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

    int result = -1;
    try {
        result = UBExportDocumentCleanerQPDF::Stripe(tempName /* input */, originalFile /* output */, pagesToKeep, pdfEmptyFileName);
    } catch (std::exception &e)
    {
        qWarning() << "qpdf_main returned " << e.what();
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
