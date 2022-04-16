/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
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



#include "UBCFFAdaptor.h"

#include <QtCore>
#include <QtXml>
#include <QTransform>
#include <QGraphicsItem>
#include <QSvgRenderer>
#include <QPainter>

#include "UBGlobals.h"
#include "UBCFFConstants.h"

//THIRD_PARTY_WARNINGS_DISABLE
#ifdef Q_OS_OSX
    #include <quazip.h>
    #include <quazipfile.h>
    #include <quazipfileinfo.h>
#else
    #include "quazip.h"
    #include "quazipfile.h"
    #include "quazipfileinfo.h"
#endif
//THIRD_PARTY_WARNINGS_ENABLE

UBCFFAdaptor::UBCFFAdaptor()
{}

bool UBCFFAdaptor::convertUBZToIWB(const QString &from, const QString &to)
{
    qDebug() << "starting converion from" << from << "to" << to;

    QString source = QString();
    if (QFileInfo(from).isDir() && QFile::exists(from)) {
        qDebug() << "File specified is dir, continuing convertion";
        source = from;
    } else {
        source = uncompressZip(from);
        if (!source.isNull()) qDebug() << "File specified is zip file. Uncompressed to tmp dir, continuing convertion";
    }
    if (source.isNull()) {
        qDebug() << "File specified is not a dir or a zip file, stopping covretion";
        return false;
    }

    QString tmpDestination = createNewTmpDir();
    if (tmpDestination.isNull()) {
        qDebug() << "can't create temp destination folder. Stopping parsing...";
        return false;
    }

    UBToCFFConverter tmpConvertrer(source, tmpDestination);
    if (!tmpConvertrer) {
        qDebug() << "The convertrer class is invalid, stopping conversion. Error message" << tmpConvertrer.lastErrStr();
        return false;
    }

    bool bParceRes = tmpConvertrer.parse();

    mConversionMessages << tmpConvertrer.getMessages();

    if (!bParceRes) {
        return false;
    }

    if (!compressZip(tmpDestination, to))
        qDebug() << "error in compression";

    //Cleanning tmp souces in filesystem
    if (!QFileInfo(from).isDir())
    if (!freeDir(source))
        qDebug() << "can't delete tmp directory" << QDir(source).absolutePath() << "try to delete them manually";

    if (!freeDir(tmpDestination))
        qDebug() << "can't delete tmp directory" << QDir(tmpDestination).absolutePath() << "try to delete them manually";

    return true;
}

QString UBCFFAdaptor::uncompressZip(const QString &zipFile)
{
    QuaZip zip(zipFile);

    if(!zip.open(QuaZip::mdUnzip)) {
        qWarning() << "Import failed. Cause zip.open(): " << zip.getZipError();
        return QString();
    }

    zip.setFileNameCodec("UTF-8");
    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    //create unique cff document root fodler
    QString documentRootFolder = createNewTmpDir();

    if (documentRootFolder.isNull()) {
        qDebug() << "can't create tmp directory for zip file" << zipFile;
        return QString();
    }

    QDir rootDir(documentRootFolder);
    QFile out;
    char c;
    bool allOk = true;
    for(bool more = zip.goToFirstFile(); more; more=zip.goToNextFile()) {
        if(!zip.getCurrentFileInfo(&info)) {
            qWarning() << "Import failed. Cause: getCurrentFileInfo(): " << zip.getZipError();
            allOk = false;
            break;
        }
        if(!file.open(QIODevice::ReadOnly)) {
            allOk = false;
            break;
        }
        if(file.getZipError()!= UNZ_OK) {
            qWarning() << "Import failed. Cause: file.getFileName(): " << zip.getZipError();
            allOk = false;
            break;
        }

        QString newFileName = documentRootFolder + "/" + file.getActualFileName();

        QFileInfo newFileInfo(newFileName);
        rootDir.mkpath(newFileInfo.absolutePath());

        out.setFileName(newFileName);
        out.open(QIODevice::WriteOnly);

        while(file.getChar(&c))
            out.putChar(c);

        out.close();

        if(file.getZipError()!=UNZ_OK) {
            qWarning() << "Import failed. Cause: " << zip.getZipError();
            allOk = false;
            break;
        }
        if(!file.atEnd()) {
            qWarning() << "Import failed. Cause: read all but not EOF";
            allOk = false;
            break;
        }
        file.close();

        if(file.getZipError()!=UNZ_OK) {
            qWarning() << "Import failed. Cause: file.close(): " <<  file.getZipError();
            allOk = false;
            break;
        }
    }

    if (!allOk) {
        out.close();
        file.close();
        zip.close();
        return QString();
    }

    if(zip.getZipError()!=UNZ_OK) {
        qWarning() << "Import failed. Cause: zip.close(): " << zip.getZipError();
        return QString();
    }

    return documentRootFolder;
}

bool UBCFFAdaptor::compressZip(const QString &source, const QString &destination)
{
    QDir toDir = QFileInfo(destination).dir();
    if (!toDir.exists())
        if (!QDir().mkpath(toDir.absolutePath())) {
            qDebug() << "can't create destination folder to uncompress file";
            return false;
        }

    QuaZip zip(destination);
    zip.setFileNameCodec("UTF-8");
    if(!zip.open(QuaZip::mdCreate)) {
        qDebug("Export failed. Cause: zip.open(): %d", zip.getZipError());
        return false;
    }

    QuaZipFile outZip(&zip);

    QFileInfo sourceInfo(source);
    if (sourceInfo.isDir()) {
        if (!compressDir(QFileInfo(source).absoluteFilePath(), "", &outZip))
            return false;
    } else if (sourceInfo.isFile()) {
        if (!compressFile(QFileInfo(source).absoluteFilePath(), "", &outZip))
            return false;
    }

    return true;
}

bool UBCFFAdaptor::compressDir(const QString &dirName, const QString &parentDir, QuaZipFile *outZip)
{
    QFileInfoList dirFiles = QDir(dirName).entryInfoList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    QListIterator<QFileInfo> iter(dirFiles);
    while (iter.hasNext()) {
        QFileInfo curFile = iter.next();

        if (curFile.isDir()) {
            if (!compressDir(curFile.absoluteFilePath(), parentDir + curFile.fileName() + "/", outZip)) {
                qDebug() << "error at compressing dir" << curFile.absoluteFilePath();
                return false;
            }
        } else if (curFile.isFile()) {
            if (!compressFile(curFile.absoluteFilePath(), parentDir, outZip)) {
               return false;
            }
        }
    }

    return true;
}

bool UBCFFAdaptor::compressFile(const QString &fileName, const QString &parentDir, QuaZipFile *outZip)
{
    QFile sourceFile(fileName);

    if(!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Compression of file" << sourceFile.fileName() << " failed. Cause: inFile.open(): " << sourceFile.errorString();
        return false;
    }

    if(!outZip->open(QIODevice::WriteOnly, QuaZipNewInfo(parentDir + QFileInfo(fileName).fileName(), sourceFile.fileName()))) {
        qDebug() << "Compression of file" << sourceFile.fileName() << " failed. Cause: outFile.open(): " << outZip->getZipError();
        sourceFile.close();
        return false;
    }

    outZip->write(sourceFile.readAll());
    if(outZip->getZipError() != UNZ_OK) {
        qDebug() << "Compression of file" << sourceFile.fileName() << " failed. Cause: outFile.write(): " << outZip->getZipError();

        sourceFile.close();
        outZip->close();
        return false;
    }

    if(outZip->getZipError() != UNZ_OK)
    {
        qWarning() << "Compression of file" << sourceFile.fileName() << " failed. Cause: outFile.close(): " << outZip->getZipError();

        sourceFile.close();
        outZip->close();
        return false;
    }

    outZip->close();
    sourceFile.close();

    return true;
}

