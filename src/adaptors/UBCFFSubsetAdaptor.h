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
class QTransform;

struct IwbExt {
    IwbExt() {;}
    IwbExt(QDomNode element) : element(element), extAttr(*(new QVector<QDomNode>())) {;}

    QDomNode group;
    QDomNode element;
    QVector<QDomNode> extAttr;
    QHash<QString, QString> textAttributes;
    operator bool() const {return group.isNull() || !element.isNull();}
};

class UBCFFSubsetAdaptor
{
public:
    UBCFFSubsetAdaptor();

    static bool ConvertCFFFileToUbz(QString &cffSourceFile, UBDocumentProxy* pDocument);

private:
    class UBCFFSubsetReader
    {
        //xml parse states definition
        enum
        {
            NONE,
            IWB,
            SVG,
            PAGESET,
            PAGE,
            TEXTAREA,
            TSPAN
        };

    public:
        UBCFFSubsetReader(UBDocumentProxy *proxy, QByteArray &content);

        QXmlStreamReader mReader;
        UBDocumentProxy *mProxy;

        bool parse();

    private:
        QString mTempFilePath;
        UBGraphicsScene *mCurrentScene;
        QRectF mCurrentSceneRect;
        QString mIndent;
        QRectF mViewBox;
        QPointF mViewBoxCenter;
        QSize mSize;

    private:
        QDomDocument mDOMdoc;
        QHash<QString, IwbExt> extProperties;
        bool hashElements();
        void addExtentionsToHash(QDomElement *parent);

        void hashSvg(QDomNode *parent, QString prefix = "");
        void hashSiblingIwbElements(QDomElement *parent, QDomElement *topGroup = 0);


        //methods to store current xml parse state
        int PopState();
        void PushState(int state);

        //elements parsing methods
        bool parseDoc();

        bool parseCurrentElementStart();
        bool parseCurrentElementCharacters();
        bool parseCurrentElementEnd();

        bool parseIwb();
        bool parseIwbMeta();
        bool parseSvg();
        bool parseRect();
        bool parseEllipse();
        bool parseTextArea();
        bool parseText();
        bool parsePolygon();
        bool parsePolyline();
        bool parsePage();
        bool parsePageSet();
        bool parseIwbElementRef();

        bool createNewScene();
        bool persistCurrentScene();


        QStack<int> stateStack;

        int currentState;

        //helper methods
        bool getCurElementTransorm(QTransform &transform);
        void repositionSvgItem(UBGraphicsSvgItem *item, qreal width, qreal height, qreal x, qreal y, bool useTransform, QTransform &transform);
        QColor colorFromString(const QString& clrString);
        QTransform transformFromString(const QString trString);
        bool getViewBoxDimenstions(const QString& viewBox);
        QSvgGenerator* createSvgGenerator(qreal width, qreal height);
        bool getTempFileName();
        void parseTextAttributes(qreal &fontSize, QColor &fontColor,
                                 QString &fontFamily, QString &fontStretch, bool &italic,
                                 int &fontWeight, int &textAlign, QTransform &fontTransform);
    };
};

#endif // UBCFFSUBSETADAPTOR_H
