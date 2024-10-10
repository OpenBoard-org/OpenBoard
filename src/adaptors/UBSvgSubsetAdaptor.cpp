/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBSvgSubsetAdaptor.h"

#include <QObject>
#include <QtCore>
#include <QtXml>
#include <QGraphicsTextItem>
#include <QDomElement>
#include <QGraphicsVideoItem>
#include <QElapsedTimer>

#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsTextItemDelegate.h"
#include "domain/UBGraphicsStroke.h"
#include "domain/UBGraphicsStrokesGroup.h"
#include "domain/UBGraphicsGroupContainerItem.h"
#include "domain/UBGraphicsGroupContainerItemDelegate.h"
#include "domain/UBItem.h"

#include "gui/UBBackgroundRuling.h"
#include "gui/UBBackgroundManager.h"
#include "tools/UBGraphicsRuler.h"
#include "tools/UBGraphicsAxes.h"
#include "tools/UBGraphicsCompass.h"
#include "tools/UBGraphicsProtractor.h"
#include "tools/UBGraphicsCurtainItem.h"
#include "tools/UBGraphicsTriangle.h"
#include "tools/UBGraphicsCache.h"

#include "document/UBDocumentProxy.h"

#include "board/UBBoardView.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardPaletteManager.h"

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBStringUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBPersistenceManager.h"
#include "core/UBApplication.h"
#include "core/UBDisplayManager.h"
#include "core/UBTextTools.h"

#include "pdf/PDFRenderer.h"

#include "core/memcheck.h"

const QString UBSvgSubsetAdaptor::nsSvg = "http://www.w3.org/2000/svg";
const QString UBSvgSubsetAdaptor::nsXHtml = "http://www.w3.org/1999/xhtml";
const QString UBSvgSubsetAdaptor::nsXLink = "http://www.w3.org/1999/xlink";
const QString UBSvgSubsetAdaptor::xmlTrue = "true";
const QString UBSvgSubsetAdaptor::xmlFalse = "false";
const QString UBSvgSubsetAdaptor::sFontSizePrefix = "font-size:";
const QString UBSvgSubsetAdaptor::sPixelUnit = "px";
const QString UBSvgSubsetAdaptor::sFontWeightPrefix = "font-weight:";
const QString UBSvgSubsetAdaptor::sFontStylePrefix = "font-style:";
const QString UBSvgSubsetAdaptor::sFormerUniboardDocumentNamespaceUri = "http://www.mnemis.com/uniboard";

const QString tElement = "element";
const QString tGroup = "group";
const QString tStrokeGroup = "strokeGroup";
const QString tGroups = "groups";
const QString aId = "id";


QString UBSvgSubsetAdaptor::toSvgTransform(const QTransform& matrix)
{
    return QString("matrix(%1, %2, %3, %4, %5, %6)")
            .arg(matrix.m11(), 0 , 'g')
            .arg(matrix.m12(), 0 , 'g')
            .arg(matrix.m21(), 0 , 'g')
            .arg(matrix.m22(), 0 , 'g')
            .arg(matrix.dx(), 0 , 'g')
            .arg(matrix.dy(), 0 , 'g');
}


QTransform UBSvgSubsetAdaptor::fromSvgTransform(const QString& transform)
{
    QTransform matrix;
    QString ts = transform;
    ts.replace("matrix(", "");
    ts.replace(")", "");
    QStringList sl = ts.split(",");

    if (sl.size() >= 6)
    {
        matrix.setMatrix(
                    sl.at(0).toFloat(),
                    sl.at(1).toFloat(),
                    0,
                    sl.at(2).toFloat(),
                    sl.at(3).toFloat(),
                    0,
                    sl.at(4).toFloat(),
                    sl.at(5).toFloat(),
                    1);
    }

    return matrix;
}



static bool itemZIndexComp(const QGraphicsItem* item1,
                           const QGraphicsItem* item2)
{
    return item1->data(UBGraphicsItemData::ItemOwnZValue).toReal() < item2->data(UBGraphicsItemData::ItemOwnZValue).toReal();
}


void UBSvgSubsetAdaptor::upgradeScene(std::shared_ptr<UBDocumentProxy> proxy, const int pageIndex)
{
    //4.2
    QDomDocument doc = loadSceneDocument(proxy, pageIndex);
    QDomElement elSvg = doc.documentElement(); // SVG tag
    QString ubVersion = elSvg.attributeNS(UBSettings::uniboardDocumentNamespaceUri, "version", "4.1"); // default to 4.1

    if (ubVersion.startsWith("4.1") || ubVersion.startsWith("4.2") || ubVersion.startsWith("4.3"))
    {
        // migrate to 4.2.1 (or latter)
        std::shared_ptr<UBGraphicsScene> scene = loadScene(proxy, pageIndex);
        scene->setModified(true);
        persistScene(proxy, scene, pageIndex);
    }
}


QDomDocument UBSvgSubsetAdaptor::loadSceneDocument(std::shared_ptr<UBDocumentProxy> proxy, const int pPageIndex)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg",pPageIndex);

    QFile file(fileName);
    QDomDocument doc("page");

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Cannot open file " << fileName << " for reading ...";
            return doc;
        }

        doc.setContent(&file, true);
        file.close();
    }

    return doc;
}


void UBSvgSubsetAdaptor::setSceneUuid(std::shared_ptr<UBDocumentProxy> proxy, const int pageIndex, QUuid pUuid)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg",pageIndex);

    QFile file(fileName);

    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return;

    QTextStream textReadStream(&file);
    QString xmlContent = textReadStream.readAll();
    int uuidIndex = xmlContent.indexOf("uuid");
    if (-1 == uuidIndex)
    {
        qWarning() << "Cannot read UUID from file" << fileName << "to set new UUID";
        file.close();
        return;
    }
    int quoteStartIndex = xmlContent.indexOf('"', uuidIndex);
    if (-1 == quoteStartIndex)
    {
        qWarning() << "Cannot read UUID from file" << fileName << "to set new UUID";
        file.close();
        return;
    }
    int quoteEndIndex = xmlContent.indexOf('"', quoteStartIndex + 1);
    if (-1 == quoteEndIndex)
    {
        qWarning() << "Cannot read UUID from file" << fileName << "to set new UUID";
        file.close();
        return;
    }

    file.close();

    QString newXmlContent = xmlContent.left(quoteStartIndex + 1);
    newXmlContent.append(UBStringUtils::toCanonicalUuid(pUuid));
    newXmlContent.append(xmlContent.right(xmlContent.length() - quoteEndIndex));

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QTextStream textWriteStream(&file);
        textWriteStream << newXmlContent;
        file.close();
    }
    else
    {
        qWarning() << "Cannot open file" << fileName  << "to write UUID";
    }
}

QString UBSvgSubsetAdaptor::uniboardDocumentNamespaceUriFromVersion(int mFileVersion)
{
    return mFileVersion >= 40200 ? UBSettings::uniboardDocumentNamespaceUri : sFormerUniboardDocumentNamespaceUri;
}


std::shared_ptr<UBGraphicsScene> UBSvgSubsetAdaptor::loadScene(std::shared_ptr<UBDocumentProxy> proxy, const int pageIndex)
{
    UBApplication::showMessage(QObject::tr("Loading scene (%1/%2)").arg(pageIndex+1).arg(proxy->pageCount()));
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex);
    qInfo() << "loading scene. Filename is : " << fileName;
    QFile file(fileName);

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Cannot open file " << fileName << " for reading ...";
            return 0;
        }

        std::shared_ptr<UBGraphicsScene> scene = loadScene(proxy, file.readAll());

        file.close();

        return scene;
    }

    return 0;
}


QByteArray UBSvgSubsetAdaptor::loadSceneAsText(std::shared_ptr<UBDocumentProxy> proxy, const int pageIndex)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex);
    qDebug() << fileName;
    QFile file(fileName);

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Cannot open file " << fileName << " for reading ...";
            return "";
        }

        return file.readAll();

        file.close();

    }
    return "";
}


QUuid UBSvgSubsetAdaptor::sceneUuid(std::shared_ptr<UBDocumentProxy> proxy, const int pageIndex)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex);

    QFile file(fileName);

    QUuid uuid;

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Cannot open file " << fileName << " for reading ...";
            return QUuid();
        }

        QXmlStreamReader xml(file.readAll());

        bool foundSvg = false;

        while (!xml.atEnd() && !foundSvg)
        {
            xml.readNext();

            if (xml.isStartElement())
            {
                if (xml.name().toString() == "svg")
                {
                    auto svgSceneUuid = xml.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "uuid");
                    if (svgSceneUuid.isNull())
                        svgSceneUuid = xml.attributes().value("http://www.mnemis.com/uniboard", "uuid");

                    if (!svgSceneUuid.isNull())
                        uuid = QUuid(svgSceneUuid.toString());

                    foundSvg = true;
                }
            }
        }

        file.close();
    }

    return uuid;
}


std::shared_ptr<UBGraphicsScene> UBSvgSubsetAdaptor::loadScene(std::shared_ptr<UBDocumentProxy> proxy, const QByteArray& pArray)
{
    UBSvgSubsetReader reader(proxy, UBTextTools::cleanHtmlCData(QString(pArray)).toUtf8());
    return reader.loadScene(proxy);
}

std::shared_ptr<UBSvgSubsetAdaptor::UBSvgReaderContext> UBSvgSubsetAdaptor::prepareLoadingScene(std::shared_ptr<UBDocumentProxy> proxy, const int pageIndex)
{
    const auto fileContent = loadSceneAsText(proxy, pageIndex);
    auto context = std::make_shared<UBSvgReaderContext>(proxy, fileContent);
    return context;
}

UBSvgSubsetAdaptor::UBSvgSubsetReader::UBSvgSubsetReader(std::shared_ptr<UBDocumentProxy> pProxy, const QByteArray& pXmlData)
    : mXmlReader(pXmlData)
    , mProxy(pProxy)
    , mDocumentPath(pProxy->persistencePath())
    , mGroupHasInfo(false)
{
    // NOOP
}


std::shared_ptr<UBGraphicsScene> UBSvgSubsetAdaptor::UBSvgSubsetReader::loadScene(std::shared_ptr<UBDocumentProxy> proxy)
{
    QElapsedTimer time;
    time.start();

    start();

    while (!mXmlReader.atEnd())
    {
        processElement();
    }

    qDebug() << "spent milliseconds: " << time.elapsed();
    return scene();
}

void UBSvgSubsetAdaptor::UBSvgSubsetReader::start()
{
    qDebug() << "loadScene() : starting reading...";
    mScene = nullptr;
    UBGraphicsWidgetItem *currentWidget = 0;
    //bool pageDpiSpecified = true;
    saveSceneAfterLoading = false;

    mFileVersion = 40100; // default to 4.1.0
}

bool UBSvgSubsetAdaptor::UBSvgSubsetReader::isFinished()
{
    return mXmlReader.atEnd();
}

