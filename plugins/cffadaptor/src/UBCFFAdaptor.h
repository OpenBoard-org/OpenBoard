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


#ifndef UBCFFADAPTOR_H
#define UBCFFADAPTOR_H

#include "UBCFFAdaptor_global.h"

#include <QtCore>

class QTransform;
class QDomDocument;
class QDomElement;
class QDomNode;
class QuaZipFile;

class UBCFFADAPTORSHARED_EXPORT UBCFFAdaptor {
    class UBToCFFConverter;

public:
    UBCFFAdaptor();
    ~UBCFFAdaptor();

    bool convertUBZToIWB(const QString &from, const QString &to);
    bool deleteDir(const QString& pDirPath) const;
    QList<QString> getConversionMessages();

private:
    QString uncompressZip(const QString &zipFile);
    bool compressZip(const QString &source, const QString &destination);
    bool compressDir(const QString &dirName, const QString &parentDir, QuaZipFile *outZip);
    bool compressFile(const QString &fileName, const QString &parentDir, QuaZipFile *outZip);

    QString createNewTmpDir();
    bool freeDir(const QString &dir);
    void freeTmpDirs();

private:
    QStringList tmpDirs;
    QList<QString> mConversionMessages;

private:

    class UBToCFFConverter {

       static const int DEFAULT_LAYER = -100000;

    public:
        UBToCFFConverter(const QString &source, const QString &destination);
        ~UBToCFFConverter();
        bool isValid() const;
        QString lastErrStr() const {return errorStr;}
        bool parse();
        QList<QString> getMessages() {return mExportErrorList;}

    private:

        void addLastExportError(QString error) {mExportErrorList.append(error);}

        void fillNamespaces();

        bool parseMetadata();
        bool parseContent();
        QDomElement parsePageset(const QStringList &pageFileNames);
        QDomElement parsePage(const QString &pageFileName);
        QDomElement parseSvgPageSection(const QDomElement &element);
        void writeQDomElementToXML(const QDomNode &node);
        bool writeExtendedIwbSection();
        QDomElement parseGroupsPageSection(const QDomElement &groupRoot);

        bool createBackground(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);
        QString createBackgroundImage(const QDomElement &element, QSize size);
        bool createPngFromSvg(QString &svgPath, QString &dstPath,  QTransform transformation, QSize size = QSize());

        bool parseSVGGGroup(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);
        bool parseUBZImage(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);
        bool parseUBZVideo(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);
        bool parseUBZAudio(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);
        bool parseForeignObject(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);
        bool parseUBZText(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);

        bool parseUBZPolygon(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);
        bool parseUBZPolyline(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);
        bool parseUBZLine(const QDomElement &element, QMultiMap<int, QDomElement> &dstSvgList);       
        void addSVGElementToResultModel(const QDomElement &element, QMultiMap<int, QDomElement> &dstList, int layer = DEFAULT_LAYER);
        void addIWBElementToResultModel(const QDomElement &element);

        qreal getAngleFromTransform(const QTransform &tr);
        QString getDstContentFolderName(const QString &elementType);
        QString getSrcContentFolderName(QString href);
        QString getFileNameFromPath(QString sPath);
        QString getExtentionFromFileName(const QString &filename);
        QString convertExtention(const QString &ext);
        QString getElementTypeFromUBZ(const QDomElement &element);

        int getElementLayer(const QDomElement &element);

        bool itIsSupportedFormat(const QString &format) const;
        bool itIsFormatToConvert(const QString &format) const;
        bool itIsSVGElementAttribute(const QString ItemType, const QString &AttrName);
        bool itIsIWBAttribute(const QString &attribute) const;
        bool itIsUBZAttributeToConvert(const QString &attribute) const;

        bool ibwAddLine(int x1, int y1, int x2, int y2, QString color=QString(), int width=1, bool isBackground=false);

        QTransform getTransformFromUBZ(const QDomElement &ubzElement);
        void setGeometryFromUBZ(const QDomElement &ubzElement, QDomElement &iwbElement);
        void setCoordinatesFromUBZ(const QDomElement &ubzElement, QDomElement &iwbElement);
        bool setContentFromUBZ(const QDomElement &ubzElement, QDomElement &svgElement);
        void setCFFTextFromUBZ(const QDomElement &ubzElement, QDomElement &iwbElement, QDomElement &svgElement);
        void setCFFTextFromHTMLTextNode(const QDomElement htmlTextNode, QDomElement &iwbElement);
        QString ubzAttrNameToCFFAttrName(QString cffAttrName);
        QString ubzAttrValueToCFFAttrName(QString cffAttrValue);

        bool setCFFAttribute(const QString &attributeName, const QString &attributeValue, const QDomElement &ubzElement, QDomElement &iwbElement,  QDomElement &svgElement);
        bool setCommonAttributesFromUBZ(const QDomElement &ubzElement, QDomElement &iwbElement,  QDomElement &svgElement);
        void setViewBox(QRect viewbox);

        QDomNode findTextNode(const QDomNode &node);
        QDomNode findNodeByTagName(const QDomNode &node, QString tagName);

        QSize getSVGDimentions(const QString &element);

        inline QRect getViewboxRect(const QString &element) const;
        inline QString rectToIWBAttr(const QRect &rect) const;
        inline QString digitFileFormat(int num) const;
        inline bool strToBool(const QString &in) const {return in == "true";}
        QString contentIWBFileName() const;

    private:
        QList<QString> mExportErrorList;
        QMap<QString, QString> iwbSVGItemsAttributes;
        QDomDocument *mDataModel; //model for reading indata
        QXmlStreamWriter *mIWBContentWriter; //stream to write outdata
        QSize mSVGSize; //svg page size
        QRect mViewbox; //Main viewbox parameter for CFF
        QString sourcePath; // dir with unpacked source data (ubz)
        QString destinationPath; //dir with unpacked destination data (iwb)
        QDomDocument *mDocumentToWrite; //document for saved QDomElements from mSvgElements and mExtendedElements
        QMultiMap<int, QDomElement> mSvgElements; //Saving svg elements to have a sorted by z order list of elements to write;
        QList<QDomElement> mExtendedElements; //Saving extended options of elements to be able to add them to the end of result iwb document;
        mutable QString errorStr; // last error string message

    public:
        operator bool() const {return isValid();}
    };

    class UBToUBZConverter {
    public:
        UBToUBZConverter();
    };


};

#endif // UBCFFADAPTOR_H
