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

#include "UBMetadataDcSubsetAdaptor.h"

#include <QtGui>
#include <QtXml>

#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#include "document/UBDocumentProxy.h"

#include "core/memcheck.h"

const QString UBMetadataDcSubsetAdaptor::nsRdf = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
const QString UBMetadataDcSubsetAdaptor::nsDc = "http://purl.org/dc/elements/1.1/";
const QString UBMetadataDcSubsetAdaptor::metadataFilename = "metadata.rdf";


UBMetadataDcSubsetAdaptor::UBMetadataDcSubsetAdaptor()
{
    /*
     *
     * sample dublin core metadata
     *
     *
     *
        <?xml version="1.0"?>

        <rdf:RDF
        xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
        xmlns:dc= "http://purl.org/dc/elements/1.1/">

        <rdf:Description rdf:about="http://www.w3schools.com">
          <dc:title>D-Lib Program</dc:title>
          <dc:description>W3Schools - Free tutorials</dc:description>
          <dc:publisher>Refsnes Data as</dc:publisher>
          <dc:date>1999-09-01</dc:date>
          <dc:type>Web Development</dc:type>
          <dc:format>text/html</dc:format>
          <dc:language>en</dc:language>
        </rdf:Description>

        </rdf:RDF>
    */
}


UBMetadataDcSubsetAdaptor::~UBMetadataDcSubsetAdaptor()
{
    // NOOP
}


void UBMetadataDcSubsetAdaptor::persist(UBDocumentProxy* proxy)
{
    if(!QDir(proxy->persistencePath()).exists()){
        //In this case the a document is an empty document so we do not persist it
        return;
    }
    QString fileName = proxy->persistencePath() + "/" + metadataFilename;
    qWarning() << fileName;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "cannot open " << fileName << " for writing ...";
        qCritical() << "error : "  << file.errorString();
        return;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartDocument();
    xmlWriter.writeDefaultNamespace(nsRdf);
    xmlWriter.writeNamespace(nsDc, "dc");
    xmlWriter.writeNamespace(UBSettings::uniboardDocumentNamespaceUri, "ub");

    xmlWriter.writeStartElement("RDF");

    xmlWriter.writeStartElement("Description");
    xmlWriter.writeAttribute("about", proxy->metaData(UBSettings::documentIdentifer).toString());

    xmlWriter.writeTextElement(nsDc, "title", proxy->metaData(UBSettings::documentName).toString());
    xmlWriter.writeTextElement(nsDc, "type", proxy->metaData(UBSettings::documentGroupName).toString());
    xmlWriter.writeTextElement(nsDc, "date", proxy->metaData(UBSettings::documentDate).toString());
    xmlWriter.writeTextElement(nsDc, "format", "image/svg+xml");

    // introduced in UB 4.2
    xmlWriter.writeTextElement(nsDc, "identifier", proxy->metaData(UBSettings::documentIdentifer).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "version", UBSettings::currentFileVersion);
    QString width = QString::number(proxy->defaultDocumentSize().width());
    QString height = QString::number(proxy->defaultDocumentSize().height());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "size", QString("%1x%2").arg(width).arg(height));

    // introduced in UB 4.4
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "updated-at", UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTimeUtc()));
    // introduced in OpenSankore 1.40.00
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionTitle,proxy->metaData(UBSettings::sessionTitle).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionAuthors,proxy->metaData(UBSettings::sessionAuthors).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionGoals,proxy->metaData(UBSettings::sessionGoals).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionKeywords,proxy->metaData(UBSettings::sessionKeywords).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionGradeLevel,proxy->metaData(UBSettings::sessionGradeLevel).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionBranch,proxy->metaData(UBSettings::sessionBranch).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionType,proxy->metaData(UBSettings::sessionType).toString());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri,UBSettings::sessionLicence,proxy->metaData(UBSettings::sessionLicence).toString());

    xmlWriter.writeEndElement(); //dc:Description
    xmlWriter.writeEndElement(); //RDF

    xmlWriter.writeEndDocument();

    file.flush();
    file.close();
}