QString UBCFFAdaptor::createNewTmpDir()
{
    int tmpNumber = 0;
    QDir systemTmp = QDir::temp();

    while (true) {
        QString dirName = QString("CFF_adaptor_filedata_store%1.%2")
                .arg(QDateTime::currentDateTime().toString("dd_MM_yyyy_HH-mm"))
                .arg(tmpNumber++);
        if (!systemTmp.exists(dirName)) {
            if (systemTmp.mkdir(dirName)) {
                QString result = systemTmp.absolutePath() + "/" + dirName;
                tmpDirs.append(result);
                return result;
            } else {
                qDebug() << "Can't create temporary dir maybe due to permissions";
                return QString();
            }
        } else if (tmpNumber == 10) {
            qWarning() << "Import failed. Failed to create temporary file ";
            return QString();
        }
        tmpNumber++;
    }

    return QString();
}
bool UBCFFAdaptor::deleteDir(const QString& pDirPath) const
{
    if (pDirPath == "" || pDirPath == "." || pDirPath == "..")
        return false;

    QDir dir(pDirPath);

    if (dir.exists())
    {
        foreach(QFileInfo dirContent, dir.entryInfoList(QDir::Files | QDir::Dirs
                | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::Name))
        {
            if (dirContent.isDir())
            {
                deleteDir(dirContent.absoluteFilePath());
            }
            else
            {
                if (!dirContent.dir().remove(dirContent.fileName()))
                {
                    return false;
                }
            }
        }
    }

    return dir.rmdir(pDirPath);
}

QList<QString> UBCFFAdaptor::getConversionMessages() 
{
    return mConversionMessages;
}

bool UBCFFAdaptor::freeDir(const QString &dir)
{
    bool result = true;
    if (!deleteDir(dir))
        result = false;

    tmpDirs.removeAll(QDir(dir).absolutePath());

    return result;
}
void UBCFFAdaptor::freeTmpDirs()
{
    foreach (QString dir, tmpDirs)
        freeDir(dir);
}

UBCFFAdaptor::~UBCFFAdaptor()
{
    freeTmpDirs();
}

UBCFFAdaptor::UBToCFFConverter::UBToCFFConverter(const QString &source, const QString &destination)
{
    sourcePath = source;
    destinationPath = destination;

    errorStr = noErrorMsg;
    mDataModel = new QDomDocument;
    mDocumentToWrite = new QDomDocument; 
    mDocumentToWrite->setContent(QString("<doc></doc>"));

    mIWBContentWriter = new QXmlStreamWriter;
    mIWBContentWriter->setAutoFormatting(true);

    iwbSVGItemsAttributes.insert(tIWBImage, iwbSVGImageAttributes);
    iwbSVGItemsAttributes.insert(tIWBVideo, iwbSVGVideoAttributes);
    iwbSVGItemsAttributes.insert(tIWBText, iwbSVGTextAttributes);
    iwbSVGItemsAttributes.insert(tIWBTextArea, iwbSVGTextAreaAttributes);
    iwbSVGItemsAttributes.insert(tIWBPolyLine, iwbSVGPolyLineAttributes);
    iwbSVGItemsAttributes.insert(tIWBPolygon, iwbSVGPolygonAttributes);
    iwbSVGItemsAttributes.insert(tIWBRect, iwbSVGRectAttributes);
    iwbSVGItemsAttributes.insert(tIWBLine, iwbSVGLineAttributes);
    iwbSVGItemsAttributes.insert(tIWBTspan, iwbSVGTspanAttributes);
}

bool UBCFFAdaptor::UBToCFFConverter::parse()
{
    if(!isValid()) {
        qDebug() << "document metadata is not valid. Can't parse";
        return false;
    }

    qDebug() << "begin parsing ubz";

    QFile outFile(contentIWBFileName());
    if (!outFile.open(QIODevice::WriteOnly| QIODevice::Text)) {
        qDebug() << "can't open output file for writing";
        errorStr = "createXMLOutputPatternError";
        return false;
    }

    mIWBContentWriter->setDevice(&outFile);

    mIWBContentWriter->writeStartDocument();
    mIWBContentWriter->writeStartElement(tIWBRoot);
    
    fillNamespaces();

    mIWBContentWriter->writeAttribute(aIWBVersion, avIWBVersionNo);

    if (!parseMetadata()) {
        if (errorStr == noErrorMsg)
            errorStr = "MetadataParsingError";

        outFile.close();
        return false;
    }

    if (!parseContent()) {
        if (errorStr == noErrorMsg)
            errorStr = "ContentParsingError";
        outFile.close();
        return false;
    }

    mIWBContentWriter->writeEndElement();
    mIWBContentWriter->writeEndDocument();

    outFile.close();

    qDebug() << "finished with success";

    return true;
}
bool UBCFFAdaptor::UBToCFFConverter::parseMetadata()
{
    int errorLine, errorColumn;
    QFile metaDataFile(sourcePath + "/" + fMetadata);

    if (!metaDataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorStr = "can't open" + QFileInfo(sourcePath + "/" + fMetadata).absoluteFilePath();
        qDebug() << errorStr;
        return false;

    } else if (!mDataModel->setContent(metaDataFile.readAll(), true, &errorStr, &errorLine, &errorColumn)) {
        qWarning() << "Error:Parseerroratline" << errorLine << ","
                   << "column" << errorColumn << ":" << errorStr;
        return false;
    }

    QDomElement nextInElement = mDataModel->documentElement();

    nextInElement = nextInElement.firstChildElement(tDescription);
    if (!nextInElement.isNull()) {

        mIWBContentWriter->writeStartElement(iwbNS, tIWBMeta);
        mIWBContentWriter->writeAttribute(aIWBName, aCreator);
        mIWBContentWriter->writeAttribute(aIWBContent, avCreator);
        mIWBContentWriter->writeEndElement();

        mIWBContentWriter->writeStartElement(iwbNS, tIWBMeta);
        mIWBContentWriter->writeAttribute(aIWBName, aOwner);
        mIWBContentWriter->writeAttribute(aIWBContent, avOwner);
        mIWBContentWriter->writeEndElement();

        mIWBContentWriter->writeStartElement(iwbNS, tIWBMeta);
        mIWBContentWriter->writeAttribute(aIWBName, aDescription);
        mIWBContentWriter->writeAttribute(aIWBContent, avDescription);
        mIWBContentWriter->writeEndElement();

        mIWBContentWriter->writeStartElement(iwbNS, tIWBMeta);
        mIWBContentWriter->writeAttribute(aIWBName, aAbout);
        mIWBContentWriter->writeAttribute(aIWBContent, nextInElement.attribute(aAbout));
        mIWBContentWriter->writeEndElement();

        nextInElement = nextInElement.firstChildElement();
        while (!nextInElement.isNull()) {

            QString textContent = nextInElement.text();
            if (!textContent.trimmed().isEmpty()) {
                if (nextInElement.tagName() == tUBZSize) { //taking main viewbox rect since for CFF specificaton we have static viewbox
                    QSize tmpSize = getSVGDimentions(nextInElement.text());
                    if (!tmpSize.isNull()) {
                        mSVGSize = tmpSize;
                    } else {
                        qDebug() << "can't interpret svg section size";
                        errorStr = "InterpretSvgSizeError";
                        return false;
                    }
                } else {
                    mIWBContentWriter->writeStartElement(iwbNS, tIWBMeta);
                    mIWBContentWriter->writeAttribute(aIWBName, nextInElement.tagName());
                    mIWBContentWriter->writeAttribute(aIWBContent, textContent);
                    mIWBContentWriter->writeEndElement();
                }

            }
            nextInElement = nextInElement.nextSiblingElement();
        }
    }

    metaDataFile.close();
    return true;
}
bool UBCFFAdaptor::UBToCFFConverter::parseContent() {

    QDir sourceDir(sourcePath);
    QStringList fileFilters;
    fileFilters << QString(pageAlias + "???." + pageFileExtentionUBZ);
    QStringList pageList = sourceDir.entryList(fileFilters, QDir::Files, QDir::Name | QDir::IgnoreCase);

    QDomElement svgDocumentSection = mDataModel->createElementNS(svgIWBNS, ":"+tSvg);

    if (!pageList.count()) {
        qDebug() << "can't find any content file";
        errorStr = "ErrorContentFile";
        return false;
    } else 
    {
        QDomElement pageset = parsePageset(pageList);
        if (pageset.isNull())
            return false;
        else
            svgDocumentSection.appendChild(pageset);
    }

    
    if (QRect() == mViewbox)
    {
        mViewbox.setRect(0,0, mSVGSize.width(), mSVGSize.height());
    }

    svgDocumentSection.setAttribute(aIWBViewBox, rectToIWBAttr(mViewbox));
    svgDocumentSection.setAttribute(aWidth, QString("%1").arg(mViewbox.width()));
    svgDocumentSection.setAttribute(aHeight, QString("%1").arg(mViewbox.height()));


    writeQDomElementToXML(svgDocumentSection);


    if (!writeExtendedIwbSection()) {
        if (errorStr == noErrorMsg)
            errorStr = "writeExtendedIwbSectionError";
        return false;
    }

    return true;
}

