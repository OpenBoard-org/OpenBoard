/*
 * UBMetadataDcSubsetAdaptor.cpp
 *
 *  Created on: Nov 27, 2008
 *      Author: luc
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
    QString fileName = proxy->persistencePath() + "/" + metadataFilename;
    qWarning() << fileName;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "cannot open " << fileName << " for writing ...";
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

    QDate today = QDate::currentDate();

    xmlWriter.writeTextElement(nsDc, "title", proxy->metaData(UBSettings::documentName).toString());
    xmlWriter.writeTextElement(nsDc, "type", proxy->metaData(UBSettings::documentGroupName).toString());
    xmlWriter.writeTextElement(nsDc, "date", QDate::currentDate().toString("yyyy-MM-dd"));
    xmlWriter.writeTextElement(nsDc, "format", "image/svg+xml");

    // introduced in UB 4.2
    xmlWriter.writeTextElement(nsDc, "identifier", proxy->metaData(UBSettings::documentIdentifer).toString());
        xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "version", UBSettings::currentFileVersion);
    QString width = QString::number(proxy->defaultDocumentSize().width());
    QString height = QString::number(proxy->defaultDocumentSize().height());
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "size", QString("%1x%2").arg(width).arg(height));

    // introduced in UB 4.4
    xmlWriter.writeTextElement(UBSettings::uniboardDocumentNamespaceUri, "updated-at", proxy->metaData(UBSettings::documentUpdatedAt).toString());

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
                            docSize = UBSettings::settings()->defaultDocumentSize;
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

    if (!updatedAtFound)
    {
        metadata.insert(UBSettings::documentUpdatedAt, date + "T00:00:00Z");
    }

    return metadata;
}

