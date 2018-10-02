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



#ifndef UBCFFSUBSETADAPTOR_H
#define UBCFFSUBSETADAPTOR_H

#include <QtXml>
#include <QString>
#include <QStack>
#include <QDomDocument>
#include <QHash>

class UBDocumentProxy;
class UBGraphicsScene;
class QSvgGenerator;
class UBGraphicsSvgItem;
class UBGraphicsPixmapItem;
class UBGraphicsItemDelegate;
class QTransform;
class QPainter;
class UBGraphicsItem;
class QGraphicsItem;
class QTextBlockFormat;
class QTextCharFormat;
class QTextCursor;
class UBGraphicsStrokesGroup;


class UBCFFSubsetAdaptor
{
public:
    UBCFFSubsetAdaptor();
    static bool ConvertCFFFileToUbz(QString &cffSourceFile, UBDocumentProxy* pDocument);

private:
    class UBCFFSubsetReader
    {
    public:
        UBCFFSubsetReader(UBDocumentProxy *proxy, QFile *content);
        ~UBCFFSubsetReader();

        UBDocumentProxy *mProxy;
        QString pwdContent;

        bool parse();

    private:
        QString mTempFilePath;
        UBGraphicsScene *mCurrentScene;
        QRectF mCurrentSceneRect;
        QString mIndent;
        QRectF mViewBox;
        QRectF mViewPort;
        qreal mVBTransFactor;
        QPointF mViewBoxCenter;
        QSize mSize;
        QPointF mShiftVector;
        bool mSvgGSectionIsOpened;
        UBGraphicsGroupContainerItem *mGSectionContainer;

    private:
        QDomDocument mDOMdoc;
        QDomNode mCurrentDOMElement;
        QHash<QString, UBGraphicsItem*> persistedItems;
        QMap<QString, QString> mRefToUuidMap;
        QDir mTmpFlashDir;

        void addItemToGSection(QGraphicsItem *item);
        bool hashElements();
        void addExtentionsToHash(QDomElement *parent, QDomElement *topGroup);

        void hashSvg(QDomNode *parent, QString prefix = "");
        void hashSiblingIwbElements(QDomElement *parent, QDomElement *topGroup = 0);

        inline void parseSvgSectionAttr(const QDomElement &);
        bool parseSvgPage(const QDomElement &parent);
        bool parseSvgPageset(const QDomElement &parent);
        bool parseSvgElement(const QDomElement &parent);
        bool parseIwbMeta(const QDomElement &element);
        bool parseSvg(const QDomElement &svgSection);

        inline bool parseGSection(const QDomElement &element);
        inline bool parseSvgSwitchSection(const QDomElement &element);
        inline bool parseSvgRect(const QDomElement &element);
        inline bool parseSvgEllipse(const QDomElement &element);
        inline bool parseSvgPolygon(const QDomElement &element);
        inline bool parseSvgPolyline(const QDomElement &element);
        inline bool parseSvgText(const QDomElement &element);
        inline bool parseSvgTextarea(const QDomElement &element);
        inline bool parseSvgImage(const QDomElement &element);
        inline bool parseSvgFlash(const QDomElement &element);
        inline bool parseSvgAudio(const QDomElement &element);
        inline bool parseSvgVideo(const QDomElement &element);
        inline UBGraphicsGroupContainerItem *parseIwbGroup(QDomElement &parent);
        inline bool parseIwbElement(QDomElement &element);
        inline void parseTSpan(const QDomElement &parent, QPainter &painter
                               , qreal &curX, qreal &curY, qreal &width, qreal &height, qreal &linespacing, QRectF &lastDrawnTextBoundingRect
                               , qreal &fontSize, QColor &fontColor, QString &fontFamily, QString &fontStretch, bool &italic
                               , int &fontWeight, int &textAlign, QTransform &fontTransform);
        inline void parseTSpan(const QDomElement &element, QTextCursor &cursor
                               , QTextBlockFormat &blockFormat, QTextCharFormat &charFormat);
        inline void hashSceneItem(const QDomElement &element, UBGraphicsItem *item);

        // to kill
        inline void parseTextAttributes(const QDomElement &element, qreal &fontSize, QColor &fontColor,
                                 QString &fontFamily, QString &fontStretch, bool &italic,
                                 int &fontWeight, int &textAlign, QTransform &fontTransform);
        inline void parseTextAttributes(const QDomElement &element, QFont &font, QColor);
        inline void readTextBlockAttr(const QDomElement &element, QTextBlockFormat &format);
        inline void readTextCharAttr(const QDomElement &element, QTextCharFormat &format);

        //elements parsing methods
        bool parseDoc();

        bool createNewScene();
        bool persistCurrentScene();
        bool persistScenes();

//        helper methods
        void repositionSvgItem(QGraphicsItem *item, qreal width, qreal height,
                               qreal x, qreal y,
                               QTransform &transform);
        QColor colorFromString(const QString& clrString);
        QTransform transformFromString(const QString trString, QGraphicsItem *item = 0);
        bool getViewBoxDimenstions(const QString& viewBox);
        QSvgGenerator* createSvgGenerator(qreal width, qreal height);
        bool getTempFileName();
        inline bool strToBool(QString);
        bool createTempFlashPath();
    };
};

#endif // UBCFFSUBSETADAPTOR_H