QDomElement UBCFFAdaptor::UBToCFFConverter::parsePage(const QString &pageFileName)
{
    qDebug() << "begin parsing page" + pageFileName;
    mSvgElements.clear(); //clean Svg elements map before parsing new page

    int errorLine, errorColumn;

    QFile pageFile(sourcePath + "/" + pageFileName);
    if (!pageFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "can't open file" << pageFileName << "for reading";
        return QDomElement();
    } else if (!mDataModel->setContent(pageFile.readAll(), true, &errorStr, &errorLine, &errorColumn)) {
        qWarning() << "Error:Parseerroratline" << errorLine << ","
                   << "column" << errorColumn << ":" << errorStr;
        pageFile.close();
        return QDomElement();
    }

    QDomElement page;
    QDomElement group;

    QDomElement nextTopElement = mDataModel->firstChildElement();
    while (!nextTopElement.isNull()) {
        QString tagname = nextTopElement.tagName();
        if (tagname == tSvg) {
            page = parseSvgPageSection(nextTopElement);
            if (page.isNull()) {
                qDebug() << "The page is empty.";
                pageFile.close();
                return QDomElement();
            }
        } else if (tagname == tUBZGroups) {
            group = parseGroupsPageSection(nextTopElement);
            if (group.isNull()) {
                qDebug() << "Page doesn't contains any groups.";
                pageFile.close();
                return QDomElement();
            }
        }

        nextTopElement = nextTopElement.nextSiblingElement();
    }

    pageFile.close(); 

    return page.hasChildNodes() ? page : QDomElement();
}

QDomElement UBCFFAdaptor::UBToCFFConverter::parsePageset(const QStringList &pageFileNames)
{   
    QMultiMap<int, QDomElement> pageList;
    int iPageNo = 1;

    QStringListIterator curPage(pageFileNames);

    while (curPage.hasNext()) {

        QString curPageFile = curPage.next();
        QDomElement iterElement = parsePage(curPageFile);
        if (!iterElement.isNull())           
        {
            iterElement.setAttribute(tId, iPageNo);
            addSVGElementToResultModel(iterElement, pageList, iPageNo);
            iPageNo++; 
        }
        else
            return QDomElement();
    }


    if (!pageList.count())
        return QDomElement(); 


    QDomElement svgPagesetElement = mDocumentToWrite->createElementNS(svgIWBNS,":"+ tIWBPageSet);

    QMapIterator<int, QDomElement> nextSVGElement(pageList);
    nextSVGElement.toFront();
    while (nextSVGElement.hasNext()) 
        svgPagesetElement.appendChild(nextSVGElement.next().value());

    return svgPagesetElement.hasChildNodes() ? svgPagesetElement : QDomElement();
}
QDomElement UBCFFAdaptor::UBToCFFConverter::parseSvgPageSection(const QDomElement &element)
{
    //we don't know about page number, so return QDomElement.

    //Parsing top level tag attributes

    //getting current page viewbox to be able to convert coordinates to global viewbox parameter
    if (element.hasAttribute(aUBZViewBox)) {
        setViewBox(getViewboxRect(element.attribute(aUBZViewBox)));
    }

    QMultiMap<int, QDomElement> svgElements;

    QDomElement svgElementPart = mDocumentToWrite->createElementNS(svgIWBNS,":"+ tIWBPage);

    if (element.hasAttribute(aDarkBackground)) {
         createBackground(element, svgElements);       
    }
   
    //Parsing svg children attributes
    // Elements can know about its layer, so it must add result QDomElements to ordrered list.
    QDomElement nextElement = element.firstChildElement();
    while (!nextElement.isNull()) {
        QString tagName = nextElement.tagName();
        if      (tagName == tUBZG)             parseSVGGGroup(nextElement, svgElements);
        else if (tagName == tUBZImage)         parseUBZImage(nextElement, svgElements);
        else if (tagName == tUBZVideo)         parseUBZVideo(nextElement, svgElements);
        else if (tagName == tUBZAudio)         parseUBZAudio(nextElement, svgElements);
        else if (tagName == tUBZForeignObject) parseForeignObject(nextElement, svgElements);
        else if (tagName == tUBZLine)          parseUBZLine(nextElement, svgElements);
        else if (tagName == tUBZPolygon)       parseUBZPolygon(nextElement, svgElements);
        else if (tagName == tUBZPolyline)      parseUBZPolyline(nextElement, svgElements);
        else if (tagName == tUBZGroups)        parseGroupsPageSection(nextElement);

        nextElement = nextElement.nextSiblingElement();
    }

    if (0 == svgElements.count())
        return QDomElement();

    // to do:
    // there we must to sort elements (take elements from list and assign parent ordered like in parseSVGGGroup)
    // we returns just element because we don't care about layer.
    QMapIterator<int, QDomElement> nextSVGElement(svgElements);
    nextSVGElement.toFront();
    while (nextSVGElement.hasNext()) 
        svgElementPart.appendChild(nextSVGElement.next().value());
 
    return svgElementPart.hasChildNodes() ? svgElementPart : QDomElement();
}

void UBCFFAdaptor::UBToCFFConverter::writeQDomElementToXML(const QDomNode &node)
{
    if (!node.isNull()) {
        if (node.isText())
            mIWBContentWriter->writeCharacters(node.nodeValue());
        else {
            mIWBContentWriter->writeStartElement(node.namespaceURI(), node.toElement().tagName());

            for (int i = 0; i < node.toElement().attributes().count(); i++) {
                QDomAttr attr =  node.toElement().attributes().item(i).toAttr();
                mIWBContentWriter->writeAttribute(attr.name(), attr.value());
            }
            QDomNode child = node.firstChild();
            while(!child.isNull()) {
                writeQDomElementToXML(child);
                child = child.nextSibling();
            }

            mIWBContentWriter->writeEndElement();
        }
    }
}

bool UBCFFAdaptor::UBToCFFConverter::writeExtendedIwbSection()
{
    if (!mExtendedElements.count()) {
        qDebug() << "extended iwb content list is empty";
        errorStr = "EmptyExtendedIwbSectionContentError";
        return false;
    }
    QListIterator<QDomElement> nextExtendedIwbElement(mExtendedElements);
    while (nextExtendedIwbElement.hasNext()) {
        writeQDomElementToXML(nextExtendedIwbElement.next());
        //TODO write iwb extended element to mIWBContentWriter
    }

    return true;
}

// extended element options
// editable, background, locked are supported for now

QDomElement UBCFFAdaptor::UBToCFFConverter::parseGroupsPageSection(const QDomElement &groupRoot)
{
//    First sankore side implementation needed. TODO in Sankore 1.5
    if (!groupRoot.hasChildNodes()) {
        qDebug() << "Group root is empty";
        return QDomElement();
    }

    QDomElement groupElement = groupRoot.firstChildElement();

    while (!groupElement.isNull()) {
        QDomElement extendedElement = mDataModel->createElementNS(iwbNS, groupElement.tagName());
        QDomElement groupChildElement = groupElement.firstChildElement();
        while (!groupChildElement.isNull()) {
            QDomElement extSubElement = mDataModel->createElementNS(iwbNS, groupChildElement.tagName());
            extSubElement.setAttribute(aRef, groupChildElement.attribute(aID, QUuid().toString()));
            extendedElement.appendChild(extSubElement);

            groupChildElement = groupChildElement.nextSiblingElement();
        }

        mExtendedElements.append(extendedElement);

        groupElement = groupElement.nextSiblingElement();
    }

    qDebug() << "parsing ubz group section";
    return groupRoot;
}

QString UBCFFAdaptor::UBToCFFConverter::getDstContentFolderName(const QString &elementType)
{
    QString sRet;
    QString sDstContentFolderName;

    // widgets must be saved as .png images.
    if ((tIWBImage == elementType) || (tUBZForeignObject == elementType))
        sDstContentFolderName = cfImages;
    else
    if (tIWBVideo == elementType)
        sDstContentFolderName = cfVideos;    
    else
    if (tIWBAudio == elementType)
        sDstContentFolderName = cfAudios;

    sRet = sDstContentFolderName;
 
    return sRet;
}

QString UBCFFAdaptor::UBToCFFConverter::getSrcContentFolderName(QString href)
{
    QString sRet;

    QStringList ls = href.split("/");
    for (int i = 0; i < ls.count()-1; i++)
    {
        QString sPart = ls.at(i);
        if (ubzContentFolders.contains(sPart))
        {
            sRet = sPart;
        }
    }

//     if (0 < ls.count())
//         sRet = ls.at(ls.count()-1);
//     
//     sRet = href.remove(sRet);
// 
//     if (sRet.endsWith("/"))
//         sRet.remove("/");

    return sRet;   
}

