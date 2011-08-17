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

#include <QRegExp>
#include <QSvgGenerator>
#include <QSvgRenderer>


#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBItem.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStroke.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsSvgItem.h"

#include "UBCFFSubsetAdaptor.h"
#include "UBMetadataDcSubsetAdaptor.h"
#include "UBThumbnailAdaptor.h"
#include "UBSvgSubsetAdaptor.h"

//enum of xmlparse status

//tag names definition
//use them everiwhere!
static QString tElement         = "element";
static QString tEllipse         = "ellipse";
static QString tIwb             = "iwb";
static QString tMeta            = "meta";
static QString tPage            = "page";
static QString tPageset         = "pageset";
static QString tPolygon         = "polygon";
static QString tRect            = "rect";
static QString tSvg             = "svg";
static QString tText            = "text";
static QString tTextarea        = "textarea";
static QString tTspan           = "tspan";
static QString tBreak           = "tbreak";

//attribute names definition
static QString aFill            = "fill";
static QString aFillopacity     = "fill-opacity";
static QString aX               = "x";
static QString aY               = "y";
static QString aWidth           = "width";
static QString aHeight          = "height";
static QString aStroke          = "stroke";
static QString aStrokewidth     = "stroke-width";
static QString aCx              = "cx";
static QString aCy              = "cy";
static QString aRx              = "rx";
static QString aRy              = "ry";
static QString aTransform       = "transform";
static QString aViewbox         = "viewbox";
static QString aFontSize        = "font-size";
static QString aFontfamily      = "font-family";
static QString aFontstretch     = "font-stretch";
static QString aFontstyle       = "font-style";
static QString aFontweight      = "font-weight";
static QString aTextalign       = "text-align";


UBCFFSubsetAdaptor::UBCFFSubsetAdaptor()
{
}

bool UBCFFSubsetAdaptor::ConvertCFFFileToUbz(QString &cffSourceFile, UBDocumentProxy* pDocument)
{
    //TODO
    // fill document proxy metadata
    // create persistance manager to save data using proxy
    // create UBCFFSubsetReader and make it parse cffSourceFolder
    QFile file(cffSourceFile);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file " << cffSourceFile << " for reading ...";
        return false;
    }

    QByteArray data = file.readAll();
    if (data.length() == 0)
    {
        qWarning() << "Either content file " << cffSourceFile << " is empty or failed to read from file";
        file.close();
        return false;
    }

    UBCFFSubsetReader cffReader(pDocument, data);
    bool result =  cffReader.parse();
    file.close();

    return result;
}