void UBSvgSubsetAdaptor::UBSvgSubsetReader::processElement()
{
    mXmlReader.readNext();
    if (mXmlReader.isStartElement())
    {
        QString name = mXmlReader.name().toString();

        if (name == "svg")
        {
            if (!mScene)
            {
                mScene = std::make_shared<UBGraphicsScene>(mProxy, false);
                mScene->setModified(false);
            }

            // introduced in UB 4.2

            auto svgUbVersion = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "version");

            if (!svgUbVersion.isNull())
            {
                QString ubVersion = svgUbVersion.toString();

                //may look like : 4 or 4.1 or 4.2 or 4.2.1, etc

                QStringList parts = ubVersion.split(".");

                if (parts.length() > 0)
                {
                    mFileVersion = parts.at(0).toInt() * 10000;
                }

                if (parts.length() > 1)
                {
                    mFileVersion += parts.at(1).toInt() * 100;
                }

                if (parts.length() > 2)
                {
                    mFileVersion += parts.at(2).toInt();
                }
            }

            mNamespaceUri = uniboardDocumentNamespaceUriFromVersion(mFileVersion);

            auto svgSceneUuid = mXmlReader.attributes().value(mNamespaceUri, "uuid");

            if (!svgSceneUuid.isNull())
                mScene->setUuid(QUuid(svgSceneUuid.toString()));

            // introduced in UB 4.0

            auto svgViewBox = mXmlReader.attributes().value("viewBox");


            if (!svgViewBox.isNull())
            {
                QStringList ts = svgViewBox.toString().split(QLatin1Char(' '), UB::SplitBehavior::SkipEmptyParts);

                QRectF sceneRect;
                if (ts.size() >= 4)
                {
                    sceneRect.setX(ts.at(0).toFloat());
                    sceneRect.setY(ts.at(1).toFloat());
                    sceneRect.setWidth(ts.at(2).toFloat());
                    sceneRect.setHeight(ts.at(3).toFloat());

                    mScene->setSceneRect(sceneRect);
                }
                else
                {
                    qWarning() << "cannot make sense of 'viewBox' value " << svgViewBox.toString();
                }
            }

            auto pageDpi = mXmlReader.attributes().value("pageDpi");

            if (!pageDpi.isNull())
                mProxy->setPageDpi(pageDpi.toInt());

            else if (mProxy->pageDpi() == 0) {
                mProxy->setPageDpi(UBApplication::displayManager->logicalDpi(ScreenRole::Control));
                //pageDpiSpecified = false;
            }

            bool darkBackground = false;
            bool crossedBackground = false;
            bool ruledBackground = false;
            bool intermediateLines = false;

            auto ubDarkBackground = mXmlReader.attributes().value(mNamespaceUri, "dark-background");

            if (!ubDarkBackground.isNull())
                darkBackground = (ubDarkBackground.toString() == xmlTrue);

            auto ubCrossedBackground = mXmlReader.attributes().value(mNamespaceUri, "crossed-background");

            if (!ubCrossedBackground.isNull())
                crossedBackground = (ubCrossedBackground.toString() == xmlTrue);

            auto ubGridSize = mXmlReader.attributes().value(mNamespaceUri, "grid-size");

            if (!ubGridSize.isNull()) {
                int gridSize = ubGridSize.toInt();

                mScene->setBackgroundGridSize(gridSize);
            }

            auto ubRuledBackground = mXmlReader.attributes().value(mNamespaceUri, "ruled-background");

            if (!ubRuledBackground.isNull())
                ruledBackground = (ubRuledBackground.toString() == xmlTrue);

            if (ruledBackground || crossedBackground) {

                auto ubIntermediateLines = mXmlReader.attributes().value(mNamespaceUri, "intermediate-lines");

                if (!ubIntermediateLines.isNull()) {
                    intermediateLines = ubIntermediateLines.toInt();
                }
            }

            const UBBackgroundRuling* background{nullptr};

            // guess background pattern from attributes
            if (crossedBackground || ruledBackground)
            {
                const auto bgManager = UBApplication::boardController->backgroundManager();
                background = bgManager->guessBackground(crossedBackground, ruledBackground, intermediateLines);
            }

            mScene->setSceneBackground(darkBackground, background);

            auto pageNominalSize = mXmlReader.attributes().value(mNamespaceUri, "nominal-size");
            if (!pageNominalSize.isNull())
            {
                QStringList ts = pageNominalSize.toString().split(QLatin1Char('x'), UB::SplitBehavior::SkipEmptyParts);

                QSize sceneSize;
                if (ts.size() >= 2)
                {
                    sceneSize.setWidth(ts.at(0).toInt());
                    sceneSize.setHeight(ts.at(1).toInt());

                    mScene->setNominalSize(sceneSize);
                }
                else
                {
                    qWarning() << "cannot make sense of 'nominal-size' value " << pageNominalSize.toString();
                }

            }
        }
        else if (name == "background")
        {
            UBBackgroundRuling bg;
            bg.parseXml(mXmlReader);

            if (bg.isValid())
            {
                const auto bgManager = UBApplication::boardController->backgroundManager();
                bgManager->addBackground(bg);
                mScene->setSceneBackground(mScene->isDarkBackground(), bgManager->background(bg.uuid()));
            }
        }
        else if (name == "g")
        {
            strokesGroup = new UBGraphicsStrokesGroup();
            graphicsItemFromSvg(strokesGroup);

            bool hasZValue;
            qreal zValue = normalizedZValue(&hasZValue);

            if (hasZValue)
            {
                mGroupZIndex = zValue;
                mGroupHasInfo = true;
            }

            auto ubFillOnDarkBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-dark-background");

            if (!ubFillOnDarkBackground.isNull())
            {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
                    mGroupDarkBackgroundColor = QColor::fromString(ubFillOnDarkBackground.toString());
#else
                mGroupDarkBackgroundColor.setNamedColor(ubFillOnDarkBackground.toString());
#endif
            }

            auto ubFillOnLightBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-light-background");

            if (!ubFillOnLightBackground.isNull())
            {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
                    mGroupLightBackgroundColor = QColor::fromString(ubFillOnLightBackground.toString());
#else
                mGroupLightBackgroundColor.setNamedColor(ubFillOnLightBackground.toString());
#endif
            }

            auto ubUuid = mXmlReader.attributes().value(mNamespaceUri, "uuid");

            if (!ubUuid.isNull())
                strokesGroup->setUuid(QUuid(ubUuid.toString()));
            else
                strokesGroup->setUuid(QUuid::createUuid());

            QString uuid_stripped = strokesGroup->uuid().toString().replace("}","").replace("{","");

            if (!mStrokesList.contains(uuid_stripped))
                mStrokesList.insert(uuid_stripped, strokesGroup);
        }
        else if (name == "polygon" || name == "line")
        {
            UBGraphicsPolygonItem* polygonItem = 0;

            QString parentId = mXmlReader.attributes().value(mNamespaceUri, "parent").toString();

            if (name == "polygon")
                polygonItem = polygonItemFromPolygonSvg(mScene->isDarkBackground() ? Qt::white : Qt::black);
            else if (name == "line")
                polygonItem = polygonItemFromLineSvg(mScene->isDarkBackground() ? Qt::white : Qt::black);

            if(parentId.isEmpty() && strokesGroup)
                parentId = strokesGroup->uuid().toString();

            if(parentId.isEmpty())
                parentId = QUuid::createUuid().toString();

            if (polygonItem)
            {
                polygonItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Graphic));

                UBGraphicsStrokesGroup* group;
                if(!mStrokesList.contains(parentId)){
                    group = new UBGraphicsStrokesGroup();
                    mStrokesList.insert(parentId,group);
                    group->setTransform(polygonItem->transform());
                    UBGraphicsItem::assignZValue(group, polygonItem->zValue());
                }
                else
                    group = mStrokesList.value(parentId);

                if (!currentStroke)
                    currentStroke = new UBGraphicsStroke();

                if(polygonItem->transform().isIdentity())
                    polygonItem->setTransform(group->transform());

                group->addToGroup(polygonItem);
                polygonItem->setStrokesGroup(group);
                polygonItem->setStroke(currentStroke);

                polygonItem->show();
                group->addToGroup(polygonItem);
            }
        }
        else if (name == "polyline")
        {
            QList<UBGraphicsPolygonItem*> polygonItems = polygonItemsFromPolylineSvg(mScene->isDarkBackground() ? Qt::white : Qt::black);

            QString parentId = mXmlReader.attributes().value(mNamespaceUri, "parent").toString();

            if(parentId.isEmpty() && strokesGroup)
                parentId = strokesGroup->uuid().toString();

            if(parentId.isEmpty())
                parentId = QUuid::createUuid().toString();

            foreach(UBGraphicsPolygonItem* polygonItem, polygonItems)
            {
                polygonItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Graphic));

                UBGraphicsStrokesGroup* group;

                if(!mStrokesList.contains(parentId)){
                    group = new UBGraphicsStrokesGroup();
                    mStrokesList.insert(parentId,group);
                    group->setTransform(polygonItem->transform());
                    UBGraphicsItem::assignZValue(group, polygonItem->zValue());
                }
                else
                    group = mStrokesList.value(parentId);

                if (!currentStroke)
                    currentStroke = new UBGraphicsStroke();

                if(polygonItem->transform().isIdentity())
                    polygonItem->setTransform(group->transform());

                group->addToGroup(polygonItem);
                polygonItem->setStrokesGroup(group);
                polygonItem->setStroke(currentStroke);

                polygonItem->show();
                group->addToGroup(polygonItem);
            }

        }
        else if (name == "image")
        {
            auto imageHref = mXmlReader.attributes().value(nsXLink, "href");

            if (!imageHref.isNull())
            {
                QString href = imageHref.toString();

                auto ubBackground = mXmlReader.attributes().value(mNamespaceUri, "background");

                bool isBackground = (!ubBackground.isNull() && ubBackground.toString() == xmlTrue);

                if (href.endsWith(".svg"))
                {
                    UBGraphicsSvgItem* svgItem = svgItemFromSvg();
                    if (svgItem)
                    {
                        svgItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                        svgItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

                        mScene->addItem(svgItem);

                        if (isBackground)
                            mScene->setAsBackgroundObject(svgItem);

                        svgItem->show();
                    }
                }
                else
                {
                    UBGraphicsPixmapItem* pixmapItem = pixmapItemFromSvg();
                    if (pixmapItem)
                    {
                        pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

                        mScene->addItem(pixmapItem);

                        if (isBackground)
                            mScene->setAsBackgroundObject(pixmapItem);

                        pixmapItem->show();
                    }
                }
            }
        }
        else if (name == "audio")
        {
            UBGraphicsMediaItem* audioItem = audioItemFromSvg();
            if (audioItem)
            {
                audioItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                audioItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

                mScene->addItem(audioItem);

                audioItem->show();

                audioItem->play();
                audioItem->pause();
            }
        }
        else if (name == "video")
        {
            UBGraphicsMediaItem* videoItem = videoItemFromSvg();
            if (videoItem)
            {

                videoItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                videoItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

                mScene->addItem(videoItem);

                videoItem->show();
            }
        }
        else if (name == "text")//This is for backward compatibility with proto text field prior to version 4.3
        {
            UBGraphicsTextItem* textItem = textItemFromSvg();
            if (textItem)
            {
                textItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

                mScene->addItem(textItem);

                textItem->show();
            }
        }
        else if (name == "curtain")
        {
            UBGraphicsCurtainItem* mask = curtainItemFromSvg();
            if (mask)
            {
                mScene->addItem(mask);
                mScene->registerTool(mask);
            }
        }
        else if (name == "ruler")
        {

            UBGraphicsRuler *ruler = rulerFromSvg();
            if (ruler)
            {
                mScene->addItem(ruler);
                mScene->registerTool(ruler);
            }

        }
        else if (name == "axes")
        {

            UBGraphicsAxes *axes = axesFromSvg();
            if (axes)
            {
                mScene->addItem(axes);
            }

        }
        else if (name == "compass")
        {
            UBGraphicsCompass *compass = compassFromSvg();
            if (compass)
            {
                mScene->addItem(compass);
                mScene->registerTool(compass);
            }
        }
        else if (name == "protractor")
        {
            UBGraphicsProtractor *protractor = protractorFromSvg();
            if (protractor)
            {
                mScene->addItem(protractor);
                mScene->registerTool(protractor);
            }
        }
        else if (name == "triangle")
        {
            UBGraphicsTriangle *triangle = triangleFromSvg();
            if (triangle)
            {
                mScene->addItem(triangle);
                mScene->registerTool(triangle);
            }
        }
        else if (name == "cache")
        {
            UBGraphicsCache* cache = cacheFromSvg();
            if(cache)
            {
                mScene->addItem(cache);
                mScene->registerTool(cache);
                UBApplication::boardController->notifyCache(true);
            }
        }
        else if (name == "foreignObject")
        {
            QString href = mXmlReader.attributes().value(nsXLink, "href").toString();
            QString src = mXmlReader.attributes().value(mNamespaceUri, "src").toString();
            QString type = mXmlReader.attributes().value(mNamespaceUri, "type").toString();
            bool isBackground = mXmlReader.attributes().value(mNamespaceUri, "background").toString() == xmlTrue;

            qreal foreignObjectWidth = mXmlReader.attributes().value("width").toString().toFloat();
            qreal foreignObjectHeight = mXmlReader.attributes().value("height").toString().toFloat();

            if (href.contains(".pdf"))
            {
                UBGraphicsPDFItem* pdfItem = pdfItemFromPDF();
                if (pdfItem)
                {
                    qreal currentDpi = UBApplication::displayManager->logicalDpi(ScreenRole::Control);
                    // qDebug() << "currentDpi = " << currentDpi;
                    qreal pdfScale = qreal(mProxy->pageDpi())/currentDpi;
                    // qDebug() << "pdfScale " << pdfScale;

                    // If the PDF is in the background, it occupies the whole page; so we can simply
                    // use that information to calculate its scale.
                    if (isBackground) {
                        qreal pageWidth = mScene->nominalSize().width();
                        qreal pageHeight = mScene->nominalSize().height();

                        qreal scaleX = pageWidth / pdfItem->sceneBoundingRect().width();
                        qreal scaleY = pageHeight / pdfItem->sceneBoundingRect().height();

                        pdfScale = (scaleX+scaleY)/2.;
                    }

                    pdfItem->setScale(pdfScale);
                    pdfItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                    pdfItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

                    mScene->addItem(pdfItem);

                    if (isBackground)
                        mScene->setAsBackgroundObject(pdfItem);

                    pdfItem->show();

                    currentWidget = 0;
                }
            }
            else if (src.contains(".wdgt")) // NOTE @letsfindaway obsolete
            {
                UBGraphicsAppleWidgetItem* appleWidgetItem = graphicsAppleWidgetFromSvg();
                if (appleWidgetItem)
                {
                    appleWidgetItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

                    appleWidgetItem->resize(foreignObjectWidth, foreignObjectHeight);

                    mScene->addItem(appleWidgetItem);

                    appleWidgetItem->show();

                    currentWidget = appleWidgetItem;
                }
            }
            else if (src.contains(".wgt"))
            {
                UBGraphicsW3CWidgetItem* w3cWidgetItem = graphicsW3CWidgetFromSvg();

                if (w3cWidgetItem)
                {
                    // check compatibility
                    QUuid uuid(mXmlReader.attributes().value(mNamespaceUri, "uuid").toString());

                    if (!mProxy->isWidgetCompatible(uuid))
                    {
                        // show a substitute image and freeze the widget
                        QPixmap pixmap(w3cWidgetItem->size().toSize());
                        pixmap.fill(Qt::lightGray);
                        QPainter painter(&pixmap);
                        QFont font = painter.font();
                        font.setPixelSize(16);
                        painter.setFont(font);
                        const QRectF rectangle(QPointF(0, 0), w3cWidgetItem->size());

                        std::string incompatibleWidgetMessage = QString("Incompatible widget (%1).").arg(w3cWidgetItem->metadatas().name).toStdString();
                        painter.drawText(rectangle, Qt::AlignCenter, QObject::tr(incompatibleWidgetMessage.c_str()));
                        w3cWidgetItem->setSnapshot(pixmap, true);

                        // disable user interactions
                        w3cWidgetItem->setFreezable(false);
                        w3cWidgetItem->setCanBeTool(false);
                    }

                    w3cWidgetItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

                    w3cWidgetItem->resize(foreignObjectWidth, foreignObjectHeight);

                    mScene->addItem(w3cWidgetItem);

                    w3cWidgetItem->show();

                    currentWidget = w3cWidgetItem;
                }
            }
            else if (type == "text")
            {
                UBGraphicsTextItem* textItem = textItemFromSvg();

                if (textItem)
                {
                    textItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                    textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                    textItem->activateTextEditor(false);

                    mScene->addItem(textItem);

                    textItem->show();
                }
            }
            else
            {
                qWarning() << "Ignoring unknown foreignObject:" << href;
            }
        }
        else if (currentWidget && (name == "preference"))
        {
            QString key = mXmlReader.attributes().value("key").toString();
            QString value = mXmlReader.attributes().value("value").toString();

            currentWidget->setPreference(key, value);
        }
        else if (currentWidget && (name == "datastoreEntry"))
        {
            QString key = mXmlReader.attributes().value("key").toString();
            QString value = mXmlReader.attributes().value("value").toString();

            currentWidget->setDatastoreEntry(key, value);
        } else if (name == tGroups) {
            //considering groups section at the end of the document
            readGroupRoot();
        }
        else
        {
            // NOOP
        }
    }
    else if (mXmlReader.isEndElement())
    {
        if (mXmlReader.name().toString() == "g")
        {
            mGroupHasInfo = false;
            mGroupDarkBackgroundColor = QColor();
            mGroupLightBackgroundColor = QColor();
            strokesGroup = NULL;
            currentStroke = NULL;
        }
    }

    if (mXmlReader.atEnd())
    {
        mMustFinalize = true;
    }
}