QString UBCFFAdaptor::UBToCFFConverter::getFileNameFromPath(const QString sPath)
{
    QString sRet;
    QStringList sl = sPath.split("/",QString::SkipEmptyParts);

    if (0 < sl.count())
    {
        QString name = sl.at(sl.count()-1);
        QString extention = getExtentionFromFileName(name);

        if (feWgt == extention)
        {
            name.remove("{");
            name.remove("}");
        }

        name.remove(name.length()-extention.length(), extention.length());
        name += convertExtention(extention);

        sRet = name;
    }
    return sRet;
}

QString UBCFFAdaptor::UBToCFFConverter::getExtentionFromFileName(const QString &filename)
{
    QStringList sl = filename.split("/",QString::SkipEmptyParts);

    if (0 < sl.count())
    {
        QString name = sl.at(sl.count()-1);
        QStringList tl = name.split(".");
        return tl.at(tl.count()-1);
    }
    return QString();
}

QString UBCFFAdaptor::UBToCFFConverter::convertExtention(const QString &ext)
{
    QString sRet;

    if (feSvg == ext)
        sRet = fePng;
    else
    if (feWgt == ext)
        sRet = fePng;
    else 
        sRet = ext;

    return sRet;
}

QString UBCFFAdaptor::UBToCFFConverter::getElementTypeFromUBZ(const QDomElement &element)
{
    QString sRet;
    if (tUBZForeignObject == element.tagName())
    {
        QString sPath;
        if (element.hasAttribute(aUBZType))
        {
            if (avUBZText == element.attribute(aUBZType))
                sRet = tIWBTextArea;
            else
                sRet = element.attribute(aUBZType);
        }
        else
        {      
            if (element.hasAttribute(aSrc))
                sPath = element.attribute(aSrc);
            else 
            if (element.hasAttribute(aUBZHref))
                sPath = element.attribute(aUBZHref);

            QStringList tsl = sPath.split(".", QString::SkipEmptyParts);
            if (0 < tsl.count())
            {
                QString elementType = tsl.at(tsl.count()-1);
                if (iwbElementImage.contains(elementType))
                    sRet = tIWBImage;
                else 
                if (iwbElementAudio.contains(elementType))
                    sRet = tIWBAudio;
                else
                if (iwbElementVideo.contains(elementType))
                    sRet = tIWBVideo;   
            }
        }
    }
    else
        sRet = element.tagName();

    return sRet;
}

int UBCFFAdaptor::UBToCFFConverter::getElementLayer(const QDomElement &element)
{
    int iRetLayer = 0;
    if (element.hasAttribute(aZLayer))
        iRetLayer = (int)element.attribute(aZLayer).toDouble();
    else 
        iRetLayer = DEFAULT_LAYER;

    return iRetLayer;
}

bool UBCFFAdaptor::UBToCFFConverter::itIsSupportedFormat(const QString &format) const
{
    bool bRet;

    QStringList tsl = format.split(".", QString::SkipEmptyParts);
    if (0 < tsl.count())
        bRet = cffSupportedFileFormats.contains(tsl.at(tsl.count()-1).toLower());       
    else
        bRet = false;

    return bRet;
}

bool UBCFFAdaptor::UBToCFFConverter::itIsFormatToConvert(const QString &format) const
{
    foreach (QString f, ubzFormatsToConvert.split(","))
    {
        if (format == f)
            return true;
    }
    return false;
}

bool UBCFFAdaptor::UBToCFFConverter::itIsSVGElementAttribute(const QString ItemType, const QString &AttrName)
{
    QString allowedElementAttributes = iwbSVGItemsAttributes[ItemType];
  
    allowedElementAttributes.remove("/t");
    allowedElementAttributes.remove(" ");
    foreach(QString attr, allowedElementAttributes.split(","))
    {
        if (AttrName == attr.trimmed())
            return true;
    }
    return false;
}


bool UBCFFAdaptor::UBToCFFConverter::itIsIWBAttribute(const QString &attribute) const
{
    foreach (QString attr, iwbElementAttributes.split(","))
    {
       if (attribute == attr.trimmed())
           return true;
    }
    return false;
}

bool UBCFFAdaptor::UBToCFFConverter::itIsUBZAttributeToConvert(const QString &attribute) const
{
    foreach (QString attr, ubzElementAttributesToConvert.split(","))
    {
        if (attribute == attr.trimmed())
            return true;
    }
    return false;
}

bool UBCFFAdaptor::UBToCFFConverter::ibwAddLine(int x1, int y1, int x2, int y2, QString color, int width, bool isBackground)
{
    bool bRet = true;

    QDomDocument doc;

    QDomElement svgBackgroundCrossPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":line");
    QDomElement iwbBackgroundCrossPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);

    QString sUUID = QUuid::createUuid().toString();

    svgBackgroundCrossPart.setTagName(tIWBLine);

    svgBackgroundCrossPart.setAttribute(aX+"1", x1);
    svgBackgroundCrossPart.setAttribute(aY+"1", y1);  
    svgBackgroundCrossPart.setAttribute(aX+"2", x2);
    svgBackgroundCrossPart.setAttribute(aY+"2", y2);

    svgBackgroundCrossPart.setAttribute(aStroke, color);
    svgBackgroundCrossPart.setAttribute(aStrokeWidth, width);

    svgBackgroundCrossPart.setAttribute(aID, sUUID);

    if (isBackground)     
    {
        iwbBackgroundCrossPart.setAttribute(aRef, sUUID);
        iwbBackgroundCrossPart.setAttribute(aLocked, avTrue);

        addIWBElementToResultModel(iwbBackgroundCrossPart);
    }

    addSVGElementToResultModel(svgBackgroundCrossPart, mSvgElements, DEFAULT_BACKGROUND_CROSS_LAYER);

    if (!bRet)
    {
        qDebug() << "|error at creating crosses on background";
        errorStr = "CreatingCrossedBackgroundParsingError.";
    }

    return bRet;
}

QTransform UBCFFAdaptor::UBToCFFConverter::getTransformFromUBZ(const QDomElement &ubzElement)
{
    QTransform trRet;
  
    QStringList transformParameters;

    QString ubzTransform = ubzElement.attribute(aTransform);
    ubzTransform.remove("matrix");
    ubzTransform.remove("(");
    ubzTransform.remove(")");

    transformParameters = ubzTransform.split(",", QString::SkipEmptyParts);

    if (6 <= transformParameters.count())
    {
        QTransform *tr = NULL;
        tr = new QTransform(transformParameters.at(0).toDouble(),
            transformParameters.at(1).toDouble(),
            transformParameters.at(2).toDouble(),
            transformParameters.at(3).toDouble(),
            transformParameters.at(4).toDouble(),
            transformParameters.at(5).toDouble());

        trRet = *tr;
        
        delete tr;
    }

    if (6 <= transformParameters.count())
    {
        QTransform *tr = NULL;
        tr = new QTransform(transformParameters.at(0).toDouble(),
            transformParameters.at(1).toDouble(),
            transformParameters.at(2).toDouble(),
            transformParameters.at(3).toDouble(),
            transformParameters.at(4).toDouble(),
            transformParameters.at(5).toDouble());

        trRet = *tr;
        
        delete tr;
    }
    return trRet;
}

qreal UBCFFAdaptor::UBToCFFConverter::getAngleFromTransform(const QTransform &tr)
{
    qreal angle = -(atan(tr.m21()/tr.m11())*180/PI);
    if (tr.m21() > 0 && tr.m11() < 0) 
        angle += 180;
    else 
        if (tr.m21() < 0 && tr.m11() < 0) 
            angle += 180;
    return angle;
}

void UBCFFAdaptor::UBToCFFConverter::setGeometryFromUBZ(const QDomElement &ubzElement, QDomElement &iwbElement)
{
    setCoordinatesFromUBZ(ubzElement,iwbElement);

  
 
}

