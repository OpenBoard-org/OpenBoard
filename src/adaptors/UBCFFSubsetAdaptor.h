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

    private:
        QDomDocument mDOMdoc;
        QDomNode mCurrentDOMElement;
        QHash<QString, UBGraphicsItem*> persistedItems;
        QDir mTmpFlashDir;

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
        inline bool parseIwbGroup(QDomElement &parent);
        inline bool parseIwbElement(QDomElement &element);
        inline void parseTSpan(const QDomElement &parent, QPainter &painter
                               , qreal &curX, qreal &curY, qreal &width, qreal &height, qreal &linespacing, QRectF &lastDrawnTextBoundingRect
                               , qreal &fontSize, QColor &fontColor, QString &fontFamily, QString &fontStretch, bool &italic
                               , int &fontWeight, int &textAlign, QTransform &fontTransform);
        inline void hashSceneItem(const QDomElement &element, UBGraphicsItem *item);

        // to kill
        void parseTextAttributes(const QDomElement &element, qreal &fontSize, QColor &fontColor,
                                 QString &fontFamily, QString &fontStretch, bool &italic,
                                 int &fontWeight, int &textAlign, QTransform &fontTransform);

        //elements parsing methods
        bool parseDoc();

        bool createNewScene();
        bool persistCurrentScene();

//        helper methods
        void repositionSvgItem(QGraphicsItem *item, qreal width, qreal height,
                               qreal x, qreal y,
                               bool useTransform, QTransform &transform);
        void experimentalReposition(QGraphicsItem *item, qreal width, qreal height,
                                    qreal x, qreal y,
                                    bool useTransform, QTransform &transform);
        QColor colorFromString(const QString& clrString);
        QTransform transformFromString(const QString trString);
        bool getViewBoxDimenstions(const QString& viewBox);
        QSvgGenerator* createSvgGenerator(qreal width, qreal height);
        bool getTempFileName();
        inline bool strToBool(QString);
        bool createTempFlashPath();
    };
};

#endif // UBCFFSUBSETADAPTOR_H
