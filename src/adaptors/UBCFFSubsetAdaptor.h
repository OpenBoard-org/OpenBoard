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

class UBDocumentProxy;
class UBGraphicsScene;
class QSvgGenerator;
class UBGraphicsSvgItem;
class QTransform;

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