void UBCFFAdaptor::UBToCFFConverter::setCoordinatesFromUBZ(const QDomElement &ubzElement, QDomElement &iwbElement)
{
    QTransform tr;

    if (QString() != ubzElement.attribute(aTransform))
        tr = getTransformFromUBZ(ubzElement);

    qreal x = ubzElement.attribute(aX).toDouble();
    qreal y = ubzElement.attribute(aY).toDouble();
    qreal height = ubzElement.attribute(aHeight).toDouble();
    qreal width = ubzElement.attribute(aWidth).toDouble();

    qreal alpha = getAngleFromTransform(tr);
 
    QRectF itemRect;
    QGraphicsRectItem item;

    item.setRect(0,0, width, height);
    item.setTransform(tr);
    item.setRotation(-alpha);
    QMatrix sceneMatrix = item.sceneMatrix();
 
    iwbElement.setAttribute(aX, x);
    iwbElement.setAttribute(aY, y);
    iwbElement.setAttribute(aHeight, height*sceneMatrix.m22());
    iwbElement.setAttribute(aWidth, width*sceneMatrix.m11());
    iwbElement.setAttribute(aTransform, QString("rotate(%1) translate(%2,%3)").arg(alpha)
                                                                              .arg(sceneMatrix.dx())
                                                                              .arg(sceneMatrix.dy()));
}

bool UBCFFAdaptor::UBToCFFConverter::setContentFromUBZ(const QDomElement &ubzElement, QDomElement &svgElement)
{
    bool bRet = true;
   
    QString srcPath;
    if (tUBZForeignObject != ubzElement.tagName())
        srcPath = ubzElement.attribute(aUBZHref);
    else 
        srcPath = ubzElement.attribute(aSrc);

    QString sSrcContentFolder = getSrcContentFolderName(srcPath);
    QString sSrcFileName = sourcePath + "/" + srcPath ;
    QString fileExtention = getExtentionFromFileName(sSrcFileName);
    QString sDstContentFolder = getDstContentFolderName(ubzElement.tagName());
    QString sDstFileName(QString(QUuid::createUuid().toString()+"."+convertExtention(fileExtention)));


    if (itIsSupportedFormat(fileExtention)) // format is supported and we can copy src. files without changing.
    {
        sSrcFileName = sourcePath + "/" + sSrcContentFolder + "/" + getFileNameFromPath(srcPath); // some elements must be exported as images, so we take hes existing thumbnails.

        QFile srcFile;
        srcFile.setFileName(sSrcFileName);

        QDir dstDocFolder(destinationPath);

        if (!dstDocFolder.exists(sDstContentFolder))
            bRet &= dstDocFolder.mkdir(sDstContentFolder);

        if (bRet)
        {
            QString dstFilePath = destinationPath+"/"+sDstContentFolder+"/"+sDstFileName;
            bRet &= srcFile.copy(dstFilePath);
        }

        if (bRet)
        {
            svgElement.setAttribute(aSVGHref, sDstContentFolder+"/"+sDstFileName);
            // NOT by standard! Enable it later!
            // validator http://validator.imsglobal.org/iwb/index.jsp?validate=package
            //svgElement.setAttribute(aSVGRequiredExtension, svgRequiredExtensionPrefix+convertExtention(fileExtention));
        }
    }
    else
    if (itIsFormatToConvert(fileExtention)) // we cannot copy that source files. We need to create dst. file from src. file without copy. 
    {
        if (feSvg == fileExtention)
        {
            QDir dstDocFolder(destinationPath);

            if (!dstDocFolder.exists(sDstContentFolder))
                bRet &= dstDocFolder.mkdir(sDstContentFolder);

            if (bRet)
            {
                if (feSvg == fileExtention) // svg images must be converted to PNG.
                {           
                    QString dstFilePath = destinationPath+"/"+sDstContentFolder+"/"+sDstFileName;
                    bRet &= createPngFromSvg(sSrcFileName, dstFilePath, getTransformFromUBZ(ubzElement));
                }
                else
                    bRet = false;
            }

            if (bRet)
            {
                svgElement.setAttribute(aSVGHref, sDstContentFolder+"/"+sDstFileName);
                // NOT by standard! Enable it later!
                // validator http://validator.imsglobal.org/iwb/index.jsp?validate=package
                //svgElement.setAttribute(aSVGRequiredExtension, svgRequiredExtensionPrefix+fePng);
            }
        }
    }else
    {
        addLastExportError(QObject::tr("Element ID = ") + QString("%1 \r\n").arg(ubzElement.attribute(aUBZUuid)) 
                         + QString("Source file  = ") + QString("%1 \r\n").arg(ubzElement.attribute(aUBZSource))
                         + QObject::tr("Content is not supported in destination format."));
        bRet = false;
    }
   
    if (!bRet)
    {
        qDebug() << "format is not supported by CFF";
    }

    return bRet;
}

void UBCFFAdaptor::UBToCFFConverter::setCFFTextFromHTMLTextNode(const QDomElement htmlTextNode,  QDomElement &iwbElement)
{

    QDomDocument textDoc;
               
    QDomElement textParentElement = iwbElement;

    QString textString;
    QDomNode htmlPNode =  htmlTextNode.firstChild();
    bool bTbreak = false;

    // reads HTML text strings - each string placed in separate <p> section 
    while(!htmlPNode.isNull())
    {  
        // add <tbreak> for split strings
        if (bTbreak)
        {
            bTbreak = false;
            
            QDomElement tbreakNode = textDoc.createElementNS(svgIWBNS, svgIWBNSPrefix+":"+tIWBTbreak);
            textParentElement.appendChild(tbreakNode.cloneNode(true));
        }

        QDomNode spanNode = htmlPNode.firstChild();

        while (!spanNode.isNull())
        {
            if (spanNode.isText())
            {
                QDomText nodeText = textDoc.createTextNode(spanNode.nodeValue());
                textParentElement.appendChild(nodeText.cloneNode(true));
            }
            else
            if (spanNode.isElement())
            {      
                QDomElement pElementIwb;
                QDomElement spanElement = textDoc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tIWBTspan);
                setCommonAttributesFromUBZ(htmlPNode.toElement(), pElementIwb, spanElement);

                if (spanNode.hasAttributes())
                {       
                    int attrCount = spanNode.attributes().count();
                    if (0 < attrCount)
                    {
                        for (int i = 0; i < attrCount; i++)
                        {
                            // html attributes like: style="font-size:40pt; color:"red";".
                            QStringList cffAttributes = spanNode.attributes().item(i).nodeValue().split(";", QString::SkipEmptyParts);
                            {
                                for (int i = 0; i < cffAttributes.count(); i++)
                                {                       
                                    QString attr = cffAttributes.at(i).trimmed();
                                    QStringList AttrVal = attr.split(":", QString::SkipEmptyParts);
                                    if(1 < AttrVal.count())
                                    {    
                                        QString sAttr = ubzAttrNameToCFFAttrName(AttrVal.at(0));
                                        if (itIsSVGElementAttribute(spanElement.tagName(), sAttr))
                                            spanElement.setAttribute(sAttr, ubzAttrValueToCFFAttrName(AttrVal.at(1)));
                                    }
                                }
                            }
                        }
                    }
                }
                QDomText nodeText = textDoc.createTextNode(spanNode.firstChild().nodeValue());                    
                spanElement.appendChild(nodeText);
                textParentElement.appendChild(spanElement.cloneNode(true));
            }
            spanNode = spanNode.nextSibling();
        }

    bTbreak = true;
    htmlPNode = htmlPNode.nextSibling();
    }
}

QString UBCFFAdaptor::UBToCFFConverter::ubzAttrNameToCFFAttrName(QString cffAttrName)
{
    QString sRet = cffAttrName;
    if (QString("color") == cffAttrName)
        sRet = QString("fill");
    if (QString("align") == cffAttrName)
        sRet = QString("text-align");

    return sRet;
}
QString UBCFFAdaptor::UBToCFFConverter::ubzAttrValueToCFFAttrName(QString cffValue)
{
    QString sRet = cffValue;
    if (QString("text") == cffValue)
        sRet = QString("normal");

    return sRet;
}