std::shared_ptr<UBGraphicsScene> UBSvgSubsetAdaptor::UBSvgSubsetReader::scene()
{
    if (mMustFinalize)
    {
        if (mXmlReader.hasError())
        {
            qWarning() << "error parsing file " << mXmlReader.errorString();
        }

        qDebug() << "Number of detected strokes: " << mStrokesList.count();

        if (mScene) {
            QHashIterator<QString, UBGraphicsStrokesGroup*> iterator(mStrokesList);
            while (iterator.hasNext()) {
                iterator.next();
                mScene->addItem(iterator.value());
            }

            mScene->setModified(saveSceneAfterLoading);
            mScene->enableUndoRedoStack();
            qDebug() << "loadScene() : created scene and read file";
        }

        mMustFinalize = false;
    }

    return mScene;
}


UBGraphicsGroupContainerItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::readGroup()
{
    UBGraphicsGroupContainerItem *group = new UBGraphicsGroupContainerItem();
    QList<QGraphicsItem *> groupContainer;

    QString id = mXmlReader.attributes().value(aId).toString();
    id = id.mid(1,id.length()-2);
    bool shouldSkipSubElements = false;
    if(mStrokesList.contains(id))
        shouldSkipSubElements = true;

    QString ubLocked = mXmlReader.attributes().value(mNamespaceUri, "locked").toString();
    if (!ubLocked.isEmpty())
    {
        bool isLocked = ubLocked.contains(xmlTrue);
        group->setData(UBGraphicsItemData::ItemLocked, QVariant(isLocked));
    }

    QString ubHiddenOnDisplay = mXmlReader.attributes().value(mNamespaceUri, "hidden-on-display").toString();
    if (!ubHiddenOnDisplay.isEmpty())
    {
        bool isHiddenOnDisplay = ubHiddenOnDisplay.contains(xmlTrue);
        group->setData(UBGraphicsItemData::ItemIsHiddenOnDisplay, QVariant(isHiddenOnDisplay));
    }

    auto ubLayer = mXmlReader.attributes().value(mNamespaceUri, "layer");
    if (!ubLayer.isNull())
    {
        bool ok;
        int layerAsInt = ubLayer.toString().toInt(&ok);

        if (ok)
            group->setData(UBGraphicsItemData::ItemLayerType, QVariant(layerAsInt));
    }

    auto ubZValue = mXmlReader.attributes().value(mNamespaceUri, "z-value");
    if (!ubZValue.isNull())
    {
        bool ok;
        qreal zValue = ubZValue.toString().toFloat(&ok);

        if (ok)
        {
            group->setZValue(zValue);
            group->setData(UBGraphicsItemData::ItemOwnZValue, QVariant(zValue));
        }
    }

    mXmlReader.readNext();
    while (!mXmlReader.atEnd())
    {
        if (mXmlReader.isEndElement()) {
            mXmlReader.readNext();
            break;
        }
        else if (mXmlReader.isStartElement()) {
            if (mXmlReader.name() == tGroup) {
                UBGraphicsGroupContainerItem *curGroup = readGroup();
                if (curGroup)
                    groupContainer.append(curGroup);
            }
            else if (mXmlReader.name() == tElement && !shouldSkipSubElements) {
                QString id = mXmlReader.attributes().value(aId).toString();
                QGraphicsItem *curItem = readElementFromGroup();

                // Explanation: the second condition discriminate the old storage version that should
                // not be interpreted anymore
                if(curItem && id.count("{") < 2)
                    groupContainer.append(curItem);
            }
            else {
                mXmlReader.skipCurrentElement();
            }
        }
        else {
            mXmlReader.readNext();
        }
    }

    foreach(QGraphicsItem* item, groupContainer)
        group->addToGroup(item);

    if (group->childItems().count())
    {
//        mScene->addItem(group);

        if (1 == group->childItems().count())
        {
            group->destroy(false);
        }
    }
    return group;
}

void UBSvgSubsetAdaptor::UBSvgSubsetReader::readGroupRoot()
{
    mXmlReader.readNext();
    while (!mXmlReader.atEnd()) {
        if (mXmlReader.isEndElement()) {
            mXmlReader.readNext();
            break;
        }
        else if (mXmlReader.isStartElement()) {
            if (mXmlReader.name() == tGroup) {

                UBGraphicsGroupContainerItem *curGroup = readGroup();

                if (curGroup) {
                    mScene->addGroup(curGroup);
                }
            }
            else {
                mXmlReader.skipCurrentElement();
            }
        }
        else {
            mXmlReader.readNext();
        }
    }
}

QGraphicsItem *UBSvgSubsetAdaptor::UBSvgSubsetReader::readElementFromGroup()
{
    QGraphicsItem *result = 0;
    QString id = mXmlReader.attributes().value(aId).toString();
    QString uuid = id.right(QUuid().toString().size());
    result = mScene->itemForUuid(QUuid(uuid));

    if(!result)
        result = mStrokesList.take(uuid.replace("}","").replace("{",""));

    mXmlReader.skipCurrentElement();
    mXmlReader.readNext();

    return result;
}

void UBSvgSubsetAdaptor::persistScene(std::shared_ptr<UBDocumentProxy> proxy, std::shared_ptr<UBGraphicsScene> pScene, const int pageIndex)
{
    UBSvgSubsetWriter writer(proxy, pScene, pageIndex);
    writer.persistScene(proxy, pageIndex);
}


UBSvgSubsetAdaptor::UBSvgSubsetWriter::UBSvgSubsetWriter(std::shared_ptr<UBDocumentProxy> proxy, std::shared_ptr<UBGraphicsScene> pScene, const int pageIndex)
    : mScene(pScene)
    , mDocumentPath(proxy->persistencePath())
    , mPageIndex(pageIndex)

{
    // NOOP
}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::writeSvgElement(std::shared_ptr<UBDocumentProxy> proxy)
{
    mXmlWriter.writeStartElement("svg");

    mXmlWriter.writeAttribute("version", "1.1");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "version", UBSettings::currentFileVersion);
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(mScene->uuid()));

    int margin = UBSettings::settings()->svgViewBoxMargin->get().toInt();
    QRect normalized = mScene->normalizedSceneRect().toRect();
    normalized.translate(margin * -1, margin * -1);
    normalized.setWidth(normalized.width() + (margin * 2));
    normalized.setHeight(normalized.height() + (margin * 2));
    mXmlWriter.writeAttribute("viewBox", QString("%1 %2 %3 %4").arg(normalized.x()).arg(normalized.y()).arg(normalized.width()).arg(normalized.height()));

    QSize pageNominalSize = mScene->nominalSize();
    if (pageNominalSize.isValid())
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "nominal-size", QString("%1x%2").arg(pageNominalSize.width()).arg(pageNominalSize.height()));
    }

    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "dark-background", mScene->isDarkBackground() ? xmlTrue : xmlFalse);

    bool crossedBackground = mScene->background() && mScene->background()->isCrossed();
    bool ruledBackground = mScene->background() && mScene->background()->isRuled();
    bool intermediateLines = mScene->background() && mScene->background()->hasIntermediateLines();

    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "crossed-background", crossedBackground ? xmlTrue : xmlFalse);
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "ruled-background", ruledBackground ? xmlTrue : xmlFalse);

    int gridSize = mScene->backgroundGridSize();
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "grid-size", QString::number(gridSize));

    if (crossedBackground || ruledBackground) {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "intermediate-lines", QString::number(intermediateLines));
    }

    if (proxy->pageDpi() == 0)
        proxy->setPageDpi(UBApplication::displayManager->logicalDpi(ScreenRole::Control));

    mXmlWriter.writeAttribute("pageDpi", QString::number(proxy->pageDpi()));


    mXmlWriter.writeStartElement("rect");
    mXmlWriter.writeAttribute("fill", mScene->isDarkBackground() ? "black" : "white");
    mXmlWriter.writeAttribute("x", QString::number(normalized.x()));
    mXmlWriter.writeAttribute("y", QString::number(normalized.y()));
    mXmlWriter.writeAttribute("width", QString::number(normalized.width()));
    mXmlWriter.writeAttribute("height", QString::number(normalized.height()));

    mXmlWriter.writeEndElement();
}