QMap<QString, QVariant> UBMetadataDcSubsetAdaptor::load(QString pPath)
{

    QMap<QString, QVariant> metadata;

    QString fileName = pPath + "/" + metadataFilename;

    QFile file(fileName);

    bool sizeFound = false;
    bool updatedAtFound = false;
    QString date;

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Cannot open file " << fileName << " for reading ...";
            return metadata;
        }

        QXmlStreamReader xml(&file);

        while (!xml.atEnd())
        {
            xml.readNext();

            if (xml.isStartElement())
            {
                QString docVersion = "4.1"; // untagged doc version 4.1

                if (xml.name() == "title")
                {
                    metadata.insert(UBSettings::documentName, xml.readElementText());
                }
                else if (xml.name() == "type")
                {
                    metadata.insert(UBSettings::documentGroupName, xml.readElementText());
                }
                else if (xml.name() == "date")
                {
                    date = xml.readElementText();
                }
                else if (xml.name() == "identifier") // introduced in UB 4.2
                {
                        metadata.insert(UBSettings::documentIdentifer, xml.readElementText());
                }
                else if (xml.name() == "version" // introduced in UB 4.2
                        && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                        docVersion = xml.readElementText();
                }
                else if (xml.name() == "size" // introduced in UB 4.2
                        && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    QString size = xml.readElementText();
                    QStringList sizeParts = size.split("x");
                    bool ok = false;
                    int width, height;
                    if (sizeParts.count() >= 2)
                    {
                        bool widthOK, heightOK;
                        width = sizeParts.at(0).toInt(&widthOK);
                        height = sizeParts.at(1).toInt(&heightOK);
                        ok = widthOK && heightOK;

                        QSize docSize(width, height);

                        if (width == 1024 && height == 768) // move from 1024/768 to 1280/960
                        {
                            docSize = UBSettings::settings()->pageSize->get().toSize();
                        }

                        metadata.insert(UBSettings::documentSize, QVariant(docSize));
                    }
                    if (!ok)
                    {
                        qWarning() << "Invalid document size:" << size;
                    }

                    sizeFound = true;

                }
                else if (xml.name() == "updated-at" // introduced in UB 4.4
                        && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::documentUpdatedAt, xml.readElementText());
                    updatedAtFound = true;
                }
                else if (xml.name() == UBSettings::sessionTitle // introduced in OpenSankore 1.40.00
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::sessionTitle, xml.readElementText());
                }
                else if (xml.name() == UBSettings::sessionAuthors // introduced in OpenSankore 1.40.00
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::sessionAuthors, xml.readElementText());
                }
                else if (xml.name() == UBSettings::sessionGoals // introduced in OpenSankore 1.40.00
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::sessionGoals, xml.readElementText());
                }
                else if (xml.name() == UBSettings::sessionKeywords // introduced in OpenSankore 1.40.00
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::sessionKeywords, xml.readElementText());
                }
                else if (xml.name() == UBSettings::sessionGradeLevel // introduced in OpenSankore 1.40.00
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::sessionGradeLevel, xml.readElementText());
                }
                else if (xml.name() == UBSettings::sessionBranch // introduced in OpenSankore 1.40.00
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::sessionBranch, xml.readElementText());
                }
                else if (xml.name() == UBSettings::sessionType // introduced in OpenSankore 1.40.00
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::sessionType, xml.readElementText());
                }
                else if (xml.name() == UBSettings::sessionLicence // introduced in OpenSankore 1.40.00
                         && xml.namespaceUri() == UBSettings::uniboardDocumentNamespaceUri)
                {
                    metadata.insert(UBSettings::sessionLicence, xml.readElementText());
                }

                metadata.insert(UBSettings::documentVersion, docVersion);
            }

            if (xml.hasError())
            {
                qWarning() << "error parsing sankore metadata.rdf file " << xml.errorString();
            }
        }

        file.close();
    }

    if (!sizeFound)
    {
        QDesktopWidget* dw = qApp->desktop();
        int controlScreenIndex = dw->primaryScreen();

        QSize docSize = dw->screenGeometry(controlScreenIndex).size();
        docSize.setHeight(docSize.height() - 70); // 70 = toolbar height

        qWarning() << "Document size not found, using default view size" << docSize;

        metadata.insert(UBSettings::documentSize, QVariant(docSize));
    }

    // this is necessary to update the old files date
    QString dateString = metadata.value(UBSettings::documentDate).toString();
    if(dateString.length() < 10){
        metadata.remove(UBSettings::documentDate);
        metadata.insert(UBSettings::documentDate,dateString+"T00:00:00Z");
    }

    if (!updatedAtFound) {
        metadata.insert(UBSettings::documentUpdatedAt, dateString);
    }

    metadata.insert(UBSettings::documentDate, QVariant(date));


    return metadata;
}