bool UBCFFAdaptor::UBToCFFConverter::setCFFAttribute(const QString &attributeName, const QString &attributeValue, const QDomElement &ubzElement, QDomElement &iwbElement,  QDomElement &svgElement)
{  
    bool bRet = true;
    bool bNeedsIWBSection = false;
    
    if (itIsIWBAttribute(attributeName))
    {
        if (!((aBackground == attributeName) && (avFalse == attributeValue)))
        {
            iwbElement.setAttribute(attributeName, attributeValue);
            bNeedsIWBSection = true;
        }
    }
    else
    if (itIsUBZAttributeToConvert(attributeName))
    {
        if (aTransform == attributeName)
        {
            setGeometryFromUBZ(ubzElement, svgElement);
        }
        else
            if (attributeName.contains(aUBZUuid))
            {

                QString parentId = ubzElement.attribute(aUBZParent);
                QString id;
                if (!parentId.isEmpty())
                    id = "{" + parentId + "}" + "{" + ubzElement.attribute(aUBZUuid)+"}";
                else
                    id = "{" + ubzElement.attribute(aUBZUuid)+"}";

                svgElement.setAttribute(aID, id);
            }
        else 
            if (attributeName.contains(aUBZHref)||attributeName.contains(aSrc))
            {
                bRet &= setContentFromUBZ(ubzElement, svgElement);
                bNeedsIWBSection = bRet||bNeedsIWBSection;
            }
    }
    else
    if (itIsSVGElementAttribute(svgElement.tagName(),attributeName))
    {
            svgElement.setAttribute(attributeName,  attributeValue);
    }

    if (bNeedsIWBSection)  
    {
        if (0 < iwbElement.attributes().count())
        {

            QStringList tl = ubzElement.attribute(aSVGHref).split("/");
            QString id = tl.at(tl.count()-1);
            // if element already have an ID, we use it. Else we create new id for element.
            if (QString() == id)
                id = QUuid::createUuid().toString();

            svgElement.setAttribute(aID, id);  
            iwbElement.setAttribute(aRef, id);
        }
    }

    return bRet;
}

bool UBCFFAdaptor::UBToCFFConverter::setCommonAttributesFromUBZ(const QDomElement &ubzElement, QDomElement &iwbElement,  QDomElement &svgElement)
{    
    bool bRet = true;

    for (int i = 0; i < ubzElement.attributes().count(); i++)
    {
        QDomNode attribute = ubzElement.attributes().item(i);
        QString attributeName = ubzAttrNameToCFFAttrName(attribute.nodeName().remove("ub:"));

        bRet &= setCFFAttribute(attributeName, ubzAttrValueToCFFAttrName(attribute.nodeValue()), ubzElement, iwbElement, svgElement);
        if (!bRet) break;
    }
    return bRet;
}

void UBCFFAdaptor::UBToCFFConverter::setViewBox(QRect viewbox)
{
    mViewbox |= viewbox;
}

QDomNode UBCFFAdaptor::UBToCFFConverter::findTextNode(const QDomNode &node)
{
    QDomNode iterNode = node;

    while (!iterNode.isNull())
    {       
        if (iterNode.isText())
        {   
            if (!iterNode.isNull())              
                return iterNode;
        }
        else 
        {
            if (!iterNode.firstChild().isNull())
            {   
                QDomNode foundNode = findTextNode(iterNode.firstChild());
                if (!foundNode.isNull())
                    if (foundNode.isText())
                        return foundNode;
            }
        }
        if (!iterNode.nextSibling().isNull())
            iterNode = iterNode.nextSibling();
        else 
            break;
    }
    return iterNode;
}

QDomNode UBCFFAdaptor::UBToCFFConverter::findNodeByTagName(const QDomNode &node, QString tagName)
{
    QDomNode iterNode = node;

    while (!iterNode.isNull())
    {       
        QString t = iterNode.toElement().tagName();
        if (tagName == t)
            return iterNode;
        else 
        {
            if (!iterNode.firstChildElement().isNull())
            {
                QDomNode foundNode = findNodeByTagName(iterNode.firstChildElement(), tagName);
                if (!foundNode.isNull()){
                    if (foundNode.isElement())
                    {
                        if (tagName == foundNode.toElement().tagName())
                            return foundNode;
                    }
                    else
                        break;
                }
            }
        }
        
        if (!iterNode.nextSibling().isNull())
            iterNode = iterNode.nextSibling();
        else 
            break;
    }
    return QDomNode();

}

bool UBCFFAdaptor::UBToCFFConverter::createBackground(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{
    qDebug() << "|creating element background";


    QDomDocument doc;

    //QDomElement svgBackgroundElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tUBZImage);
    QDomElement svgBackgroundElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tIWBRect);
    QDomElement iwbBackgroundElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);


    QRect bckRect(mViewbox);

    if (0 <= mViewbox.topLeft().x())
        bckRect.topLeft().setX(0);

    if (0 <= mViewbox.topLeft().y())
        bckRect.topLeft().setY(0);

    if (QRect() != bckRect)
    {     
        QString sElementID = QUuid::createUuid().toString();

        bool darkBackground = (avTrue == element.attribute(aDarkBackground));    
        svgBackgroundElementPart.setAttribute(aFill, darkBackground ? "black" : "white");
        svgBackgroundElementPart.setAttribute(aID, sElementID);
        svgBackgroundElementPart.setAttribute(aX, bckRect.x());
        svgBackgroundElementPart.setAttribute(aY, bckRect.y());
        svgBackgroundElementPart.setAttribute(aHeight, bckRect.height());
        svgBackgroundElementPart.setAttribute(aWidth, bckRect.width());

        //svgBackgroundElementPart.setAttribute(aSVGHref, backgroundImagePath);

        iwbBackgroundElementPart.setAttribute(aRef, sElementID);
        iwbBackgroundElementPart.setAttribute(aBackground, avTrue);
        //iwbBackgroundElementPart.setAttribute(aLocked, avTrue);

        addSVGElementToResultModel(svgBackgroundElementPart, dstSvgList, DEFAULT_BACKGROUND_LAYER);
        addIWBElementToResultModel(iwbBackgroundElementPart);
        return true;
    }
    else
    {
        qDebug() << "|error at creating element background";
        errorStr = "CreatingElementBackgroundParsingError.";
        return false;
    }
}

QString UBCFFAdaptor::UBToCFFConverter::createBackgroundImage(const QDomElement &element, QSize size)
{
    QString sRet;

    QString sDstFileName(fIWBBackground);

    bool bDirExists = true;
    QDir dstDocFolder(destinationPath);

    if (!dstDocFolder.exists(cfImages))
        bDirExists &= dstDocFolder.mkdir(cfImages);

    QString dstFilePath;
    if (bDirExists)
        dstFilePath = destinationPath+"/"+cfImages+"/"+sDstFileName;

    if (!QFile().exists(dstFilePath))
    {
        QRect rect(0,0, size.width(), size.height());

        QImage *bckImage = new QImage(size, QImage::Format_RGB888);

        QPainter *painter = new QPainter(bckImage);   

        bool darkBackground = (avTrue == element.attribute(aDarkBackground));
     
        QColor bCrossColor;

        bCrossColor = darkBackground?QColor(Qt::white):QColor(Qt::blue);
        int penAlpha = (int)(255/2); // default Sankore value for transform.m11 < 1
        bCrossColor.setAlpha(penAlpha);
        painter->setPen(bCrossColor);
        painter->setBrush(darkBackground?QColor(Qt::black):QColor(Qt::white));

        painter->drawRect(rect);

        if (avTrue == element.attribute(aCrossedBackground))
        {    
            qreal firstY = ((int) (rect.y () / iCrossSize)) * iCrossSize;

            for (qreal yPos = firstY; yPos <= rect.y () + rect.height (); yPos += iCrossSize)
            {
                painter->drawLine (rect.x (), yPos, rect.x () + rect.width (), yPos);
            }

            qreal firstX = ((int) (rect.x () / iCrossSize)) * iCrossSize;

            for (qreal xPos = firstX; xPos <= rect.x () + rect.width (); xPos += iCrossSize)
            {
                painter->drawLine (xPos, rect.y (), xPos, rect.y () + rect.height ());
            }
        }
        
        painter->end();
        painter->save();
        
        if (QString() != dstFilePath)
           if (bckImage->save(dstFilePath))
               sRet = cfImages+"/"+sDstFileName;

        delete bckImage;
        delete painter;
    }
    else 
        sRet = cfImages+"/"+sDstFileName;

    return sRet;
}

bool UBCFFAdaptor::UBToCFFConverter::createPngFromSvg(QString &svgPath, QString &dstPath, QTransform transformation, QSize size)
{
    if (QFile().exists(svgPath))
    {
        QImage i(svgPath);

        QSize iSize = (QSize() == size)?QSize(i.size().width()*transformation.m11(), i.size().height()*transformation.m22()):size;

        QImage image(iSize, QImage::Format_ARGB32_Premultiplied);        
        image.fill(0);
        QPainter imagePainter(&image);
        QSvgRenderer renderer(svgPath);  
        renderer.render(&imagePainter);     
      
        return image.save(dstPath);

    }
    else 
        return false;
}