bool UBSvgSubsetAdaptor::UBSvgSubsetWriter::persistScene(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex)
{
    Q_UNUSED(pageIndex);

    //Creating dom structure to store information
    QDomDocument groupDomDocument;
    QDomElement groupRoot = groupDomDocument.createElement(tGroups);
    groupDomDocument.appendChild(groupRoot);

    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    mXmlWriter.setDevice(&buffer);

    mXmlWriter.setAutoFormatting(true);

    mXmlWriter.writeStartDocument();
    mXmlWriter.writeDefaultNamespace(nsSvg);
    mXmlWriter.writeNamespace(nsXLink, "xlink");
    mXmlWriter.writeNamespace(UBSettings::uniboardDocumentNamespaceUri, "ub");
    mXmlWriter.writeNamespace(nsXHtml, "xhtml");

    writeSvgElement(proxy);

    if (mScene->background())
    {
        mScene->background()->toXml(mXmlWriter);
    }

    // Get the items from the scene
    QList<QGraphicsItem*> items = mScene->items();

    std::sort(items.begin(), items.end(), itemZIndexComp);

    UBGraphicsStroke *openStroke = 0;

    bool groupHoldsInfo = false;

    while (!items.empty())
    {
        QGraphicsItem *item = items.takeFirst();

        // Is the item a polygon?
        UBGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<UBGraphicsPolygonItem*> (item);
        if (polygonItem && polygonItem->isVisible())
        {
            UBGraphicsStroke* currentStroke = polygonItem->stroke();
            if (openStroke && (currentStroke != openStroke))
            {
                mXmlWriter.writeEndElement(); //g
                openStroke = 0;
                groupHoldsInfo = false;
            }

            bool firstPolygonInStroke = currentStroke  && !openStroke;

            if (firstPolygonInStroke)
            {
                mXmlWriter.writeStartElement("g");
                openStroke = currentStroke;

                UBGraphicsStroke* stroke = dynamic_cast<UBGraphicsStroke* >(currentStroke);

                if (stroke)
                {
                    QColor colorOnDarkBackground = polygonItem->colorOnDarkBackground();
                    QColor colorOnLightBackground = polygonItem->colorOnLightBackground();
                    UBGraphicsStrokesGroup * sg = polygonItem->strokesGroup();

                    if (colorOnDarkBackground.isValid() && colorOnLightBackground.isValid() && sg)
                    {
                        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value"
                                                  , QString("%1").arg(polygonItem->strokesGroup()->zValue()));

                        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri
                                                  , "fill-on-dark-background", colorOnDarkBackground.name());
                        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri
                                                  , "fill-on-light-background", colorOnLightBackground.name());

                        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(sg->uuid()));

                        QVariant locked = sg->data(UBGraphicsItemData::ItemLocked);
                        if (!locked.isNull() && locked.toBool())
                            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "locked", xmlTrue);

                        QVariant hiddenOnDisplay = sg->data(UBGraphicsItemData::ItemIsHiddenOnDisplay);
                        if (!hiddenOnDisplay.isNull() && hiddenOnDisplay.toBool())
                            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "hidden-on-display", xmlTrue);
                        else
                            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "hidden-on-display", xmlFalse);

                        QVariant layer = sg->data(UBGraphicsItemData::ItemLayerType);
                        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "layer", QString("%1").arg(layer.toInt()));

                        QTransform matrix = sg->sceneTransform();
                        if (!matrix.isIdentity())
                            mXmlWriter.writeAttribute("transform", toSvgTransform(matrix));

                        qDebug() << "Attributes written";

                        groupHoldsInfo = true;
                    }
                }

                if (stroke && !stroke->hasPressure())
                {

                    strokeToSvgPolyline(stroke, groupHoldsInfo);

                    //we can dequeue all polygons belonging to that stroke
                    foreach(UBGraphicsPolygonItem* gi, stroke->polygons())
                    {
                        items.removeOne(gi);
                    }
                    continue;
                }
            }

            UBGraphicsStroke* stroke = dynamic_cast<UBGraphicsStroke* >(currentStroke);

            if (stroke && stroke->hasPressure())
                polygonItemToSvgPolygon(polygonItem, groupHoldsInfo);

            else if (polygonItem->isNominalLine())
                polygonItemToSvgLine(polygonItem, groupHoldsInfo);


            continue;
        }

        if (openStroke)
        {
            mXmlWriter.writeEndElement(); //g
            groupHoldsInfo = false;
            openStroke = 0;
        }

        // Is the item a picture?
        UBGraphicsPixmapItem *pixmapItem = qgraphicsitem_cast<UBGraphicsPixmapItem*> (item);
        if (pixmapItem && pixmapItem->isVisible())
        {
            pixmapItemToLinkedImage(pixmapItem);
            continue;
        }

        // Is the item a shape?
        UBGraphicsSvgItem *svgItem = qgraphicsitem_cast<UBGraphicsSvgItem*> (item);
        if (svgItem && svgItem->isVisible())
        {
            svgItemToLinkedSvg(svgItem);
            continue;
        }

        UBGraphicsVideoItem * videoItem = qgraphicsitem_cast<UBGraphicsVideoItem*> (item);

        if (videoItem && videoItem->isVisible()) {
            videoItemToLinkedVideo(videoItem);
            continue;
        }

        UBGraphicsAudioItem * audioItem = qgraphicsitem_cast<UBGraphicsAudioItem*> (item);

        if (audioItem && audioItem->isVisible()) {
            audioItemToLinkedAudio(audioItem);
            continue;
        }

        // Is the item an app? // NOTE @letsfindaway obsolete
        UBGraphicsAppleWidgetItem *appleWidgetItem = qgraphicsitem_cast<UBGraphicsAppleWidgetItem*> (item);
        if (appleWidgetItem && appleWidgetItem->isVisible())
        {
            graphicsAppleWidgetToSvg(appleWidgetItem);
            continue;
        }

        // Is the item a W3C?
        UBGraphicsW3CWidgetItem *w3cWidgetItem = qgraphicsitem_cast<UBGraphicsW3CWidgetItem*> (item);
        if (w3cWidgetItem && w3cWidgetItem->isVisible())
        {
            graphicsW3CWidgetToSvg(w3cWidgetItem);
            continue;
        }

        // Is the item a PDF?
        UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*> (item);
        if (pdfItem && pdfItem->isVisible())
        {
            pdfItemToLinkedPDF(pdfItem);
            continue;
        }

        // Is the item a text?
        UBGraphicsTextItem *textItem = qgraphicsitem_cast<UBGraphicsTextItem*> (item);
        if (textItem && textItem->isVisible())
        {
            textItemToSvg(textItem);
            continue;
        }

        // Is the item a curtain?
        UBGraphicsCurtainItem *curtainItem = qgraphicsitem_cast<UBGraphicsCurtainItem*> (item);
        if (curtainItem && curtainItem->isVisible())
        {
            curtainItemToSvg(curtainItem);
            continue;
        }

        // Is the item a ruler?
        UBGraphicsRuler *ruler = qgraphicsitem_cast<UBGraphicsRuler*> (item);
        if (ruler && ruler->isVisible())
        {
            rulerToSvg(ruler);
            continue;
        }

        // Is the item a axes?
        UBGraphicsAxes *axes = qgraphicsitem_cast<UBGraphicsAxes*> (item);
        if (axes && axes->isVisible())
        {
            axesToSvg(axes);
            continue;
        }

        // Is the item a cache?
        UBGraphicsCache* cache = qgraphicsitem_cast<UBGraphicsCache*>(item);
        if(cache && cache->isVisible())
        {
            cacheToSvg(cache);
            continue;
        }

        // Is the item a compass
        UBGraphicsCompass *compass = qgraphicsitem_cast<UBGraphicsCompass*> (item);
        if (compass && compass->isVisible())
        {
            compassToSvg(compass);
            continue;
        }

        // Is the item a protractor?
        UBGraphicsProtractor *protractor = qgraphicsitem_cast<UBGraphicsProtractor*> (item);
        if (protractor && protractor->isVisible())
        {
            protractorToSvg(protractor);
            continue;
        }

        // Is the item a triangle?
        UBGraphicsTriangle *triangle = qgraphicsitem_cast<UBGraphicsTriangle*> (item);
        if (triangle && triangle->isVisible())
        {
            triangleToSvg(triangle);
            continue;
        }

        // Is the item a group?
        UBGraphicsGroupContainerItem *groupItem = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(item);
        if (groupItem && groupItem->isVisible())
        {
            persistGroupToDom(groupItem, &groupRoot, &groupDomDocument);
            continue;
        }
    }

    if (openStroke)
    {
        mXmlWriter.writeEndElement();
        openStroke = 0;
    }

    //writing group data
    if (groupRoot.hasChildNodes()) {
        mXmlWriter.writeStartElement(tGroups);
        QDomElement curElement = groupRoot.firstChildElement();
        while (!curElement.isNull()) {
            if (curElement.hasAttribute(aId)) {
                mXmlWriter.writeStartElement(curElement.tagName());
                mXmlWriter.writeAttribute(aId, curElement.attribute(aId));
                if(curElement.hasAttribute("locked")){
                    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri,"locked",curElement.attribute("locked"));
                }
                if(curElement.hasAttribute("hidden-on-display")){
                    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri,"hidden-on-display", curElement.attribute("hidden-on-display"));
                }
                if(curElement.hasAttribute("layer")){
                    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri,"layer",curElement.attribute("layer"));
                }
                if(curElement.hasAttribute("z-value")){
                    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri,"z-value",curElement.attribute("z-value"));
                }

                QDomElement curSubElement = curElement.firstChildElement();
                while (!curSubElement.isNull()) {
                    if (curSubElement.hasAttribute(aId)) {
                        mXmlWriter.writeStartElement(curSubElement.tagName());
                        mXmlWriter.writeAttribute(aId, curSubElement.attribute(aId));
                        mXmlWriter.writeEndElement();
                        curSubElement = curSubElement.nextSiblingElement();
                    }
                }
                mXmlWriter.writeEndElement();
            }
            curElement = curElement.nextSiblingElement();
        }
        mXmlWriter.writeEndElement();
    }

    mXmlWriter.writeEndDocument();
    QString fileName = mDocumentPath + UBFileSystemUtils::digitFileFormat("/page%1.svg", mPageIndex);
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "cannot open " << fileName << " for writing. Error : " << file.errorString();
        return false;
    }
    file.write(buffer.data());
    file.flush();
    file.close();

    return true;
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::persistGroupToDom(QGraphicsItem *groupItem, QDomElement *curParent, QDomDocument *groupDomDocument)
{
    QUuid uuid = UBGraphicsScene::getPersonalUuid(groupItem);
    if (!uuid.isNull()) {
        QDomElement curGroupElement = groupDomDocument->createElement(tGroup);
        curGroupElement.setAttribute(aId, uuid.toString());
        UBGraphicsGroupContainerItem* group = dynamic_cast<UBGraphicsGroupContainerItem*>(groupItem);
        if(group && group->Delegate()){
            if(group->Delegate()->isLocked())
                curGroupElement.setAttribute("locked", xmlTrue);
            else
                curGroupElement.setAttribute("locked", xmlFalse);

            if (group->data(UBGraphicsItemData::ItemIsHiddenOnDisplay).toBool())
                curGroupElement.setAttribute("hidden-on-display", xmlTrue);
            else
                curGroupElement.setAttribute("hidden-on-display", xmlFalse);

            curGroupElement.setAttribute("layer", group->data(UBGraphicsItemData::ItemLayerType).toString());

            QString zs;
            zs.setNum(group->zValue(), 'f'); // 'f' keeps precision
            curGroupElement.setAttribute("z-value", zs);
        }
        curParent->appendChild(curGroupElement);
        foreach (QGraphicsItem *item, groupItem->childItems()) {
            QUuid tmpUuid = UBGraphicsScene::getPersonalUuid(item);
            if (!tmpUuid.isNull()) {
                if (item->type() == UBGraphicsGroupContainerItem::Type && item->childItems().count())
                    persistGroupToDom(item, curParent, groupDomDocument);
                else {
                    QDomElement curSubElement = groupDomDocument->createElement(tElement);
                    curSubElement.setAttribute(aId, tmpUuid.toString());
                    curGroupElement.appendChild(curSubElement);
                }
            }
        }
    }
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::polygonItemToSvgLine(UBGraphicsPolygonItem* polygonItem, bool groupHoldsInfo)
{
    mXmlWriter.writeStartElement("line");

    QLineF line = polygonItem->originalLine();

    mXmlWriter.writeAttribute("x1", QString::number(line.p1().x(), 'f', 2));
    mXmlWriter.writeAttribute("y1", QString::number(line.p1().y(), 'f', 2));

    // SVG renderers (Chrome) do not like line where (x1, y1) == (x2, y2)
    qreal x2 = line.p2().x();
    if (line.p1() == line.p2())
        x2 += 0.01;

    mXmlWriter.writeAttribute("x2", QString::number(x2, 'f', 2));
    mXmlWriter.writeAttribute("y2", QString::number(line.p2().y(), 'f', 2));

    mXmlWriter.writeAttribute("stroke-width", QString::number(polygonItem->originalWidth(), 'f', -1));
    mXmlWriter.writeAttribute("stroke", polygonItem->brush().color().name());

    qreal alpha = polygonItem->brush().color().alphaF();
    if (alpha < 1.0)
        mXmlWriter.writeAttribute("stroke-opacity", QString::number(alpha, 'f', 2));
    mXmlWriter.writeAttribute("stroke-linecap", "round");

    if (!groupHoldsInfo)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", QString("%1").arg(polygonItem->zValue()));
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "fill-on-dark-background", polygonItem->colorOnDarkBackground().name());
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "fill-on-light-background", polygonItem->colorOnLightBackground().name());
    }

    mXmlWriter.writeEndElement();

}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::strokeToSvgPolyline(UBGraphicsStroke* stroke, bool groupHoldsInfo)
{
    QList<UBGraphicsPolygonItem*> pols = stroke->polygons();

    if (pols.length() > 0)
    {
        mXmlWriter.writeStartElement("polyline");
        QVector<QPointF> points;

        foreach(UBGraphicsPolygonItem* polygon, pols)
        {
            points << polygon->originalLine().p1();
        }

        points << pols.last()->originalLine().p2();

        // SVG renderers (Chrome) do not like line withe where x1/y1 == x2/y2
        if (points.size() == 2 && (points.at(0) == points.at(1)))
        {
            points[1] = QPointF(points[1].x() + 0.01, points[1].y());
        }

        QString svgPoints = pointsToSvgPointsAttribute(points);
        mXmlWriter.writeAttribute("points", svgPoints);

        UBGraphicsPolygonItem* firstPolygonItem = pols.at(0);

        mXmlWriter.writeAttribute("fill", "none");
        mXmlWriter.writeAttribute("stroke-width", QString::number(firstPolygonItem->originalWidth(), 'f', 2));
        mXmlWriter.writeAttribute("stroke", firstPolygonItem->brush().color().name());
        mXmlWriter.writeAttribute("stroke-opacity", QString("%1").arg(firstPolygonItem->brush().color().alphaF()));
        mXmlWriter.writeAttribute("stroke-linecap", "round");

        if (!groupHoldsInfo)
        {

            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", QString("%1").arg(firstPolygonItem->zValue()));

            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri
                                      , "fill-on-dark-background", firstPolygonItem->colorOnDarkBackground().name());
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri
                                      , "fill-on-light-background", firstPolygonItem->colorOnLightBackground().name());
        }

        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(firstPolygonItem->uuid()));
        if (firstPolygonItem->parentItem()) {
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "parent", UBStringUtils::toCanonicalUuid(UBGraphicsItem::getOwnUuid(firstPolygonItem->strokesGroup())));
        }

        mXmlWriter.writeEndElement();
    }
}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::strokeToSvgPolygon(UBGraphicsStroke* stroke, bool groupHoldsInfo)
{
    QList<UBGraphicsPolygonItem*> pis = stroke->polygons();

    if (pis.length() > 0)
    {
        QPolygonF united;

        foreach(UBGraphicsPolygonItem* pi, pis)
        {
            united = united.united(pi->polygon());
        }


        QScopedPointer<UBGraphicsPolygonItem> clone(static_cast<UBGraphicsPolygonItem*>(pis.at(0)->deepCopy()));
        clone->setPolygon(united);

        polygonItemToSvgPolygon(clone.get(), groupHoldsInfo);
    }
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::polygonItemToSvgPolygon(UBGraphicsPolygonItem* polygonItem, bool groupHoldsInfo)
{

    QPolygonF polygon = polygonItem->polygon();
    int pointsCount = polygon.size();

    if (pointsCount > 0)
    {
        mXmlWriter.writeStartElement("polygon");

        QString points = pointsToSvgPointsAttribute(polygon);
        mXmlWriter.writeAttribute("points", points);
        mXmlWriter.writeAttribute("transform",toSvgTransform(polygonItem->transform()));
        mXmlWriter.writeAttribute("fill", polygonItem->brush().color().name());

        qreal alpha = polygonItem->brush().color().alphaF();
        mXmlWriter.writeAttribute("fill-opacity", QString::number(alpha, 'f', 2));

        // we trick SVG antialiasing, to avoid seeing light gaps between polygons
        if (alpha < 1.0)
        {
            qreal trickedAlpha = trickAlpha(alpha);
            mXmlWriter.writeAttribute("stroke", polygonItem->brush().color().name());
            mXmlWriter.writeAttribute("stroke-width", "1");
            mXmlWriter.writeAttribute("stroke-opacity", QString::number(trickedAlpha, 'f', 2));
        }

        // svg default fill rule is nonzero, but Qt is evenodd
        //
        //http://www.w3.org/TR/SVG11/painting.html
        //http://doc.trolltech.com/4.5/qgraphicspolygonitem.html#fillRule
        //

        if (polygonItem->fillRule() == Qt::OddEvenFill)
            mXmlWriter.writeAttribute("fill-rule", "evenodd");
        else
            mXmlWriter.writeAttribute("fill-rule", "winding");

        if (!groupHoldsInfo)
        {
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", QString("%1").arg(polygonItem->zValue()));
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri
                                      , "fill-on-dark-background", polygonItem->colorOnDarkBackground().name());
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri
                                      , "fill-on-light-background", polygonItem->colorOnLightBackground().name());
        }

        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(polygonItem->uuid()));
        UBGraphicsStrokesGroup* sg = polygonItem->strokesGroup();
        if (sg)
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "parent", UBStringUtils::toCanonicalUuid(sg->uuid()));

        mXmlWriter.writeEndElement();
    }
}


UBGraphicsPolygonItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::polygonItemFromPolygonSvg(const QColor& pDefaultColor)
{
    UBGraphicsPolygonItem* polygonItem = new UBGraphicsPolygonItem();

    graphicsItemFromSvg(polygonItem);

    auto svgPoints = mXmlReader.attributes().value("points");

    QPolygonF polygon;

    if (!svgPoints.isNull())
    {
        QStringList ts = svgPoints.toString().split(QLatin1Char(' '), UB::SplitBehavior::SkipEmptyParts);

        foreach(const QString sPoint, ts)
        {
            QStringList sCoord = sPoint.split(QLatin1Char(','), UB::SplitBehavior::SkipEmptyParts);

            if (sCoord.size() == 2)
            {
                QPointF point;
                point.setX(sCoord.at(0).toFloat());
                point.setY(sCoord.at(1).toFloat());
                polygon << point;
            }
            else if (sCoord.size() == 4){
                //This is the case on system were the "," is used to seperate decimal
                QPointF point;
                QString x = sCoord.at(0) + "." + sCoord.at(1);
                QString y = sCoord.at(2) + "." + sCoord.at(3);
                point.setX(x.toFloat());
                point.setY(y.toFloat());
                polygon << point;
            }
            else
            {
                qWarning() << "cannot make sense of a 'point' value" << sCoord;
            }
        }
    }
    else
    {
        qWarning() << "cannot make sense of 'points' value " << svgPoints.toString();
    }

    polygonItem->setPolygon(polygon);

    auto svgFill = mXmlReader.attributes().value("fill");

    QColor brushColor = pDefaultColor;

    if (!svgFill.isNull())
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        brushColor = QColor::fromString(svgFill.toString());
#else
        brushColor.setNamedColor(svgFill.toString());
#endif

    auto svgFillOpacity = mXmlReader.attributes().value("fill-opacity");
    qreal opacity = 1.0;

    if (!svgFillOpacity.isNull())
    {
        opacity = svgFillOpacity.toString().toFloat();
        brushColor.setAlphaF(opacity);
    }

    polygonItem->setColor(brushColor);

    auto ubFillOnDarkBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-dark-background");

    if (!ubFillOnDarkBackground.isNull())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QColor color = QColor::fromString(ubFillOnDarkBackground.toString());
#else
        QColor color;
        color.setNamedColor(ubFillOnDarkBackground.toString());
#endif
        if (!color.isValid())
            color = Qt::white;

        color.setAlphaF(opacity);
        polygonItem->setColorOnDarkBackground(color);
    }
    else
    {
        QColor color = mGroupDarkBackgroundColor;
        color.setAlphaF(opacity);
        polygonItem->setColorOnDarkBackground(color);
    }

    auto ubFillOnLightBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-light-background");

    if (!ubFillOnLightBackground.isNull())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QColor color = QColor::fromString(ubFillOnLightBackground.toString());
#else
        QColor color;
        color.setNamedColor(ubFillOnLightBackground.toString());
#endif
        if (!color.isValid())
            color = Qt::black;
        color.setAlphaF(opacity);
        polygonItem->setColorOnLightBackground(color);
    }
    else
    {
        QColor color = mGroupLightBackgroundColor;
        color.setAlphaF(opacity);
        polygonItem->setColorOnLightBackground(color);
    }

    // Before OpenBoard v1.4, fill rule was only saved if it was "Even-odd". Therefore if no fill rule
    // is specified, we assume that it should be Winding fill.

    auto fillRule = mXmlReader.attributes().value("fill-rule");

    if (!fillRule.isNull() && fillRule.toString() == "evenodd")
        polygonItem->setFillRule(Qt::OddEvenFill);
    else
        polygonItem->setFillRule(Qt::WindingFill);

    return polygonItem;

}

UBGraphicsPolygonItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::polygonItemFromLineSvg(const QColor& pDefaultColor)
{
    auto svgX1 = mXmlReader.attributes().value("x1");
    auto svgY1 = mXmlReader.attributes().value("y1");
    auto svgX2 = mXmlReader.attributes().value("x2");
    auto svgY2 = mXmlReader.attributes().value("y2");

    QLineF line;

    if (!svgX1.isNull() && !svgY1.isNull() && !svgX2.isNull() && !svgY2.isNull())
    {
        qreal x1 = svgX1.toString().toFloat();
        qreal y1 = svgY1.toString().toFloat();
        qreal x2 = svgX2.toString().toFloat();
        qreal y2 = svgY2.toString().toFloat();

        line.setLine(x1, y1, x2, y2);
    }
    else
    {
        qWarning() << "cannot make sense of 'line' value";
        return 0;
    }

    auto strokeWidth = mXmlReader.attributes().value("stroke-width");

    qreal lineWidth = 1.;

    if (!strokeWidth.isNull())
    {
        lineWidth = strokeWidth.toString().toFloat();
    }

    UBGraphicsPolygonItem* polygonItem = new UBGraphicsPolygonItem(line, lineWidth);
    graphicsItemFromSvg(polygonItem);

    auto svgStroke = mXmlReader.attributes().value("stroke");

    QColor brushColor = pDefaultColor;

    if (!svgStroke.isNull())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        brushColor = QColor::fromString(svgStroke.toString());
#else
        brushColor.setNamedColor(svgStroke.toString());
#endif

    }

    auto svgStrokeOpacity = mXmlReader.attributes().value("stroke-opacity");
    qreal opacity = 1.0;

    if (!svgStrokeOpacity.isNull())
    {
        opacity = svgStrokeOpacity.toString().toFloat();
        brushColor.setAlphaF(opacity);
    }

    polygonItem->setColor(brushColor);

    auto ubFillOnDarkBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-dark-background");

    if (!ubFillOnDarkBackground.isNull())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QColor color = QColor::fromString(ubFillOnDarkBackground.toString());
#else
        QColor color;
        color.setNamedColor(ubFillOnDarkBackground.toString());
#endif
        if (!color.isValid())
            color = Qt::white;

        color.setAlphaF(opacity);
        polygonItem->setColorOnDarkBackground(color);
    }
    else
    {
        QColor color = mGroupDarkBackgroundColor;
        color.setAlphaF(opacity);
        polygonItem->setColorOnDarkBackground(color);
    }

    auto ubFillOnLightBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-light-background");

    if (!ubFillOnLightBackground.isNull())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QColor color = QColor::fromString(ubFillOnLightBackground.toString());
#else
        QColor color;
        color.setNamedColor(ubFillOnLightBackground.toString());
#endif
        if (!color.isValid())
            color = Qt::black;
        color.setAlphaF(opacity);
        polygonItem->setColorOnLightBackground(color);
    }
    else
    {
        QColor color = mGroupLightBackgroundColor;
        color.setAlphaF(opacity);
        polygonItem->setColorOnLightBackground(color);
    }

    return polygonItem;
}

QList<UBGraphicsPolygonItem*> UBSvgSubsetAdaptor::UBSvgSubsetReader::polygonItemsFromPolylineSvg(const QColor& pDefaultColor)
{
    auto strokeWidth = mXmlReader.attributes().value("stroke-width");

    qreal lineWidth = 1.;

    if (!strokeWidth.isNull())
    {
        lineWidth = strokeWidth.toString().toFloat();
    }

    QColor brushColor = pDefaultColor;

    auto svgStroke = mXmlReader.attributes().value("stroke");
    if (!svgStroke.isNull())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        brushColor = QColor::fromString(svgStroke.toString());
#else
        brushColor.setNamedColor(svgStroke.toString());
#endif
    }

    qreal opacity = 1.0;

    auto svgStrokeOpacity = mXmlReader.attributes().value("stroke-opacity");
    if (!svgStrokeOpacity.isNull())
    {
        opacity = svgStrokeOpacity.toString().toFloat();
        brushColor.setAlphaF(opacity);
    }

    bool hasZValue;
    qreal zValue = normalizedZValue(&hasZValue);

    if (!hasZValue)
    {
        zValue = mGroupZIndex;
    }


    QColor colorOnDarkBackground = mGroupDarkBackgroundColor;

    auto ubFillOnDarkBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-dark-background");
    if (!ubFillOnDarkBackground.isNull())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        colorOnDarkBackground = QColor::fromString(ubFillOnDarkBackground.toString());
#else
        colorOnDarkBackground.setNamedColor(ubFillOnDarkBackground.toString());
#endif
    }

    if (!colorOnDarkBackground.isValid())
        colorOnDarkBackground = Qt::white;

    colorOnDarkBackground.setAlphaF(opacity);

    QColor colorOnLightBackground = mGroupLightBackgroundColor;

    auto ubFillOnLightBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-light-background");
    if (!ubFillOnLightBackground.isNull())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        colorOnLightBackground = QColor::fromString(ubFillOnLightBackground.toString());
#else
        colorOnLightBackground.setNamedColor(ubFillOnLightBackground.toString());
#endif
    }

    if (!colorOnLightBackground.isValid())
        colorOnLightBackground = Qt::black;

    colorOnLightBackground.setAlphaF(opacity);

    auto svgPoints = mXmlReader.attributes().value("points");

    QList<UBGraphicsPolygonItem*> polygonItems;

    if (!svgPoints.isNull())
    {
        QStringList ts = svgPoints.toString().split(QLatin1Char(' '),
                                                    UB::SplitBehavior::SkipEmptyParts);

        QList<QPointF> points;

        foreach(const QString sPoint, ts)
        {
            QStringList sCoord = sPoint.split(QLatin1Char(','), UB::SplitBehavior::SkipEmptyParts);

            if (sCoord.size() == 2)
            {
                QPointF point;
                point.setX(sCoord.at(0).toFloat());
                point.setY(sCoord.at(1).toFloat());
                points << point;
            }
            else if (sCoord.size() == 4){
                //This is the case on system were the "," is used to seperate decimal
                QPointF point;
                QString x = sCoord.at(0) + "." + sCoord.at(1);
                QString y = sCoord.at(2) + "." + sCoord.at(3);
                point.setX(x.toFloat());
                point.setY(y.toFloat());
                points << point;
            }
            else
            {
                qWarning() << "cannot make sense of a 'point' value" << sCoord;
            }
        }

        for (int i = 0; i < points.size() - 1; i++)
        {
            UBGraphicsPolygonItem* polygonItem = new UBGraphicsPolygonItem(QLineF(points.at(i), points.at(i + 1)), lineWidth);
            polygonItem->setColor(brushColor);
            UBGraphicsItem::assignZValue(polygonItem, zValue);
            polygonItem->setColorOnDarkBackground(colorOnDarkBackground);
            polygonItem->setColorOnLightBackground(colorOnLightBackground);

            polygonItems <<polygonItem;
        }
    }
    else
    {
        qWarning() << "cannot make sense of 'points' value " << svgPoints.toString();
    }

    return polygonItems;
}




void UBSvgSubsetAdaptor::UBSvgSubsetWriter::pixmapItemToLinkedImage(UBGraphicsPixmapItem* pixmapItem)
{
    // find image file
    QDir imageDir = mDocumentPath + "/" + UBPersistenceManager::imageDirectory;
    QStringList imageFiles = imageDir.entryList({pixmapItem->uuid().toString() + ".*"});

    if (imageFiles.size() >= 1)
    {
        mXmlWriter.writeStartElement("image");

        QString fileName = UBPersistenceManager::imageDirectory + "/" + imageFiles.last();

        mXmlWriter.writeAttribute(nsXLink, "href", fileName);

        graphicsItemToSvg(pixmapItem);

        mXmlWriter.writeEndElement();
    }
}


UBGraphicsPixmapItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::pixmapItemFromSvg()
{
    UBGraphicsPixmapItem* pixmapItem = nullptr;

    auto imageHref = mXmlReader.attributes().value(nsXLink, "href");

    if (!imageHref.isNull())
    {
        pixmapItem = new UBGraphicsPixmapItem();
        QString href = imageHref.toString();
        QImageReader rdr(mDocumentPath + "/" + UBFileSystemUtils::normalizeFilePath(href));
        rdr.setAutoTransform(true);
        QImage img = rdr.read();
        QPixmap pix = QPixmap::fromImage(img);
        pixmapItem->setPixmap(pix);
        graphicsItemFromSvg(pixmapItem);
    }
    else
    {
        qWarning() << "cannot make sense of image href value";
    }

    return pixmapItem;
}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::svgItemToLinkedSvg(UBGraphicsSvgItem* svgItem)
{

    mXmlWriter.writeStartElement("image");

    QString fileName = UBPersistenceManager::imageDirectory + "/" + svgItem->uuid().toString() + ".svg";


    mXmlWriter.writeAttribute(nsXLink, "href", fileName);

    graphicsItemToSvg(svgItem);

    mXmlWriter.writeEndElement();
}


UBGraphicsSvgItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::svgItemFromSvg()
{
    UBGraphicsSvgItem* svgItem = 0;

    auto imageHref = mXmlReader.attributes().value(nsXLink, "href");

    if (!imageHref.isNull())
    {
        QString href = imageHref.toString();

        svgItem = new UBGraphicsSvgItem(mDocumentPath + "/" + UBFileSystemUtils::normalizeFilePath(href));
    }
    else
    {
        qWarning() << "cannot make sens of image href value";
        return 0;
    }

    graphicsItemFromSvg(svgItem);

    return svgItem;

}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::pdfItemToLinkedPDF(UBGraphicsPDFItem* pdfItem)
{
    mXmlWriter.writeStartElement("foreignObject");
    mXmlWriter.writeAttribute("requiredExtensions", "http://ns.adobe.com/pdf/1.3/");

    QString fileName = UBPersistenceManager::objectDirectory + "/" + pdfItem->fileUuid().toString() + ".pdf";

    QString path = mDocumentPath + "/" + fileName;

    if (!QFile::exists(path))
    {
        QDir dir;
        dir.mkdir(mDocumentPath + "/" + UBPersistenceManager::objectDirectory);

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly))
        {
            qWarning() << "cannot open file for writing embeded pdf content " << path;
            return;
        }

        file.write(pdfItem->fileData());
        file.close();
    }

    mXmlWriter.writeAttribute(nsXLink, "href", fileName + "#page=" + QString::number(pdfItem->pageNumber()));

    graphicsItemToSvg(pdfItem);

    mXmlWriter.writeEndElement();
}


UBGraphicsPDFItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::pdfItemFromPDF()
{
    UBGraphicsPDFItem* pdfItem = 0;

    QString href = mXmlReader.attributes().value(nsXLink, "href").toString();
    QStringList parts = href.split("#page=");
    if (parts.count() != 2)
    {
        qWarning() << "invalid pdf href value" << href;
        return 0;
    }

    QString pdfPath = parts[0];
    QUuid uuid(QFileInfo(pdfPath).baseName());
    int pageNumber = parts[1].toInt();

    pdfItem = new UBGraphicsPDFItem(PDFRenderer::rendererForUuid(uuid, mDocumentPath + "/" + UBFileSystemUtils::normalizeFilePath(pdfPath)), pageNumber);

    graphicsItemFromSvg(pdfItem);

    return pdfItem;
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::audioItemToLinkedAudio(UBGraphicsAudioItem *audioItem)
{
    mXmlWriter.writeStartElement("audio");

    graphicsItemToSvg(audioItem);

    if (audioItem->playerState() == QMediaPlayer::PausedState &&
       (audioItem->mediaDuration() - audioItem->mediaPosition()) > 0)
    {
        qint64 pos = audioItem->mediaPosition();
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "position", QString("%1").arg(pos));
    }

    QString audioFileHref = "audios/" + audioItem->mediaFileUrl().fileName();

    mXmlWriter.writeAttribute(nsXLink, "href", audioFileHref);
    mXmlWriter.writeEndElement();
}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::videoItemToLinkedVideo(UBGraphicsVideoItem* videoItem)
{
    /* w3c sample
     *
     *  <video xlink:href="noonoo.avi" volume=".8" type="video/x-msvideo"
     *               width="320" height="240" x="50" y="50" repeatCount="indefinite"/>
     *
     */

    mXmlWriter.writeStartElement("video");

    graphicsItemToSvg(videoItem);

    if (videoItem->playerState() == QMediaPlayer::PausedState &&
       (videoItem->mediaDuration() - videoItem->mediaPosition()) > 0)
    {
        qint64 pos = videoItem->mediaPosition();
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "position", QString("%1").arg(pos));
    }

    QString videoFileHref = "videos/" + videoItem->mediaFileUrl().fileName();

    mXmlWriter.writeAttribute(nsXLink, "href", videoFileHref);
    mXmlWriter.writeEndElement();
}

UBGraphicsMediaItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::audioItemFromSvg()
{

    auto audioHref = mXmlReader.attributes().value(nsXLink, "href");

    if (audioHref.isNull())
    {
        qWarning() << "cannot make sens of video href value";
        return 0;
    }

    QString href = mDocumentPath + "/" + audioHref.toString();

    //Claudio this is necessary to fix the absolute path added on Sankore 3.1 1.00.00
    //The absoult path doesn't work when you want to share Sankore documents.
    if(!audioHref.toString().startsWith("audios/")){
        int indexOfAudioDirectory = href.lastIndexOf("audios");
        href = mDocumentPath + "/" + href.right(href.length() - indexOfAudioDirectory);
    }

    UBGraphicsMediaItem* audioItem = UBGraphicsMediaItem::createMediaItem(QUrl::fromLocalFile(href));
    if(audioItem)
    {
        audioItem->connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), audioItem, SLOT(activeSceneChanged()));

        graphicsItemFromSvg(audioItem);
        auto ubPos = mXmlReader.attributes().value(mNamespaceUri, "position");

        qint64 p = 0;
        if (!ubPos.isNull())
            p = ubPos.toString().toLongLong();

        audioItem->setInitialPos(p);
    }

    return audioItem;
}

UBGraphicsMediaItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::videoItemFromSvg()
{

    auto videoHref = mXmlReader.attributes().value(nsXLink, "href");

    if (videoHref.isNull())
    {
        qWarning() << "cannot make sens of video href value";
        return 0;
    }

    QString href = mDocumentPath + "/" + videoHref.toString();

    //Claudio this is necessary to fix the absolute path added on Sankore 3.1 1.00.00
    //The absoult path doesn't work when you want to share Sankore documents.
    if(!videoHref.toString().startsWith("videos/")){
        int indexOfVideoDirectory = href.lastIndexOf("videos");
        href = mDocumentPath + "/" + href.right(href.length() - indexOfVideoDirectory);
    }

    UBGraphicsMediaItem* videoItem = UBGraphicsMediaItem::createMediaItem(QUrl::fromLocalFile(href));
    if(videoItem){
        videoItem->connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), videoItem, SLOT(activeSceneChanged()));

        graphicsItemFromSvg(videoItem);
        auto ubPos = mXmlReader.attributes().value(mNamespaceUri, "position");

        qint64 p = 0;
        if (!ubPos.isNull())
        {
            p = ubPos.toString().toLongLong();
        }

        videoItem->setInitialPos(p);
    }

    return videoItem;
}

void UBSvgSubsetAdaptor::UBSvgSubsetReader::graphicsItemFromSvg(QGraphicsItem* gItem)
{

    auto svgTransform = mXmlReader.attributes().value("transform");

    QTransform itemMatrix;

    if (!svgTransform.isNull())
    {
        itemMatrix = fromSvgTransform(svgTransform.toString());
        gItem->setTransform(itemMatrix);
    }

    auto svgX = mXmlReader.attributes().value("x");
    auto svgY = mXmlReader.attributes().value("y");

    if (mFileVersion >= 40202)
    {
        gItem->setPos(0,0);
    }
    else if (mFileVersion >= 40201)
    {
        if (!svgX.isNull() && !svgY.isNull())
        {
            gItem->setPos(svgX.toString().toFloat() * itemMatrix.m11(), svgY.toString().toFloat() * itemMatrix.m22());
        }
    }
    else
    {
        if (!svgX.isNull() && !svgY.isNull())
        {
#ifdef Q_OS_WIN
            gItem->setPos(svgX.toString().toFloat(), svgY.toString().toFloat());
#endif
        }
    }

    UBResizableGraphicsItem *rgi = dynamic_cast<UBResizableGraphicsItem*>(gItem);

    if (rgi)
    {
        auto svgWidth = mXmlReader.attributes().value("width");
        auto svgHeight = mXmlReader.attributes().value("height");

        if (!svgWidth.isNull() && !svgHeight.isNull())
        {
            rgi->resize(svgWidth.toString().toFloat(), svgHeight.toString().toFloat());
        }
    }

    bool hasZValue;
    qreal zValue = normalizedZValue(&hasZValue);

    if (hasZValue)
    {
        UBGraphicsItem::assignZValue(gItem, zValue);
    }

    UBItem* ubItem = dynamic_cast<UBItem*>(gItem);

    if (ubItem)
    {
        auto ubUuid = mXmlReader.attributes().value(mNamespaceUri, "uuid");

        if (!ubUuid.isNull())
            ubItem->setUuid(QUuid(ubUuid.toString()));
        else
            ubItem->setUuid(QUuid::createUuid());

        auto ubSource = mXmlReader.attributes().value(mNamespaceUri, "source");

        if (!ubSource.isNull())
            ubItem->setSourceUrl(QUrl(ubSource.toString()));
    }

    auto ubLocked = mXmlReader.attributes().value(mNamespaceUri, "locked");

    if (!ubLocked.isNull())
    {
        bool isLocked = (ubLocked.toString() == xmlTrue || ubLocked.toString() == "1");
        gItem->setData(UBGraphicsItemData::ItemLocked, QVariant(isLocked));
    }

    auto ubHiddenOnDisplay = mXmlReader.attributes().value(mNamespaceUri, "hidden-on-display");

    if (!ubHiddenOnDisplay.isNull())
    {
        bool isHiddenOnDisplay = (ubHiddenOnDisplay.toString() == xmlTrue || ubHiddenOnDisplay.toString() == "1");
        gItem->setData(UBGraphicsItemData::ItemIsHiddenOnDisplay, QVariant(isHiddenOnDisplay));
    }

    auto ubEditable = mXmlReader.attributes().value(mNamespaceUri, "editable");

    if (!ubEditable.isNull())
    {
        bool isEditable = (ubEditable.toString() == xmlTrue || ubEditable.toString() == "1");
        gItem->setData(UBGraphicsItemData::ItemEditable, QVariant(isEditable));
    }

    //deprecated as of 4.4.a.12
    auto ubLayer = mXmlReader.attributes().value(mNamespaceUri, "layer");
    if (!ubLayer.isNull())
    {
        bool ok;
        int layerAsInt = ubLayer.toString().toInt(&ok);

        if (ok)
            gItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(layerAsInt));
    }
}

qreal UBSvgSubsetAdaptor::UBSvgSubsetReader::normalizedZValue(bool* hasValue)
{
    auto ubZValue = mXmlReader.attributes().value(mNamespaceUri, "z-value");
    qreal zValue = 0;

    if (!ubZValue.isNull())
    {
        zValue = ubZValue.toFloat(hasValue);

        // FIX: In the first zvalue implemenations values outside of the boudaries have been used.
        // No boundaries specified on documentation but too small values are not correctly handled.
        // Note: zValue could even be -inf, so we're better off with some arbitrary value instead
        // of doing any calculations.
        if (zValue < -999999)
        {
            zValue = -999999;
        }
    }
    else
    {
        *hasValue = false;
    }

    return zValue;
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::graphicsItemToSvg(QGraphicsItem* item)
{
    mXmlWriter.writeAttribute("x", "0");
    mXmlWriter.writeAttribute("y", "0");

    QRectF rect = item->boundingRect();

    QAbstractGraphicsShapeItem* shapeItem = dynamic_cast<QAbstractGraphicsShapeItem*>(item);

    if (shapeItem && shapeItem->pen().style() != Qt::NoPen)
    {
        qreal margin = shapeItem->pen().widthF() / 2.f;
        rect -= QMarginsF(margin, margin, margin, margin);
    }

    mXmlWriter.writeAttribute("width", QString("%1").arg(rect.width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(rect.height()));

    mXmlWriter.writeAttribute("transform", toSvgTransform(item->sceneTransform()));

    QString zs;
    zs.setNum(item->zValue(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", zs);

    bool isBackground = mScene->isBackgroundObject(item);

    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "background", isBackground ? xmlTrue : xmlFalse);

    UBItem* ubItem = dynamic_cast<UBItem*>(item);

    if (ubItem)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(ubItem->uuid()));

        QUrl sourceUrl = ubItem->sourceUrl();

        if (!sourceUrl.isEmpty())
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "source", sourceUrl.path());
    }

    QVariant layer = item->data(UBGraphicsItemData::ItemLayerType);
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "layer", QString("%1").arg(layer.toInt()));

    QVariant locked = item->data(UBGraphicsItemData::ItemLocked);
    if (!locked.isNull() && locked.toBool())
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "locked", xmlTrue);

    QVariant hiddenOnDisplay = item->data(UBGraphicsItemData::ItemIsHiddenOnDisplay);
    if (!hiddenOnDisplay.isNull() && hiddenOnDisplay.toBool())
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "hidden-on-display", xmlTrue);
    else
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "hidden-on-display", xmlFalse);

    QVariant editable = item->data(UBGraphicsItemData::ItemEditable);
    if (!editable.isNull()) {
        if (editable.toBool())
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "editable", xmlTrue);
        else
            mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "editable", xmlFalse);
    }
}



// NOTE @letsfindaway obsolete
void UBSvgSubsetAdaptor::UBSvgSubsetWriter::graphicsAppleWidgetToSvg(UBGraphicsAppleWidgetItem* item)
{
    graphicsWidgetToSvg(item);
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::graphicsW3CWidgetToSvg(UBGraphicsW3CWidgetItem* item)
{
    graphicsWidgetToSvg(item);
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::graphicsWidgetToSvg(UBGraphicsWidgetItem* item)
{
    QUrl widgetRootUrl = item->widgetUrl();
    QString widgetDirectoryPath = UBPersistenceManager::widgetDirectory;
    if (widgetRootUrl.toString().startsWith("file://"))
    {
        QString widgetRootDir = widgetRootUrl.toLocalFile();
        QFileInfo fi(widgetRootDir);
        QString extension = fi.suffix();

        QString widgetTargetDir = widgetDirectoryPath + "/" + item->uuid().toString() + "." + extension;

        QString path = mDocumentPath + "/" + widgetTargetDir;
        QDir dir(path);

        if (!dir.exists(path))
        {
            QDir dir;
            dir.mkpath(path);
            UBFileSystemUtils::copyDir(widgetRootDir, path);
        }

        // save snapshot of widget
        item->saveSnapshot();

        widgetRootUrl = widgetTargetDir;
    }

    QString widgetPath = "widgets/" + widgetRootUrl.fileName();

    mXmlWriter.writeStartElement("foreignObject");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "src", widgetPath);

    graphicsItemToSvg(item);

    if (item->isFrozen())
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "frozen", xmlTrue);
    }

    mXmlWriter.writeStartElement(nsXHtml, "iframe");

    mXmlWriter.writeAttribute("style", "border: none");
    QRectF rect = item->boundingRect() - QMarginsF(0.5, 0.5, 0.5, 0.5);
    mXmlWriter.writeAttribute("width", QString("%1").arg(rect.width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(rect.height()));

    QString startFileUrl;
    if (item->mainHtmlFileName().startsWith("http://"))
        startFileUrl = item->mainHtmlFileName();
    else
        startFileUrl = widgetRootUrl.toString() + "/" + item->mainHtmlFileName();

    startFileUrl = QUrl::fromPercentEncoding(startFileUrl.toUtf8());

    mXmlWriter.writeAttribute("src", startFileUrl);
    mXmlWriter.writeEndElement(); //iFrame

    //persists widget state
    QMap<QString, QString> preferences = item->preferences();

    foreach(QString key, preferences.keys())
    {
        QString value = preferences.value(key);

        mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "preference");

        mXmlWriter.writeAttribute("key", key);
        mXmlWriter.writeAttribute("value", value);

        mXmlWriter.writeEndElement(); //ub::preference
    }

    //persists datastore state
    QMap<QString, QString> datastore = item->datastoreEntries();

    foreach(QString key, datastore.keys())
    {
        QString value = datastore.value(key);

        mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "datastoreEntry");

        mXmlWriter.writeAttribute("key", key);
        mXmlWriter.writeAttribute("value", value);

        mXmlWriter.writeEndElement(); //ub::datastoreEntry
    }

    mXmlWriter.writeEndElement();
}

