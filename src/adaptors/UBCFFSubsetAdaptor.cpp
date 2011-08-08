#include <QRegExp>
#include <QSvgGenerator>
#include <QSvgRenderer>

#include "UBCFFSubsetAdaptor.h"


#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBItem.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStroke.h"

#include "UBMetadataDcSubsetAdaptor.h"
#include "UBThumbnailAdaptor.h"
#include "UBSvgSubsetAdaptor.h"

//enum of xmlparse status

//tag names definition
//use them everiwhere!
static char* tElement       = "element";
static char* tEllipse       = "ellipse";
static char* tIwb           = "iwb";
static char* tMeta          = "meta";
static char* tPage          = "page";
static char* tPageset       = "pageset";
static char* tPolygon       = "polygon";
static char* tRect          = "rect";
static char* tSvg           = "svg";
static char* tTextarea      = "textarea";

//attribute names definition
static char* aFill          = "fill";
static char* aFillopacity   = "fill-opacity";
static char* aX             = "x";
static char* aY             = "y";
static char* aWidth         = "width";
static char* aHeight        = "height";
static char* aStroke        = "stroke";
static char* aStrokewidth   = "stroke-width";
static char* aCx            = "cx";
static char* aCy            = "cy";
static char* aRx            = "rx";
static char* aRy            = "ry";
static char* aTransform     = "transform";
static char* aViewbox       = "viewbox";

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
    //TODO parse
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

    //fill and stroke color
    QColor fillColor = colorFromString(mReader.attributes().value(aFill).toString());
    QColor strokeColor = colorFromString(mReader.attributes().value(aStroke).toString());
    int strokeWidth = mReader.attributes().value(aStrokewidth).toString().toInt();

    //rect lef top corner coordinates
    qreal x1 = mReader.attributes().value(aX).toString().toDouble();
    qreal y1 = mReader.attributes().value(aY).toString().toDouble();
    //rect dimensions
    qreal width = mReader.attributes().value(aWidth).toString().toDouble();
    qreal height = mReader.attributes().value(aHeight).toString().toDouble();

    //init svg generator with temp file
    QSvgGenerator *generator = createSvgGenerator();

    //init painter to paint to svg
    QPainter painter;
    painter.begin(generator);

    //check if rect is rotated
    if (mReader.attributes().hasAttribute(aTransform))
    {
        QTransform transform = transformFromString(mReader.attributes().value(aTransform).toString());
        painter.setTransform(transform);
        //change left top coordinates to correspond to transformation
        x1 -= transform.dx();
        y1 -= transform.dy();
    }

    painter.setBrush(QBrush(fillColor));
    painter.fillRect(x1, y1, width, height, fillColor);

    QPen pen(strokeColor);
    pen.setWidth(strokeWidth);
    painter.setPen(pen);
    painter.drawRect(x1, y1, width, height);

    painter.end();

    //add resulting svg file to scene
    mCurrentScene->addSvg(QUrl::fromLocalFile(generator->fileName()));
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

    QSvgGenerator *generator = createSvgGenerator();

    //fill and stroke color
    QColor fillColor = colorFromString(mReader.attributes().value(aFill).toString());
    QColor strokeColor = colorFromString(mReader.attributes().value(aStroke).toString());
    int strokeWidth = mReader.attributes().value(aStrokewidth).toString().toInt();

    //ellipse center coordinates
    qreal cx = mReader.attributes().value(aCx).toString().toDouble();
    qreal cy = mReader.attributes().value(aCy).toString().toDouble();
    //ellipse horisontal and vertical radius
    qreal rx = mReader.attributes().value(aRx).toString().toDouble();
    qreal ry = mReader.attributes().value(aRy).toString().toDouble();

    //we should change cx and cy by rx and ry because qpainter
    //draws ellipse by its rect coordinates
    cx -= rx;
    cy -= ry;

    //init painter to paint to svg
    QPainter painter;
    painter.begin(generator);

    //check if ellipse is rotated
    if (mReader.attributes().hasAttribute(aTransform))
    {
        QTransform transform = transformFromString(mReader.attributes().value(aTransform).toString());
        painter.setTransform(transform);
        //change cx and cy to correspond to transformation
        cx -= transform.dx();
        cy -= transform.dy();
    }

    QPen pen(strokeColor);
    pen.setWidth(strokeWidth);
    painter.setPen(pen);
    painter.setBrush(QBrush(fillColor));

    painter.drawEllipse(cx, cy, rx * 2, ry * 2);

    painter.end();

    //add resulting svg file to scene
    mCurrentScene->addSvg(QUrl::fromLocalFile(generator->fileName()));
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

QTransform UBCFFSubsetAdaptor::UBCFFSubsetReader::transformFromString(const QString trString)
{
    //check pattern for strings like 'rotate(10)'
    QRegExp regexp("rotate\\(([-+]{0,1}[0-9]*\\.{0,1}[0-9]*)\\)");
    if (regexp.exactMatch(trString))
    {
        if (regexp.capturedTexts().count() == 2 && regexp.capturedTexts().at(0).length() == trString.length())
        {
            qreal angle = regexp.capturedTexts().at(1).toDouble();
            return QTransform().rotate(angle);
        }
    }

    //check pattern for strings like 'rotate(10,20,20)' or 'rotate(10.1,10.2,34.2)'
    regexp.setPattern("rotate\\(([-+]{0,1}[0-9]*\\.{0,1}[0-9]*),([-+]{0,1}[0-9]*\\.{0,1}[0-9]*),([-+]{0,1}[0-9]*\\.{0,1}[0-9]*)\\)");
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
            return true;
        }
    }

    mViewBox = QRectF(0, 0, 1000, 1000);
    return false;
}

QSvgGenerator* UBCFFSubsetAdaptor::UBCFFSubsetReader::createSvgGenerator()
{
    QSvgGenerator* generator = new QSvgGenerator();
    generator->setFileName(mTempFilePath);
    generator->setSize(mSize);
    generator->setViewBox(mViewBox);

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