bool UBCFFAdaptor::UBToCFFConverter::parseSVGGGroup(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{
    qDebug() << "|parsing g section";
    QDomElement nextElement = element.firstChildElement();
    if (nextElement.isNull()) {
        qDebug() << "Empty g element";
        errorStr = "EmptyGSection";
        return false;
    }

    QMultiMap<int, QDomElement> svgElements;

    QDomDocument doc;
    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tIWBG);
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);
    
    // Elements can know about its layer, so it must add result QDomElements to ordrered list.
    while (!nextElement.isNull()) {
        QString tagName = nextElement.tagName();
        if      (tagName == tUBZLine)       parseUBZLine(nextElement, svgElements);
        else if (tagName == tUBZPolygon)    parseUBZPolygon(nextElement, svgElements);
        else if (tagName == tUBZPolyline)   parseUBZPolyline(nextElement, svgElements);

        nextElement = nextElement.nextSiblingElement();
    }

    QList<int> layers;
    QMapIterator<int, QDomElement> nextSVGElement(svgElements);
    while (nextSVGElement.hasNext()) 
        layers << nextSVGElement.next().key();

    qSort(layers);
    int layer = layers.at(0);

    nextSVGElement.toFront();
    while (nextSVGElement.hasNext()) 
        svgElementPart.appendChild(nextSVGElement.next().value());

    addSVGElementToResultModel(svgElementPart, dstSvgList, layer);
 
    return true;
}
bool UBCFFAdaptor::UBToCFFConverter::parseUBZImage(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{
    qDebug() << "|parsing image";

    QDomDocument doc;

    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + getElementTypeFromUBZ(element));
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);

    if (setCommonAttributesFromUBZ(element, iwbElementPart, svgElementPart))
    {
        addSVGElementToResultModel(svgElementPart, dstSvgList, getElementLayer(element));
       
        if (0 < iwbElementPart.attributes().count())
            addIWBElementToResultModel(iwbElementPart);
        return true;
    }
    else
    {
        qDebug() << "|error at image parsing";
        errorStr = "ImageParsingError";
        return false;

    }
}

bool UBCFFAdaptor::UBToCFFConverter::parseUBZVideo(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{
    qDebug() << "|parsing video";

    QDomDocument doc;

    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + getElementTypeFromUBZ(element));
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);

    if (setCommonAttributesFromUBZ(element, iwbElementPart, svgElementPart))
    {
        QDomElement svgSwitchSection = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tIWBSwitch);
        svgSwitchSection.appendChild(svgElementPart);

        // if viewer cannot open that content - it must use that:
        QDomElement svgText = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tIWBTextArea);
        svgText.setAttribute(aX, svgElementPart.attribute(aX));
        svgText.setAttribute(aY, svgElementPart.attribute(aY));
        svgText.setAttribute(aWidth, svgElementPart.attribute(aWidth));
        svgText.setAttribute(aHeight, svgElementPart.attribute(aHeight));
        svgText.setAttribute(aTransform, svgElementPart.attribute(aTransform));

        QDomText text = doc.createTextNode("Cannot Open Content");  
        svgText.appendChild(text);

        svgSwitchSection.appendChild(svgText);

        addSVGElementToResultModel(svgSwitchSection, dstSvgList, getElementLayer(element));

        if (0 < iwbElementPart.attributes().count())
            addIWBElementToResultModel(iwbElementPart);  
        return true;
    }
    else
    {
        qDebug() << "|error at video parsing";
        errorStr = "VideoParsingError";
        return false;
    }
}

bool UBCFFAdaptor::UBToCFFConverter::parseUBZAudio(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{
    qDebug() << "|parsing audio";

    // audio file must be linked to cff item excluding video.
    // to do:
    // 1 add image for audio element.
    // 2 set id for this element
    // 3 add <svg:a> section with xlink:href to audio file
    // 4 add shild to a section with id of the image

    QDomDocument doc;

    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + getElementTypeFromUBZ(element));
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);

    if (setCommonAttributesFromUBZ(element, iwbElementPart, svgElementPart))
    {
        //we must create image-containers for audio files
        int audioImageDimention = qMin(svgElementPart.attribute(aWidth).toInt(), svgElementPart.attribute(aHeight).toInt());
        QString srcAudioImageFile(sAudioElementImage);
        QString elementId = QString(QUuid::createUuid().toString());
        QString sDstAudioImageFileName = elementId+"."+fePng;
        QString dstAudioImageFilePath = destinationPath+"/"+cfImages+"/"+sDstAudioImageFileName;
        QString dstAudioImageRelativePath = cfImages+"/"+sDstAudioImageFileName;

        QFile srcFile(srcAudioImageFile);

        //creating folder for audioImage
        QDir dstDocFolder(destinationPath);
        bool bRes = true;
        if (!dstDocFolder.exists(cfImages))
            bRes &= dstDocFolder.mkdir(cfImages);
        
        // CFF cannot show SVG images, so we need to convert it to png.
        if (bRes && createPngFromSvg(srcAudioImageFile, dstAudioImageFilePath, getTransformFromUBZ(element), QSize(audioImageDimention, audioImageDimention)))
        {
            // switch section disabled because of imcompatibility with validator http://validator.imsglobal.org/iwb/index.jsp?validate=package
            // QDomElement svgSwitchSection = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tIWBSwitch);

            // first we place content
            QDomElement svgASection = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tIWBA);
            svgASection.setAttribute(aSVGHref, svgElementPart.attribute(aSVGHref));
        
            svgElementPart.setTagName(tIWBImage);
            svgElementPart.setAttribute(aSVGHref, dstAudioImageRelativePath); 
            svgElementPart.setAttribute(aHeight, audioImageDimention);
            svgElementPart.setAttribute(aWidth, audioImageDimention);

            svgASection.appendChild(svgElementPart);
            // switch section disabled because of imcompatibility with validator http://validator.imsglobal.org/iwb/index.jsp?validate=package
            // svgSwitchSection.appendChild(svgASection);

            // if viewer cannot open that content - it must use that:
            QDomElement svgText = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + tIWBTextArea);
            svgText.setAttribute(aX, svgElementPart.attribute(aX));
            svgText.setAttribute(aY, svgElementPart.attribute(aY));
            svgText.setAttribute(aWidth, svgElementPart.attribute(aWidth));
            svgText.setAttribute(aHeight, svgElementPart.attribute(aHeight));
            svgText.setAttribute(aTransform, svgElementPart.attribute(aTransform));

            QDomText text = doc.createTextNode("Cannot Open Content");  
            svgText.appendChild(text);

            // switch section disabled because of imcompatibility with validator http://validator.imsglobal.org/iwb/index.jsp?validate=package          
            // svgSwitchSection.appendChild(svgText);

            // switch section disabled because of imcompatibility with validator http://validator.imsglobal.org/iwb/index.jsp?validate=package
            addSVGElementToResultModel(svgASection/*svgSwitchSection*/, dstSvgList, getElementLayer(element));

            if (0 < iwbElementPart.attributes().count())
                addIWBElementToResultModel(iwbElementPart);
            return true;
        }
        return false;
    }
    else
    {
        qDebug() << "|error at audio parsing";
        errorStr = "AudioParsingError";
        return false;
    }
}

bool UBCFFAdaptor::UBToCFFConverter::parseForeignObject(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{

    if (element.attribute(aUBZType) == avUBZText) {
        return parseUBZText(element, dstSvgList);
    }

    qDebug() << "|parsing foreign object";

    QDomDocument doc;

    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + getElementTypeFromUBZ(element));
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);

    if (setCommonAttributesFromUBZ(element, iwbElementPart, svgElementPart))
    {
        addSVGElementToResultModel(svgElementPart, dstSvgList, getElementLayer(element));
        if (0 < iwbElementPart.attributes().count())
            addIWBElementToResultModel(iwbElementPart);
        return true;
    }
    else
    {
        qDebug() << "|error at parsing foreign object";
        errorStr = "ForeignObjectParsingError";
        return false;
    }
}