// NOTE @letsfindaway obsolete
UBGraphicsAppleWidgetItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::graphicsAppleWidgetFromSvg()
{

    auto widgetUrl = mXmlReader.attributes().value(mNamespaceUri, "src");

    if (widgetUrl.isNull())
    {
        qWarning() << "cannot make sens of widget src value";
        return 0;
    }

    QString href = widgetUrl.toString();

    QUrl url(href);

    if (url.isRelative())
    {
        href = mDocumentPath + "/" + UBFileSystemUtils::normalizeFilePath(widgetUrl.toString());
    }

    UBGraphicsAppleWidgetItem* widgetItem = new UBGraphicsAppleWidgetItem(QUrl::fromLocalFile(href));

    graphicsItemFromSvg(widgetItem);

    return widgetItem;
}

UBGraphicsW3CWidgetItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::graphicsW3CWidgetFromSvg()
{
    auto widgetUrl = mXmlReader.attributes().value(mNamespaceUri, "src");

    if (widgetUrl.isNull())
    {
        qWarning() << "cannot make sense of widget src value";
        return 0;
    }

    QString href = widgetUrl.toString();
    QUrl url(href);

    if (url.isRelative())
    {
        href = mDocumentPath + "/" + UBFileSystemUtils::normalizeFilePath(widgetUrl.toString());
    }

    UBGraphicsW3CWidgetItem* widgetItem = new UBGraphicsW3CWidgetItem(QUrl::fromLocalFile(href));
    widgetItem->setWebActive(false);

    auto uuid = mXmlReader.attributes().value(mNamespaceUri, "uuid");
    QString pixPath = mDocumentPath + "/" + UBPersistenceManager::widgetDirectory + "/" + uuid.toString() + ".png";
    widgetItem->setSnapshotPath(QUrl::fromLocalFile(pixPath));

    QPixmap snapshot(pixPath);

    auto frozen = mXmlReader.attributes().value(mNamespaceUri, "frozen");

    if (!snapshot.isNull())
    {
        widgetItem->setSnapshot(snapshot, !frozen.isNull() && frozen.toString() == xmlTrue);
    }

    graphicsItemFromSvg(widgetItem);

    return widgetItem;
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::textItemToSvg(UBGraphicsTextItem* item)
{
    /**
     * sample
     *
        <foreignObject x="0" y="0" width="489.297" height="76.3303" transform="matrix(0.508456, -0.861088, 0.861088, 0.508456, -181.721, 243.124)" ub:z-value="-9999998.000000" ub:background="false" ub:fill-on-dark-background="#81ff5c" ub:fill-on-light-background="#008000">
            <xhtml:body>
                <xhtml:div>
                    <xhtml:font face="Arial" style="font-size:48px;" color="#81ff5c">this a text<br/>on 2 lines</xhtml:font>
                </xhtml:div>
            </xhtml:body>
        </foreignObject>
     */

    mXmlWriter.writeStartElement("foreignObject");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "type", "text");

    graphicsItemToSvg(item);

    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "width", QString("%1").arg(item->textWidth()));
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "height", QString("%1").arg(item->textHeight()));
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "pixels-per-point", QString("%1").arg(item->pixelsPerPoint()));

    QColor colorDarkBg = item->colorOnDarkBackground();
    QColor colorLightBg = item->colorOnLightBackground();

    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri
                              , "fill-on-dark-background", colorDarkBg.name());
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri
                              , "fill-on-light-background", colorLightBg.name());

    //for new documents from version 4.5.0
    mXmlWriter.writeStartElement("itemTextContent");

    // Note: don't use mXmlWriter.writeCDATA(htmlString); because it doesn't escape characters sequences correctly.
    // Texts copied from other programs like Open-Office can truncate the svg file.
    //mXmlWriter.writeCharacters(item->toHtml());
    mXmlWriter.writeCharacters(UBTextTools::cleanHtmlCData(item->toHtml()));
    mXmlWriter.writeEndElement(); //itemTextContent

    mXmlWriter.writeEndElement(); //foreignObject
}

UBGraphicsTextItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::textItemFromSvg()
{
    qreal width = mXmlReader.attributes().value("width").toString().toFloat();
    qreal height = mXmlReader.attributes().value("height").toString().toFloat();

    qreal originalPixelsPerPoint = mXmlReader.attributes().value(mNamespaceUri, "pixels-per-point").toString().toDouble();

    UBGraphicsTextItem* textItem = new UBGraphicsTextItem();

    graphicsItemFromSvg(textItem);

    auto ubFillOnDarkBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-dark-background");
    auto ubFillOnLightBackground = mXmlReader.attributes().value(mNamespaceUri, "fill-on-light-background");

    if (!ubFillOnDarkBackground.isNull()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QColor color = QColor::fromString(ubFillOnDarkBackground.toString());
#else
        QColor color;
        color.setNamedColor(ubFillOnDarkBackground.toString());
#endif
        if (!color.isValid())
            color = Qt::white;
        textItem->setColorOnDarkBackground(color);
    }

    if (!ubFillOnLightBackground.isNull()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QColor color = QColor::fromString(ubFillOnLightBackground.toString());
#else
        QColor color;
        color.setNamedColor(ubFillOnLightBackground.toString());
#endif
        if (!color.isValid())
            color = Qt::black;
        textItem->setColorOnLightBackground(color);
    }

    QString text;
    QString name;

    while (!(mXmlReader.isEndElement() && ((name = mXmlReader.name().toString()) == "font" || name == "foreignObject")))
    {
        if (mXmlReader.hasError())
        {
            delete textItem;
            textItem = 0;
            return 0;
        }

        mXmlReader.readNext();
        if (mXmlReader.isStartElement())
        {
            //for new documents from version 4.5.0
            if (mFileVersion >= 40500) {
                if (mXmlReader.name().toString() == "itemTextContent") {
                    text = mXmlReader.readElementText();
                    textItem->setHtml(text);

                    // Fonts sizes are not displayed the same across platforms: e.g a text item with the same
                    // font size (in Pts) is displayed smaller on Linux than Windows. This messes up layouts
                    // when importing documents created on another computer, so if a font is being displayed
                    // at a different size (relative to the rest of the document) than it was when created,
                    // we adjust its size.
                    if (originalPixelsPerPoint != 0) {
                        qreal pixelsPerPoint = textItem->pixelsPerPoint();

                        qDebug() << "Pixels per point: original/current" << originalPixelsPerPoint
                                 << "/" << pixelsPerPoint;
                        qreal ratio = originalPixelsPerPoint/pixelsPerPoint;

                        if (ratio != 1) {
                            qDebug() << "Scaling text by " << ratio;
                            UBGraphicsTextItemDelegate* textDelegate = dynamic_cast<UBGraphicsTextItemDelegate*>(textItem->Delegate());
                            if (textDelegate)
                                textDelegate->scaleTextSize(ratio);
                        }
                    }
                    else
                        // mark scene as modified so the text item will be saved with a pixelsPerPoint value
                        saveSceneAfterLoading = true;


                    textItem->resize(width, height);
                    if (textItem->toPlainText().isEmpty()) {
                        delete textItem;
                        textItem = 0;
                    }
                    return textItem;
                }

                //tracking for backward capability with older versions
            } else if (mXmlReader.name().toString() == "font")  {
                QFont font = textItem->font();

                auto fontFamily = mXmlReader.attributes().value("face");

                if (!fontFamily.isNull()) {
                    font.setFamily(fontFamily.toString());
                }
                auto fontStyle = mXmlReader.attributes().value("style");
                if (!fontStyle.isNull()) {
                    foreach (QString styleToken, fontStyle.toString().split(";")) {
                        styleToken = styleToken.trimmed();
                        if (styleToken.startsWith(sFontSizePrefix) && styleToken.endsWith(sPixelUnit)) {
                            int fontSize = styleToken.mid(
                                        sFontSizePrefix.length(),
                                        styleToken.length() - sFontSizePrefix.length() - sPixelUnit.length()).toInt();
                            font.setPixelSize(fontSize);
                        } else if (styleToken.startsWith(sFontWeightPrefix)) {
                            QString fontWeight = styleToken.mid(
                                        sFontWeightPrefix.length(),
                                        styleToken.length() - sFontWeightPrefix.length());
                            font.setBold(fontWeight.contains("bold"));
                        } else if (styleToken.startsWith(sFontStylePrefix)) {
                            QString fontStyle = styleToken.mid(
                                        sFontStylePrefix.length(),
                                        styleToken.length() - sFontStylePrefix.length());
                            font.setItalic(fontStyle.contains("italic"));
                        }
                    }
                }

                QTextCursor curCursor = textItem->textCursor();
                QTextCharFormat format;

                format.setFont(font);
                curCursor.mergeCharFormat(format);
                textItem->setTextCursor(curCursor);
                textItem->setFont(font);

                auto fill = mXmlReader.attributes().value("color");
                if (!fill.isNull()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
                    QColor textColor = QColor::fromString(fill.toString());
#else
                    QColor textColor;
                    textColor.setNamedColor(fill.toString());
#endif
                    textItem->setDefaultTextColor(textColor);
                }

                while (!(mXmlReader.isEndElement() && mXmlReader.name().toString() == "font")) {
                    if (mXmlReader.hasError()) {
                        break;
                    }
                    QXmlStreamReader::TokenType tt = mXmlReader.readNext();
                    if (tt == QXmlStreamReader::Characters) {
                        text += mXmlReader.text().toString();
                    }

                    if (mXmlReader.isStartElement() && mXmlReader.name().toString() == "br") {
                        text += "\n";
                    }
                }
            }
        }
    }

    if (text.isEmpty()) {
        delete textItem;
        textItem = 0;
    } else {
        textItem->setPlainText(text);
        textItem->resize(width, height);
    }

    return textItem;
}




void UBSvgSubsetAdaptor::UBSvgSubsetWriter::curtainItemToSvg(UBGraphicsCurtainItem* curtainItem)
{
    /**
     *
     * sample
     *
      <ub:curtain x="250" y="150" width="122" height="67"...>
      </ub:curtain>
     */

    mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "curtain");
    QRectF rect = curtainItem->boundingRect() - QMarginsF(0.5, 0.5, 0.5, 0.5);
    mXmlWriter.writeAttribute("x", QString("%1").arg(rect.center().x()));
    mXmlWriter.writeAttribute("y", QString("%1").arg(rect.center().y()));
    mXmlWriter.writeAttribute("width", QString("%1").arg(rect.width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(rect.height()));
    mXmlWriter.writeAttribute("transform", toSvgTransform(curtainItem->sceneTransform()));

    //graphicsItemToSvg(curtainItem);
    QString zs;
    zs.setNum(curtainItem->zValue(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", zs);

    UBItem* ubItem = dynamic_cast<UBItem*>(curtainItem);

    if (ubItem)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(ubItem->uuid()));
    }

    mXmlWriter.writeEndElement();
}