UBCFFSubsetAdaptor::UBCFFSubsetReader::UBCFFSubsetReader(UBDocumentProxy *proxy, QByteArray &content):
    mReader(content), mProxy(proxy), currentState(NONE)
{
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parse()
{
    UBMetadataDcSubsetAdaptor::persist(mProxy);

    mIndent = "";
    if (!getTempFileName())
        return false;

    bool result = parseDoc();
    if (result)
        result = mProxy->pageCount() != 0;

    if (QFile::exists(mTempFilePath))
        QFile::remove(mTempFilePath);

    return result;
}

int UBCFFSubsetAdaptor::UBCFFSubsetReader::PopState()
{
    if (stateStack.count() == 0)
        currentState = NONE;
    else
        currentState = stateStack.pop();

    return currentState;
}

void UBCFFSubsetAdaptor::UBCFFSubsetReader::PushState(int state)
{
    if (currentState != NONE)
        stateStack.push(currentState);
    currentState = state;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseDoc()
{
    while (!mReader.atEnd())
    {
        mReader.readNext();
        if (mReader.isStartElement())
        {
            if (!parseCurrentElementStart())
                return false;
        }
        else
            if (mReader.isCharacters())
            {
                if (!parseCurrentElementCharacters())
                    return false;
            }
        else
                if (mReader.isEndElement())
                {
                    if (!parseCurrentElementEnd())
                        return false;
                }
    }
    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseCurrentElementStart()
{
    QStringRef elName = mReader.name();
    QString log = QString("%1<%2>").arg(mIndent).arg(elName.toString());
    qDebug() << log;
    mIndent += " ";
    if ( elName == tIwb)
    {
        if (!parseIwb())
            return false;

        PushState(IWB);
    }
    else
    if ( elName == tMeta)
    {
        if (!parseIwbMeta())
            return false;
    }
    else
    if ( elName == tSvg)
    {
        if (!parseSvg())
            return false;

        PushState(SVG);
    }
    else
    if ( elName == tRect)
    {
        if (!parseRect())
            return false;
    }
    else
    if ( elName == tEllipse)
    {
        if (!parseEllipse())
            return false;
    }
    else
    if ( elName == tText)
    {
        if (!parseText())
            return false;
    }
    else
    if ( elName == tTextarea)
    {
        if (!parseTextArea())
            return false;
    }
    else
    if ( elName == tPolygon)
    {
        if (!parsePolygon())
            return false;
    }
    else
    if ( elName == tPage)
    {
        if (!parsePage())
            return false;

        PushState(PAGE);
    }
    else
    if ( elName == tPageset)
    {
        if (!parsePageSet())
            return false;

        PushState(PAGESET);
    }
    else
    if ( elName == tElement)
    {
        if (!parseIwbElementRef())
            return false;
    }

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseCurrentElementCharacters()
{
    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseCurrentElementEnd()
{
    QStringRef elName = mReader.name();
    mIndent.remove(0,1);
    QString log = QString("%1</%2>").arg(mIndent).arg(elName.toString());
    qDebug() << log;

    if ( elName == tIwb)
        PopState();
    else
    if ( elName == tMeta)
    {
    }
    else
    if ( elName == tSvg)
    {
        persistCurrentScene();
        PopState();
    }
    else
    if ( elName == tPage)
    {
        persistCurrentScene();
        PopState();
    }
    else
    if ( elName == tPageset)
    {
        PopState();
    }

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseIwb()
{
    if (!stateStack.empty() || currentState != NONE)
    {
        qWarning() << "iwb content parse error, unexpected iwb tag at line" << mReader.lineNumber();
        return false;
    }

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseIwbMeta()
{
    if (currentState != IWB)
    {
        qWarning() << "iwb content parse error, unexpected meta tag at line" << mReader.lineNumber();
        return false;
    }

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseSvg()
{
    if (currentState != IWB)
    {
        qWarning() << "iwb content parse error, unexpected svg tag at line" << mReader.lineNumber();
        return false;
    }

    getViewBoxDimenstions(mReader.attributes().value(aViewbox).toString());
    mSize = QSize(mReader.attributes().value(aWidth).toString().toInt(),
                  mReader.attributes().value(aHeight).toString().toInt());

    return true;
}

void UBCFFSubsetAdaptor::UBCFFSubsetReader::repositionSvgItem(UBGraphicsSvgItem *item, qreal width, qreal height, qreal x, qreal y, bool useTransform, QTransform &transform)
{
    QTransform curTrans = item->transform();
    qWarning() << QString().sprintf("Item current transform = %f 0 0 %f %f %f, position %f, %f", curTrans.m11(), curTrans.m22(), curTrans.dx(), curTrans.dy(), item->x(), item->y());
    //check if rect is rotated
    //rotate svg item itself
    QRectF itemBounds = item->boundingRect();
    //first, svg is mapped to svg item bound
    //second, svg item is mapped to scene
    //so, get svg to svg item scale and multiple by scene scale
    qreal hScale = itemBounds.width() / width * curTrans.m11();
    qreal vScale = itemBounds.height() / height * curTrans.m22();
    
    if (useTransform)
    {
        QPointF oldVector((x - transform.dx()), (y - transform.dy()));
        QTransform rTransform(transform.m11(), transform.m12(), transform.m21(), transform.m22(), 0, 0);
        QPointF newVector = rTransform.map(oldVector);
        rTransform.scale(curTrans.m11(), curTrans.m22());
        item->setTransform(QTransform(rTransform.m11(), rTransform.m12(), rTransform.m21(), rTransform.m22(), 0, 0));
        item->setPos((x - mViewBoxCenter.x() + (newVector - oldVector).x()) * hScale, (y - mViewBoxCenter.y() + (newVector - oldVector).y()) * vScale );
    }
    else
    {
        item->setPos((x - mViewBoxCenter.x()) * hScale, (y  - mViewBoxCenter.y()) * vScale);
    }

    QTransform newTrans = item->transform();
    qWarning() << QString("Item new transform = %3 0 0 %4 %1 %2, position %5, %6").arg(newTrans.dx()).arg(newTrans.dy()).arg(newTrans.m11()).arg(newTrans.m22()).arg(item->x()).arg(item->y());
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseRect()
{
    if (currentState != SVG && currentState != PAGE)
    {
        qWarning() << "iwb content parse error, unexpected rect tag at line" << mReader.lineNumber();
        return false;
    }

    //create new scene if it's not created yet (for one page document case)
    if (currentState == SVG && mCurrentScene == NULL)
        createNewScene();

    //rect lef top corner coordinates
    qreal x1 = mReader.attributes().value(aX).toString().toDouble();
    qreal y1 = mReader.attributes().value(aY).toString().toDouble();
    //rect dimensions
    qreal width = mReader.attributes().value(aWidth).toString().toDouble();
    qreal height = mReader.attributes().value(aHeight).toString().toDouble();

    //init svg generator with temp file
    QSvgGenerator *generator = createSvgGenerator(width + 10, height + 10);

    //init painter to paint to svg
    QPainter painter;
    painter.begin(generator);

    //fill rect
    if (mReader.attributes().hasAttribute(aFill))
    {
        QColor fillColor = colorFromString(mReader.attributes().value(aFill).toString());
        painter.setBrush(QBrush(fillColor));
        painter.fillRect(5, 5, width, height, fillColor);
    }

    bool hasStrokeColor = mReader.attributes().hasAttribute(aStroke);
    bool hasStrokeWidth = mReader.attributes().hasAttribute(aStrokewidth);
    if (hasStrokeColor || hasStrokeWidth)
    {
        QPen pen;
        if (hasStrokeColor)
            pen.setColor(colorFromString(mReader.attributes().value(aStroke).toString()));
        if (hasStrokeWidth)
            pen.setWidth(mReader.attributes().value(aStrokewidth).toString().toInt());
        
        painter.setPen(pen);
        painter.drawRect(5, 5, width, height);
    }

    painter.end();

    //add resulting svg file to scene
    UBGraphicsSvgItem *svgItem = mCurrentScene->addSvg(QUrl::fromLocalFile(generator->fileName()));
    QTransform transform;
    bool hastransform = getCurElementTransorm(transform);
    repositionSvgItem(svgItem, width + 10, height + 10, x1 - 5, y1 - 5, hastransform, transform);
    delete generator;

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseEllipse()
{
    if (currentState != SVG && currentState != PAGE)
    {
        qWarning() << "iwb content parse error, unexpected ellipse tag at line" << mReader.lineNumber();
        return false;
    }

    //create new scene if it's not created yet (for one page document case)
    if (currentState == SVG && mCurrentScene == NULL)
        createNewScene();

    //ellipse horisontal and vertical radius
    qreal rx = mReader.attributes().value(aRx).toString().toDouble();
    qreal ry = mReader.attributes().value(aRy).toString().toDouble();
    QSvgGenerator *generator = createSvgGenerator(rx * 2 + 10, ry * 2 + 10);

    //fill and stroke color
    QColor fillColor = colorFromString(mReader.attributes().value(aFill).toString());
    QColor strokeColor = colorFromString(mReader.attributes().value(aStroke).toString());
    int strokeWidth = mReader.attributes().value(aStrokewidth).toString().toInt();

    //ellipse center coordinates
    qreal cx = mReader.attributes().value(aCx).toString().toDouble();
    qreal cy = mReader.attributes().value(aCy).toString().toDouble();

    //init painter to paint to svg
    QPainter painter;
    painter.begin(generator);

    QPen pen(strokeColor);
    pen.setWidth(strokeWidth);
    painter.setPen(pen);
    painter.setBrush(QBrush(fillColor));

    painter.drawEllipse(5, 5, rx * 2, ry * 2);

    painter.end();


    //add resulting svg file to scene
    UBGraphicsSvgItem *svgItem = mCurrentScene->addSvg(QUrl::fromLocalFile(generator->fileName()));

    QTransform transform;
    bool hasTransform = getCurElementTransorm(transform);

    repositionSvgItem(svgItem, rx * 2 + 10, ry * 2 + 10, cx - rx - 5, cy - ry -5, hasTransform, transform);

    delete generator;

    return true;
   }

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseTextArea()
{
    if (currentState != SVG && currentState != PAGE)
    {
        qWarning() << "iwb content parse error, unexpected textarea tag at line" << mReader.lineNumber();
        return false;
    }

    //create new scene if it's not created yet (for one page document case)
    if (currentState == SVG && mCurrentScene == NULL)
        createNewScene();

    //TODO textarea node
    qreal x = mReader.attributes().value(aX).toString().toDouble();
    qreal y = mReader.attributes().value(aY).toString().toDouble();
    qreal width = mReader.attributes().value(aWidth).toString().toDouble();
    qreal height = mReader.attributes().value(aHeight).toString().toDouble();

    qWarning() << QString().sprintf("Text coordinates : %f,%f. Text size %f,%f", x, y, width, height);

    qreal fontSize = 12.0;
    QColor fontColor;
    QString fontFamily = "Arial";
    QString fontStretch = "normal";
    bool italic = false;
    int fontWeight = QFont::Normal;
    int textAlign = Qt::AlignLeft;
    QTransform fontTransform;
    parseTextAttributes(fontSize, fontColor, fontFamily, fontStretch, italic, fontWeight, textAlign, fontTransform);

    QSvgGenerator *generator = createSvgGenerator(width, height);
    QPainter painter;
    painter.begin(generator);
    painter.setFont(QFont(fontFamily, fontSize, fontWeight, italic));

    qreal curY = 0.0;
    qreal curX = 0.0;
    qreal linespacing = QFontMetricsF(painter.font()).leading();

    //remember if text area has transform
    QString transformString;
    QTransform transform;
    bool hasTransform = getCurElementTransorm(transform);

    QRectF lastDrawnTextBoundingRect;
    //parse text area tags
    while(true)
    {
        mReader.readNext();
        QStringRef elementName = mReader.name();
        if (mReader.isEndDocument())
            break;
        if (mReader.isEndElement() && elementName == tBreak)
        {
            //when tbreak appers, move down by the drawn rect height
            //TODO: line spacing is not calculated yet, additional code is required
            curY += lastDrawnTextBoundingRect.height() + linespacing;
            curX = 0.0;
            lastDrawnTextBoundingRect = QRectF(0,0,0,0);
            continue;
        }
        if (mReader.isEndElement() && elementName == tTextarea)
            break;
        if (mReader.isStartElement() && elementName == tTspan)
        {
            parseTextAttributes(fontSize, fontColor, fontFamily, fontStretch, italic, fontWeight, textAlign, fontTransform);
            painter.setFont(QFont(fontFamily, fontSize, fontWeight, italic));
            painter.setPen(fontColor);
            linespacing = QFontMetricsF(painter.font()).leading();
            continue;
        }
        if (mReader.isCharacters() || mReader.isCDATA())
        {
            QString text = mReader.text().toString().trimmed();

            //skip empty text
            if (text.length() == 0)
                continue;

            //get bounding rect to obtain desired text height
            lastDrawnTextBoundingRect = painter.boundingRect(QRectF(curX, curY, width, height - curY), textAlign|Qt::TextWordWrap, text);
            QString log = QString().sprintf(" at rect  %f, %f, %f, %f. Bounding rect is %f, %f, %f, %f", 0.0, curY, width, height - curY, lastDrawnTextBoundingRect.x(), lastDrawnTextBoundingRect.y(), lastDrawnTextBoundingRect.width(), lastDrawnTextBoundingRect.height());
            qWarning() << "Text " << text << log;
            painter.drawText(curX, curY, width, lastDrawnTextBoundingRect.height(), textAlign|Qt::TextWordWrap, text);
            curX += lastDrawnTextBoundingRect.x() + lastDrawnTextBoundingRect.width();
            
            continue;
        }
    }

    painter.end();

    //add resulting svg file to scene
    UBGraphicsSvgItem *svgItem = mCurrentScene->addSvg(QUrl::fromLocalFile(generator->fileName()));

    repositionSvgItem(svgItem, width, height, x, y, hasTransform, transform);
    delete generator;

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseText()
{
    if (currentState != SVG && currentState != PAGE)
    {
        qWarning() << "iwb content parse error, unexpected textarea tag at line" << mReader.lineNumber();
        return false;
    }

    //create new scene if it's not created yet (for one page document case)
    if (currentState == SVG && mCurrentScene == NULL)
        createNewScene();

    qreal x = mReader.attributes().value(aX).toString().toDouble();
    qreal y = mReader.attributes().value(aY).toString().toDouble();

    qreal width = 0;
    qreal height = 0;

    QList<QRectF> textRects;
    QList<QFont> textFonts;
    QList<QString> textLines;
    QList<int> textAligns;
    QList<QColor> textColors;

    qWarning() << QString().sprintf("Text coordinates : %f,%f. Text size %f,%f", x, y, width, height);

    qreal fontSize = 12.0;
    QFont textFont;
    QColor fontColor;
    QString fontFamily = "Arial";
    QString fontStretch = "normal";

    bool italic = false;
    int fontWeight = QFont::Normal;
    int textAlign = Qt::AlignLeft;
    QTransform fontTransform;
    parseTextAttributes(fontSize, fontColor, fontFamily, fontStretch, italic, fontWeight, textAlign, fontTransform);
    textFont = QFont(fontFamily, fontSize, fontWeight, italic);

    QFontMetricsF metrics = QFontMetricsF(textFont);
    qreal curHeight = metrics.height();

    qreal curY = 0.0;
    qreal curX = 0.0;

    qreal linespacing = QFontMetrics(textFont).leading();

    //remember if text area has transform
    QTransform transform;
    bool hasTransform = getCurElementTransorm(transform);

    QRectF lastDrawnTextBoundingRect;

    QStack<QFont> fontStack;
    QStack<QColor> colorStack;
    QStack<int> alignStack;

    // first extimate desired text area size
    // to do that, parse text area tags
    while(true)
    {
        mReader.readNext();
        QStringRef elementName = mReader.name();
        if (mReader.isEndDocument())
            break;
        if (mReader.isEndElement())
        {
            if (elementName == tBreak)
            {
                //when tbreak appers, move down by the drawn rect height
                //TODO: line spacing is not calculated yet, probably additional code is required
                curY += lastDrawnTextBoundingRect.height() + linespacing;
                curX = 0.0;
                height += lastDrawnTextBoundingRect.height();
                lastDrawnTextBoundingRect = QRectF(0,0,0,0);
                continue;
            }
            if (elementName == tTspan)
            {
                textFont = fontStack.pop();
                fontColor = colorStack.pop();
                textAlign = alignStack.pop();
                continue;
            }
        }
        if (mReader.isEndElement() && elementName == tText)
            break;
        if (mReader.isStartElement() && elementName == tTspan)
        {
            fontStack.push(textFont);
            colorStack.push(fontColor);
            alignStack.push(textAlign);

            parseTextAttributes(fontSize, fontColor, fontFamily, fontStretch, italic, fontWeight, textAlign, fontTransform);
            textFont = QFont(fontFamily, fontSize, fontWeight, italic);
            metrics = QFontMetricsF(textFont);
            curHeight = metrics.height();
            linespacing = QFontMetricsF(textFont).leading();
            continue;
        }
        if (mReader.isCharacters() || mReader.isCDATA())
        {
            QString text = mReader.text().toString();

            //skip empty text
            if (text.trimmed().length() == 0)
                continue;
            //get bounding rect to obtain desired text height
            lastDrawnTextBoundingRect = metrics.boundingRect(QRectF(), textAlign, text);
            QString log = QString().sprintf(" at rect  %f, %f, %f, %f. Bounding rect is %f, %f, %f, %f", 0.0, curY, width, height - curY, lastDrawnTextBoundingRect.x(), lastDrawnTextBoundingRect.y(), lastDrawnTextBoundingRect.width(), lastDrawnTextBoundingRect.height());
            qWarning() << "Text " << text << log;
            textFonts.append(textFont);
            textRects.append(QRectF(curX, curY, lastDrawnTextBoundingRect.width(), lastDrawnTextBoundingRect.height()));
            textLines.append(text);
            textAligns.append(textAlign);
            textColors.append(fontColor);
            curX += lastDrawnTextBoundingRect.width();
            if (width < curX)
                width = curX;
            if (height == 0)
                height = curHeight;
            
            continue;
        }
    }

    QSvgGenerator *generator = createSvgGenerator(width, height);
    QPainter painter;
    painter.begin(generator);

    if (textRects.count() != 0)
    {
        QListIterator<QRectF> textRectsIter(textRects);
        QListIterator<QFont> textFontsIter(textFonts);
        QListIterator<QString> textLinesIter(textLines);
        QListIterator<int> textAlignsIter(textAligns);
        QListIterator<QColor> textColorsIter(textColors);

        while (textRectsIter.hasNext())
        {
            QRectF rt = textRectsIter.next();
            QFont font = textFontsIter.next();
            QString line = textLinesIter.next();
            int align = textAlignsIter.next();
            QColor color = textColorsIter.next();
            painter.setFont(font);
            painter.setPen(color);
            painter.drawText(rt.x(), rt.y(), rt.width(), rt.height(), align, line);
        }
    }

    painter.end();

    //add resulting svg file to scene
    UBGraphicsSvgItem *svgItem = mCurrentScene->addSvg(QUrl::fromLocalFile(generator->fileName()));
    repositionSvgItem(svgItem, width, height, x, y, hasTransform, transform);

    delete generator;

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parsePolygon()
{
    if (currentState != SVG && currentState != PAGE)
    {
        qWarning() << "iwb content parse error, unexpected polygon tag at line" << mReader.lineNumber();
        return false;
    }

    //create new scene if it's not created yet (for one page document case)
    if (currentState == SVG && mCurrentScene == NULL)
        createNewScene();

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parsePage()
{
    if (currentState != PAGESET && currentState != SVG)
    {
        qWarning() << "iwb content parse error, unexpected page tag at line" << mReader.lineNumber();
        return false;
    }

    createNewScene();

    qWarning() << "Added page number" << mProxy->pageCount();

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parsePageSet()
{
    if (currentState != SVG)
    {
        qWarning() << "iwb content parse error, unexpected page tag at line" << mReader.lineNumber();
        return false;
    }

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::parseIwbElementRef()
{
    if (currentState != IWB)
    {
        qWarning() << "iwb content parse error, unexpected element tag at line" << mReader.lineNumber();
        return false;
    }

    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::createNewScene()
{
    mCurrentScene = UBPersistenceManager::persistenceManager()->createDocumentSceneAt(mProxy, mProxy->pageCount());
    mCurrentSceneRect = mCurrentScene->normalizedSceneRect();
    return true;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::persistCurrentScene()
{
    if (mCurrentScene != 0 && mCurrentScene->isModified())
    {
        UBThumbnailAdaptor::persistScene(mProxy->persistencePath(), mCurrentScene, mProxy->pageCount() - 1);
        UBSvgSubsetAdaptor::persistScene(mProxy, mCurrentScene, mProxy->pageCount() - 1);

        mCurrentScene->setModified(false);
        mCurrentScene = 0;
    }
    return true;
}

QColor UBCFFSubsetAdaptor::UBCFFSubsetReader::colorFromString(const QString& clrString)
{
    //init regexp with pattern
    //pattern corresponds to strings like 'rgb(1,2,3) or rgb(10%,20%,30%)'
    QRegExp regexp("rgb\\(([0-9]+%{0,1}),([0-9]+%{0,1}),([0-9]+%{0,1})\\)");
    if (regexp.exactMatch(clrString))
    {
        if (regexp.capturedTexts().count() == 4 && regexp.capturedTexts().at(0).length() == clrString.length())
        {
            int r = regexp.capturedTexts().at(1).toInt();
            if (regexp.capturedTexts().at(1).indexOf("%") != -1)
                r = r * 255 / 100;
            int g = regexp.capturedTexts().at(2).toInt();
            if (regexp.capturedTexts().at(2).indexOf("%") != -1)
                g = g * 255 / 100;
            int b = regexp.capturedTexts().at(3).toInt();
            if (regexp.capturedTexts().at(3).indexOf("%") != -1)
                b = b * 255 / 100;
            return QColor(r, g, b);
        }
        else
            return QColor();
    }
    else
        return QColor(clrString);
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::getCurElementTransorm(QTransform &transform)
{
    if (mReader.attributes().hasAttribute(aTransform))
    {
        transform = transformFromString(mReader.attributes().value(aTransform).toString());
        return true;
    }
    else
        return false;
}

QTransform UBCFFSubsetAdaptor::UBCFFSubsetReader::transformFromString(const QString trString)
{
    //check pattern for strings like 'rotate(10)'
    QRegExp regexp("rotate\\( *([-+]{0,1}[0-9]*\\.{0,1}[0-9]*) *\\)");
    if (regexp.exactMatch(trString))
    {
        if (regexp.capturedTexts().count() == 2 && regexp.capturedTexts().at(0).length() == trString.length())
        {
            qreal angle = regexp.capturedTexts().at(1).toDouble();
            return QTransform().rotate(angle);
        }
    }

    //check pattern for strings like 'rotate(10,20,20)' or 'rotate(10.1,10.2,34.2)'
    regexp.setPattern("rotate\\( *([-+]{0,1}[0-9]*\\.{0,1}[0-9]*) *, *([-+]{0,1}[0-9]*\\.{0,1}[0-9]*) *, *([-+]{0,1}[0-9]*\\.{0,1}[0-9]*) *\\)");
    if (regexp.exactMatch(trString))
    {
        if (regexp.capturedTexts().count() == 4 && regexp.capturedTexts().at(0).length() == trString.length())
        {
            qreal angle = regexp.capturedTexts().at(1).toDouble();
            qreal dx = regexp.capturedTexts().at(2).toDouble();
            qreal dy = regexp.capturedTexts().at(3).toDouble();
            return QTransform().translate(dx, dy).rotate(angle);
        }
    }

    return QTransform();
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::getViewBoxDimenstions(const QString& viewBox)
{
    //check pattern for strings like 'rotate(10)'
    QRegExp regexp("([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)");
    if (regexp.exactMatch(viewBox))
    {
        int capturesCount = regexp.capturedTexts().count();
        if (capturesCount == 5 && regexp.capturedTexts().at(0).length() == viewBox.length())
        {
            mViewBox = QRectF(0, 0, regexp.capturedTexts().at(3).toDouble(), regexp.capturedTexts().at(4).toDouble());
            mViewBoxCenter.setX(mViewBox.width() / 2);
            mViewBoxCenter.setY(mViewBox.height() / 2);
            return true;
        }
    }

    mViewBox = QRectF(0, 0, 1000, 1000);
    mViewBoxCenter = QPointF(500, 500);
    return false;
}

QSvgGenerator* UBCFFSubsetAdaptor::UBCFFSubsetReader::createSvgGenerator(qreal width, qreal height)
{
    QSvgGenerator* generator = new QSvgGenerator();
    qWarning() << QString("Making generator with file %1, size (%2, %3) and viewbox (%4 %5 %6 %7)").arg(mTempFilePath)
        .arg(width).arg(height).arg(0.0).arg(0.0).arg(width).arg(width);
    generator->setResolution(QApplication::desktop()->physicalDpiY());
    generator->setFileName(mTempFilePath);
    generator->setSize(QSize(width, height));
    generator->setViewBox(QRectF(0, 0, width, height));

    return generator;
}

bool UBCFFSubsetAdaptor::UBCFFSubsetReader::getTempFileName()
{
    int tmpNumber = 0;
    QDir rootDir;
    while (true)
    {
        mTempFilePath = QString("%1/sanksvg%2.%3")
                .arg(rootDir.tempPath())
                .arg(QDateTime::currentDateTime().toString("dd_MM_yyyy_HH-mm"))
                .arg(tmpNumber);
        if (!QFile::exists(mTempFilePath))
            return true;
        tmpNumber++;
        if (tmpNumber == 100000)
        {
            qWarning() << "Import failed. Failed to create temporary file for svg objects";
            return false;
        }
    }

}

void UBCFFSubsetAdaptor::UBCFFSubsetReader::parseTextAttributes(qreal &fontSize, QColor &fontColor,
                                                                QString &fontFamily, QString &fontStretch, bool &italic,
                                                                int &fontWeight, int &textAlign, QTransform &fontTransform)
{
    if (mReader.attributes().hasAttribute(aFontSize))
    {
        //consider inch has 72 liens
        //since svg font size is given in pixels, divide it by pixels per line
        fontSize = mReader.attributes().value(aFontSize).toString().toDouble() * 72 / QApplication::desktop()->physicalDpiY();
    }

    if (mReader.attributes().hasAttribute(aFill))
        fontColor = colorFromString(mReader.attributes().value(aFill).toString());

    if (mReader.attributes().hasAttribute(aFontfamily))
        fontFamily = mReader.attributes().value(aFontfamily).toString();

    if (mReader.attributes().hasAttribute(aFontstretch))
        fontStretch = mReader.attributes().value(aFontstretch).toString();

    if (mReader.attributes().hasAttribute(aFontstyle))
    {
        QStringRef fontStyle = mReader.attributes().value(aFontstyle);
        italic = fontStyle == "italic";
    }

    if (mReader.attributes().hasAttribute(aFontweight))
    {
        QStringRef weight = mReader.attributes().value(aFontweight);
        if (weight == "normal")
            fontWeight = QFont::Normal;
        else if (weight == "light")
            fontWeight = QFont::Light;
        else if (weight == "demibold")
            fontWeight = QFont::DemiBold;
        else if (weight == "bold")
            fontWeight = QFont::Bold;
        if (weight == "black")
            fontWeight = QFont::Black;
    }

    if (mReader.attributes().hasAttribute(aTextalign))
    {
        QString align = mReader.attributes().value(aTextalign).toString();
        if (align == "middle" || align == "center")
            textAlign = Qt::AlignHCenter;
        else
            if (align == "start")
                textAlign = Qt::AlignLeft;
        else
                if (align == "end")
                    textAlign = Qt::AlignRight;
    }

    if (mReader.attributes().hasAttribute(aTransform))
        fontTransform = transformFromString(mReader.attributes().value(aTransform).toString());
}