bool UBCFFAdaptor::UBToCFFConverter::parseUBZText(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{
    qDebug() << "|parsing text";

    QDomDocument doc;

    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + getElementTypeFromUBZ(element));
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);
    
    if (element.hasChildNodes())
    {
        QDomDocument htmlDoc;
        htmlDoc.setContent(findTextNode(element).nodeValue());
        QDomNode bodyNode = findNodeByTagName(htmlDoc.firstChildElement(), "body");

        setCFFTextFromHTMLTextNode(bodyNode.toElement(), svgElementPart);

        if (setCommonAttributesFromUBZ(element, iwbElementPart, svgElementPart))
        {
            QString commonParams;
            for (int i = 0; i < bodyNode.attributes().count(); i++)
            {
                commonParams += " " + bodyNode.attributes().item(i).nodeValue();
            }
            commonParams.remove(" ");
            commonParams.remove("'");

            QStringList commonAttributes = commonParams.split(";", QString::SkipEmptyParts);
            for (int i = 0; i < commonAttributes.count(); i++)
            {
                QStringList AttrVal = commonAttributes.at(i).split(":", QString::SkipEmptyParts);
                if (1 < AttrVal.count())
                {                
                    QString sAttr = ubzAttrNameToCFFAttrName(AttrVal.at(0));
                    QString sVal  = ubzAttrValueToCFFAttrName(AttrVal.at(1));

                    setCFFAttribute(sAttr, sVal, element, iwbElementPart, svgElementPart);
                }
            }    
            addSVGElementToResultModel(svgElementPart, dstSvgList, getElementLayer(element));
            if (0 < iwbElementPart.attributes().count())
                addIWBElementToResultModel(iwbElementPart);
            return true;
        }  
        return false;
    }
    else
    {
        qDebug() << "|error at text parsing";
        errorStr = "TextParsingError";
        return false;
    }
}

bool UBCFFAdaptor::UBToCFFConverter::parseUBZPolygon(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{
    qDebug() << "||parsing polygon";

    QDomDocument doc;

    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + getElementTypeFromUBZ(element));
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);

    if (setCommonAttributesFromUBZ(element, iwbElementPart, svgElementPart))
    {
        svgElementPart.setAttribute(aStroke, svgElementPart.attribute(aFill));
        addSVGElementToResultModel(svgElementPart, dstSvgList, getElementLayer(element));

        if (0 < iwbElementPart.attributes().count())
        {   
            QString id = svgElementPart.attribute(aUBZUuid);
            if (id.isEmpty())
                id = QUuid::createUuid().toString();

            svgElementPart.setAttribute(aID, id);
            iwbElementPart.setAttribute(aRef, id);     

            addIWBElementToResultModel(iwbElementPart);
        }
        return true;
    }
    else
    {
        qDebug() << "||error at parsing polygon";
        errorStr = "PolygonParsingError";
        return false;
    }

}

bool UBCFFAdaptor::UBToCFFConverter::parseUBZPolyline(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{
    qDebug() << "||parsing polyline";
    QDomElement resElement;

    QDomDocument doc;

    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + getElementTypeFromUBZ(element));
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);

    if (setCommonAttributesFromUBZ(element, iwbElementPart, svgElementPart))
    {
        svgElementPart.setAttribute(aStroke, svgElementPart.attribute(aFill));
        addSVGElementToResultModel(svgElementPart, dstSvgList, getElementLayer(element));

        if (0 < iwbElementPart.attributes().count())
        {
            QString id = QUuid::createUuid().toString();
            svgElementPart.setAttribute(aID, id);
            iwbElementPart.setAttribute(aRef, id);

            addIWBElementToResultModel(iwbElementPart);
        }
        return true;
    }
    else
    {
        qDebug() << "||error at parsing polygon";
        errorStr = "PolylineParsingError";
        return false;
    }
    
}

bool UBCFFAdaptor::UBToCFFConverter::parseUBZLine(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList)
{   
    qDebug() << "||parsing line";
    QDomElement resElement;
    QDomDocument doc;

    QDomElement svgElementPart = doc.createElementNS(svgIWBNS,svgIWBNSPrefix + ":" + getElementTypeFromUBZ(element));
    QDomElement iwbElementPart = doc.createElementNS(iwbNS,iwbNsPrefix + ":" + tElement);

    if (setCommonAttributesFromUBZ(element, iwbElementPart, svgElementPart))
    {
        svgElementPart.setAttribute(aStroke, svgElementPart.attribute(aFill));
        addSVGElementToResultModel(svgElementPart, dstSvgList, getElementLayer(element));

        if (0 < iwbElementPart.attributes().count())
        {
            QString id = QUuid::createUuid().toString();
            svgElementPart.setAttribute(aID, id);
            iwbElementPart.setAttribute(aRef, id);

            addIWBElementToResultModel(iwbElementPart);
        }      
    }
    else
    {
        qDebug() << "||error at parsing polygon";
        errorStr = "LineParsingError";
        return false;
    }
    return true;
}

void UBCFFAdaptor::UBToCFFConverter::addSVGElementToResultModel(const QDomElement &element, QMultiMap<int, QDomElement> &dstList, int layer)
{
    int elementLayer = (DEFAULT_LAYER == layer) ? DEFAULT_LAYER : layer;

    QDomElement rootElement = element.cloneNode(true).toElement();
    mDocumentToWrite->firstChildElement().appendChild(rootElement);
    dstList.insert(elementLayer, rootElement);
}

void UBCFFAdaptor::UBToCFFConverter::addIWBElementToResultModel(const QDomElement &element)
{
    QDomElement rootElement = element.cloneNode(true).toElement();
    mDocumentToWrite->firstChildElement().appendChild(rootElement);
    mExtendedElements.append(rootElement);
}

UBCFFAdaptor::UBToCFFConverter::~UBToCFFConverter()
{
    if (mDataModel)
        delete mDataModel;
    if (mIWBContentWriter)
        delete mIWBContentWriter;
    if (mDocumentToWrite)
        delete mDocumentToWrite;
}
bool UBCFFAdaptor::UBToCFFConverter::isValid() const
{
    bool result = QFileInfo(sourcePath).exists()
               && QFileInfo(sourcePath).isDir()
               && errorStr == noErrorMsg;

    if (!result) {
        qDebug() << "specified data is not valid";
        errorStr = "ValidateDataError";
    }

    return result;
}

void UBCFFAdaptor::UBToCFFConverter::fillNamespaces()
{
    mIWBContentWriter->writeDefaultNamespace(svgUBZNS);
    mIWBContentWriter->writeNamespace(iwbNS, iwbNsPrefix);
    mIWBContentWriter->writeNamespace(svgIWBNS, svgIWBNSPrefix);
    mIWBContentWriter->writeNamespace(xlinkNS, xlinkNSPrefix);
}

QString UBCFFAdaptor::UBToCFFConverter::digitFileFormat(int digit) const
{
    return QString("%1").arg(digit, 3, 10, QLatin1Char('0'));
}
QString UBCFFAdaptor::UBToCFFConverter::contentIWBFileName() const
{
    return destinationPath + "/" + fIWBContent;
}

//setting SVG dimenitons
QSize UBCFFAdaptor::UBToCFFConverter::getSVGDimentions(const QString &element)
{

    QStringList dimList;

    dimList = element.split(dimensionsDelimiter1, QString::KeepEmptyParts);
    if (dimList.count() != 2) // row unlike 0x0
        return QSize();

    bool ok;

    int width = dimList.takeFirst().toInt(&ok);
    if (!ok || !width)
        return QSize();

    int height = dimList.takeFirst().toInt(&ok);
    if (!ok || !height)
        return QSize();

    return QSize(width, height);
}

//Setting viewbox rectangle
QRect UBCFFAdaptor::UBToCFFConverter::getViewboxRect(const QString &element) const
{
    QStringList dimList;

    dimList = element.split(dimensionsDelimiter2, QString::KeepEmptyParts);
    if (dimList.count() != 4) // row unlike 0 0 0 0
        return QRect();
    
    bool ok = false;

    int x = dimList.takeFirst().toInt(&ok);
    if (!ok || !x)
        return QRect();

    int y = dimList.takeFirst().toInt(&ok);
    if (!ok || !y)
        return QRect();

    int width = dimList.takeFirst().toInt(&ok);
    if (!ok || !width)
        return QRect();

    int height = dimList.takeFirst().toInt(&ok);
    if (!ok || !height)
        return QRect();

    return QRect(x, y, width, height);
}

QString UBCFFAdaptor::UBToCFFConverter::rectToIWBAttr(const QRect &rect) const
{
    if (rect.isNull()) return QString();

    return QString("%1 %2 %3 %4").arg(rect.topLeft().x())
                                 .arg(rect.topLeft().y())
                                 .arg(rect.width())
                                 .arg(rect.height());
}

UBCFFAdaptor::UBToUBZConverter::UBToUBZConverter()
{

}