UBGraphicsCurtainItem* UBSvgSubsetAdaptor::UBSvgSubsetReader::curtainItemFromSvg()
{
    UBGraphicsCurtainItem* curtainItem = new UBGraphicsCurtainItem();

    graphicsItemFromSvg(curtainItem);

    auto svgX = mXmlReader.attributes().value("x");
    auto svgY = mXmlReader.attributes().value("y");
    auto svgWidth = mXmlReader.attributes().value("width");
    auto svgHeight = mXmlReader.attributes().value("height");


    QRect rect;
    rect.setX(svgX.toString().toFloat()-svgWidth.toString().toFloat()/2);
    rect.setY(svgY.toString().toFloat()-svgHeight.toString().toFloat()/2);
    rect.setWidth(svgWidth.toString().toFloat());
    rect.setHeight(svgHeight.toString().toFloat());

    curtainItem->setRect(rect);

    curtainItem->setVisible(true);

    return curtainItem;
}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::rulerToSvg(UBGraphicsRuler* item)
{

    /**
     *
     * sample
     *
      <ub:ruler x="250" y="150" width="122" height="67"...>
      </ub:ruler>
     */

    mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "ruler");
    QRectF rect = item->boundingRect() - QMarginsF(0.5, 0.5, 0.5, 0.5);
    mXmlWriter.writeAttribute("x", QString("%1").arg(rect.x()));
    mXmlWriter.writeAttribute("y", QString("%1").arg(rect.y()));
    mXmlWriter.writeAttribute("width", QString("%1").arg(rect.width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(rect.height()));
    mXmlWriter.writeAttribute("transform", toSvgTransform(item->sceneTransform()));

    QString zs;
    zs.setNum(item->zValue(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", zs);

    UBItem* ubItem = dynamic_cast<UBItem*>(item);

    if (ubItem)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(ubItem->uuid()));
    }

    mXmlWriter.writeEndElement();
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::axesToSvg(UBGraphicsAxes *item)
{

    /**
     *
     * sample
     *
      <ub:axes x="250" y="150" left="65" top="28" width="122" height="67" numbers="1"...>
      </ub:ruler>
     */

    mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "axes");
    mXmlWriter.writeAttribute("x", QString("%1").arg(item->pos().x()));
    mXmlWriter.writeAttribute("y", QString("%1").arg(item->pos().y()));
    mXmlWriter.writeAttribute("left", QString("%1").arg(item->bounds().left()));
    mXmlWriter.writeAttribute("top", QString("%1").arg(item->bounds().top()));
    mXmlWriter.writeAttribute("width", QString("%1").arg(item->bounds().width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(item->bounds().height()));
    mXmlWriter.writeAttribute("numbers", QString("%1").arg(item->showNumbes()));

    QString zs;
    zs.setNum(item->zValue(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", zs);

    UBItem* ubItem = dynamic_cast<UBItem*>(item);

    if (ubItem)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(ubItem->uuid()));
    }

    mXmlWriter.writeEndElement();
}


UBGraphicsRuler* UBSvgSubsetAdaptor::UBSvgSubsetReader::rulerFromSvg()
{
    UBGraphicsRuler* ruler = new UBGraphicsRuler();

    graphicsItemFromSvg(ruler);

    ruler->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    auto svgWidth = mXmlReader.attributes().value("width");
    auto svgHeight = mXmlReader.attributes().value("height");
    auto svgX = mXmlReader.attributes().value("x");
    auto svgY = mXmlReader.attributes().value("y");

    if (!svgWidth.isNull() && !svgHeight.isNull() && !svgX.isNull() && !svgY.isNull())
    {
        ruler->setRect(svgX.toString().toFloat(), svgY.toString().toFloat(),  svgWidth.toString().toFloat(), svgHeight.toString().toFloat());
    }

    ruler->setVisible(true);

    return ruler;
}

UBGraphicsAxes *UBSvgSubsetAdaptor::UBSvgSubsetReader::axesFromSvg()
{
    UBGraphicsAxes* axes = new UBGraphicsAxes();

    graphicsItemFromSvg(axes);

    axes->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    auto svgX = mXmlReader.attributes().value("x");
    auto svgY = mXmlReader.attributes().value("y");
    auto svgLeft = mXmlReader.attributes().value("left");
    auto svgTop = mXmlReader.attributes().value("top");
    auto svgWidth = mXmlReader.attributes().value("width");
    auto svgHeight = mXmlReader.attributes().value("height");
    auto svgNumbers = mXmlReader.attributes().value("numbers");

    if (!svgX.isNull() && !svgY.isNull())
    {
        axes->setPos(svgX.toString().toFloat(), svgY.toString().toFloat());
    }

    if (!svgWidth.isNull() && !svgHeight.isNull() && !svgLeft.isNull() && !svgTop.isNull())
    {
        axes->setRect(svgLeft.toString().toFloat(), svgTop.toString().toFloat(),
                      svgWidth.toString().toFloat(), svgHeight.toString().toFloat());
    }

    if (!svgNumbers.isNull())
    {
        axes->setShowNumbers(svgNumbers.toInt());
    }

    axes->setVisible(true);

    return axes;
}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::compassToSvg(UBGraphicsCompass* item)
{

    /**
     *
     * sample
     *
      <ub:compass x="250" y="150" width="122" height="67"...>
      </ub:compass>
     */

    mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "compass");
    QRectF rect = item->boundingRect() - QMarginsF(0.5, 0.5, 0.5, 0.5);
    mXmlWriter.writeAttribute("x", QString("%1").arg(rect.x()));
    mXmlWriter.writeAttribute("y", QString("%1").arg(rect.y()));
    mXmlWriter.writeAttribute("width", QString("%1").arg(rect.width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(rect.height()));
    mXmlWriter.writeAttribute("transform", toSvgTransform(item->sceneTransform()));

    QString zs;
    zs.setNum(item->zValue(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", zs);

    UBItem* ubItem = dynamic_cast<UBItem*>(item);

    if (ubItem)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(ubItem->uuid()));
    }

    mXmlWriter.writeEndElement();
}


UBGraphicsCompass* UBSvgSubsetAdaptor::UBSvgSubsetReader::compassFromSvg()
{
    UBGraphicsCompass* compass = new UBGraphicsCompass();

    graphicsItemFromSvg(compass);

    compass->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    auto svgX = mXmlReader.attributes().value("x");
    auto svgY = mXmlReader.attributes().value("y");
    auto svgWidth = mXmlReader.attributes().value("width");
    auto svgHeight = mXmlReader.attributes().value("height");

    if (!svgX.isNull() && !svgY.isNull() && !svgWidth.isNull() && !svgHeight.isNull())
    {
        compass->setRect(svgX.toString().toFloat(), svgY.toString().toFloat()
                         , svgWidth.toString().toFloat(), svgHeight.toString().toFloat());
    }

    compass->setVisible(true);

    return compass;
}


void UBSvgSubsetAdaptor::UBSvgSubsetWriter::protractorToSvg(UBGraphicsProtractor* item)
{

    /**
     *
     * sample
     *
      <ub:protractor x="250" y="150" width="122" height="67"...>
      </ub:protractor>
     */

    mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "protractor");

    mXmlWriter.writeAttribute("x", QString("%1").arg(item->rect().x()));
    mXmlWriter.writeAttribute("y", QString("%1").arg(item->rect().y()));
    mXmlWriter.writeAttribute("width", QString("%1").arg(item->rect().width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(item->rect().height()));
    mXmlWriter.writeAttribute("transform", toSvgTransform(item->sceneTransform()));

    QString angle;
    angle.setNum(item->angle(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "angle", angle);
    angle.setNum(item->markerAngle(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "marker-angle", angle);

    QString zs;
    zs.setNum(item->zValue(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", zs);

    UBItem* ubItem = dynamic_cast<UBItem*>(item);

    if (ubItem)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(ubItem->uuid()));
    }

    mXmlWriter.writeEndElement();
}


UBGraphicsProtractor* UBSvgSubsetAdaptor::UBSvgSubsetReader::protractorFromSvg()
{
    UBGraphicsProtractor* protractor = new UBGraphicsProtractor();

    protractor->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    graphicsItemFromSvg(protractor);

    auto angle = mXmlReader.attributes().value(mNamespaceUri, "angle");
    if (!angle.isNull())
    {
        protractor->setAngle(angle.toString().toFloat());
    }

    auto markerAngle = mXmlReader.attributes().value(mNamespaceUri, "marker-angle");
    if (!markerAngle.isNull())
    {
        protractor->setMarkerAngle(markerAngle.toString().toFloat());
    }

    auto svgX = mXmlReader.attributes().value("x");
    auto svgY = mXmlReader.attributes().value("y");
    auto svgWidth = mXmlReader.attributes().value("width");
    auto svgHeight = mXmlReader.attributes().value("height");

    if (!svgX.isNull() && !svgY.isNull() && !svgWidth.isNull() && !svgHeight.isNull())
    {
        protractor->setRect(svgX.toString().toFloat(), svgY.toString().toFloat()
                            , svgWidth.toString().toFloat(), svgHeight.toString().toFloat());
    }

    protractor->setVisible(true);

    return protractor;
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::triangleToSvg(UBGraphicsTriangle *item)
{

    /**
     *
     * sample
     *
      <ub:triangle x="250" y="150" width="122" height="67"...>
      </ub:triangle>
     */

    mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "triangle");
    QRectF rect = item->boundingRect() - QMarginsF(0.5, 0.5, 0.5, 0.5);
    mXmlWriter.writeAttribute("x", QString("%1").arg(rect.x()));
    mXmlWriter.writeAttribute("y", QString("%1").arg(rect.y()));
    mXmlWriter.writeAttribute("width", QString("%1").arg(rect.width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(rect.height()));
    mXmlWriter.writeAttribute("transform", toSvgTransform(item->sceneTransform()));
    mXmlWriter.writeAttribute("orientation", UBGraphicsTriangle::orientationToStr(item->getOrientation()));

    QString zs;
    zs.setNum(item->zValue(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", zs);

    UBItem* ubItem = dynamic_cast<UBItem*>(item);

    if (ubItem)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(ubItem->uuid()));
    }

    mXmlWriter.writeEndElement();
}

UBGraphicsTriangle* UBSvgSubsetAdaptor::UBSvgSubsetReader::triangleFromSvg()
{
    UBGraphicsTriangle* triangle = new UBGraphicsTriangle();

    triangle->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    graphicsItemFromSvg(triangle);

    auto svgX = mXmlReader.attributes().value("x");
    auto svgY = mXmlReader.attributes().value("y");
    auto svgWidth = mXmlReader.attributes().value("width");
    auto svgHeight = mXmlReader.attributes().value("height");

    auto orientationStringRef = mXmlReader.attributes().value("orientation");
    UBGraphicsTriangle::UBGraphicsTriangleOrientation orientation = UBGraphicsTriangle::orientationFromStr(orientationStringRef.toString());
    triangle->setOrientation(orientation);

    if (!svgX.isNull() && !svgY.isNull() && !svgWidth.isNull() && !svgHeight.isNull())
    {
        triangle->setRect(svgX.toString().toFloat(), svgY.toString().toFloat(), svgWidth.toString().toFloat(), svgHeight.toString().toFloat(), orientation);
    }

    triangle->setVisible(true);
    return triangle;
}

UBGraphicsCache* UBSvgSubsetAdaptor::UBSvgSubsetReader::cacheFromSvg()
{
    UBGraphicsCache* pCache = mScene->graphicsCache();
    pCache->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    graphicsItemFromSvg(pCache);

    auto colorR = mXmlReader.attributes().value("colorR");
    auto colorG = mXmlReader.attributes().value("colorG");
    auto colorB = mXmlReader.attributes().value("colorB");
    auto colorA = mXmlReader.attributes().value("colorA");
    auto shape = mXmlReader.attributes().value("shape");
    auto shapeSize = mXmlReader.attributes().value("shapeSize");

    QColor color(colorR.toString().toInt(), colorG.toString().toInt(), colorB.toString().toInt(), colorA.toString().toInt());

    pCache->setMaskColor(color);
    pCache->setShapeWidth(shapeSize.toString().toInt());
    pCache->setMaskShape(static_cast<eMaskShape>(shape.toString().toInt()));

    pCache->setVisible(true);

    return pCache;
}

void UBSvgSubsetAdaptor::UBSvgSubsetWriter::cacheToSvg(UBGraphicsCache* item)
{
    mXmlWriter.writeStartElement(UBSettings::uniboardDocumentNamespaceUri, "cache");

    mXmlWriter.writeAttribute("x", QString("%1").arg(item->rect().x()));
    mXmlWriter.writeAttribute("y", QString("%1").arg(item->rect().y()));
    mXmlWriter.writeAttribute("width", QString("%1").arg(item->rect().width()));
    mXmlWriter.writeAttribute("height", QString("%1").arg(item->rect().height()));
    mXmlWriter.writeAttribute("colorR", QString("%1").arg(item->maskColor().red()));
    mXmlWriter.writeAttribute("colorG", QString("%1").arg(item->maskColor().green()));
    mXmlWriter.writeAttribute("colorB", QString("%1").arg(item->maskColor().blue()));
    mXmlWriter.writeAttribute("colorA", QString("%1").arg(item->maskColor().alpha()));
    mXmlWriter.writeAttribute("shape", QString("%1").arg(item->maskshape()));
    mXmlWriter.writeAttribute("shapeSize", QString("%1").arg(item->shapeWidth()));

    QString zs;
    zs.setNum(item->zValue(), 'f'); // 'f' keeps precision
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", zs);

    UBItem* ubItem = dynamic_cast<UBItem*>(item);

    if (ubItem)
    {
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid", UBStringUtils::toCanonicalUuid(ubItem->uuid()));
    }

    mXmlWriter.writeEndElement();
}

void UBSvgSubsetAdaptor::convertPDFObjectsToImages(std::shared_ptr<UBDocumentProxy> proxy)
{
    for (int i = 0; i < proxy->pageCount(); i++)
    {
        std::shared_ptr<UBGraphicsScene> scene = loadScene(proxy, i);

        if (scene)
        {
            bool foundPDFItem = false;

            foreach(QGraphicsItem* item, scene->items())
            {
                UBGraphicsPDFItem *pdfItem = dynamic_cast<UBGraphicsPDFItem*>(item);

                if (pdfItem)
                {
                    foundPDFItem = true;
                    UBGraphicsPixmapItem* pixmapItem = pdfItem->toPixmapItem();

                    scene->removeItem(pdfItem);
                    scene->addItem(pixmapItem);

                }
            }

            if (foundPDFItem)
            {
                scene->setModified(true);
                persistScene(proxy, scene, i);
            }
        }

    }
}


void UBSvgSubsetAdaptor::convertSvgImagesToImages(std::shared_ptr<UBDocumentProxy> proxy)
{
    for (int i = 0; i < proxy->pageCount(); i++)
    {
        std::shared_ptr<UBGraphicsScene> scene = loadScene(proxy, i);

        if (scene)
        {
            bool foundSvgItem = false;

            foreach(QGraphicsItem* item, scene->items())
            {
                UBGraphicsSvgItem *svgItem = dynamic_cast<UBGraphicsSvgItem*>(item);

                if (svgItem)
                {
                    foundSvgItem = true;
                    UBGraphicsPixmapItem* pixmapItem = svgItem->toPixmapItem();

                    scene->removeItem(svgItem);
                    scene->addItem(pixmapItem);
                }
            }

            if (foundSvgItem)
            {
                scene->setModified(true);
                persistScene(proxy, scene, i);
            }
        }
    }
}

UBSvgSubsetAdaptor::UBSvgReaderContext::UBSvgReaderContext(std::shared_ptr<UBDocumentProxy> proxy, const QByteArray& pXmlData)
{
    reader = new UBSvgSubsetReader(proxy, pXmlData);
    reader->start();
}

UBSvgSubsetAdaptor::UBSvgReaderContext::~UBSvgReaderContext()
{
    delete reader;
}

bool UBSvgSubsetAdaptor::UBSvgReaderContext::isFinished() const
{
    return reader->isFinished();
}

void UBSvgSubsetAdaptor::UBSvgReaderContext::step()
{
    reader->processElement();
}

std::shared_ptr<UBGraphicsScene> UBSvgSubsetAdaptor::UBSvgReaderContext::scene() const
{
    return reader->scene();
}
