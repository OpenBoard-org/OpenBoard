/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#include "UBSvgShapeAdaptor.h"

#include "board/UBBoardController.h"

#include "core/UBApplication.h"

#include "domain/UBGraphicsScene.h"
#include "domain/shapes/UB1HEditableGraphicsCircleItem.h"
#include "domain/shapes/UB1HEditableGraphicsSquareItem.h"
#include "domain/shapes/UB3HEditableGraphicsEllipseItem.h"
#include "domain/shapes/UB3HEditableGraphicsRectItem.h"
#include "domain/shapes/UBAbstractGraphicsPathItem.h"
#include "domain/shapes/UBEditableGraphicsLineItem.h"
#include "domain/shapes/UBEditableGraphicsRegularShapeItem.h"

#include "frameworks/UBStringUtils.h"


void UBSvgShapeAdaptor::registerExtension()
{
    UBSvgSubsetAdaptor::registerAdapterExtension(new UBSvgShapeAdaptor);
}

UBSvgShapeAdaptor::UBSvgShapeAdaptor()
{
}

UBSvgSubsetAdaptor::UBSvgReaderExtension* UBSvgShapeAdaptor::createSvgReaderExtension(QXmlStreamReader& xmlReader)
{
    return new UBSvgShapeReader{xmlReader};
}

UBSvgSubsetAdaptor::UBSvgWriterExtension* UBSvgShapeAdaptor::createSvgWriterExtension(QXmlStreamWriter& xmlWriter)
{
    return new UBSvgShapeWriter{xmlWriter};
}

void UBSvgShapeAdaptor::UBSvgShapeReader::readerExtension(std::shared_ptr<UBGraphicsScene> scene)
{
    QString name = mXmlReader.name().toString();
    QStringView shapeTypeString = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "shapeType");
    ShapeType shapeType{ShapeType::None};

    if (!shapeTypeString.isNull())
    {
        shapeType = UBShapeFactory::shapeTypeFromString(shapeTypeString.toString());
    }

    if (name == "ellipse" &&
        (shapeType == ShapeType::Circle || shapeType == ShapeType::Ellipse)) // EV-7 - ALTI/AOU - 20131231
    {
        UBAbstractGraphicsItem* item{nullptr};

        Qt::GlobalColor color = scene->isDarkBackground() ? Qt::white : Qt::black;

        if (shapeType == ShapeType::Circle)
        {
            item = shapeCircleFromSvg(color);
        }
        else
        {
            item = shapeEllipseFromSvg(color);
        }

        scene->addItem(item);
    }
    else if (name == "rect" &&
             (shapeType == ShapeType::Square || shapeType == ShapeType::Rectangle)) // EV-7 - ALTI/CFA - 20131231
    {
        UBAbstractGraphicsItem* item{nullptr};

        Qt::GlobalColor color = scene->isDarkBackground() ? Qt::white : Qt::black;

        if (shapeType == ShapeType::Square)
        {
            item = shapeSquareFromSvg(color);
        }
        else
        {
            item = shapeRectFromSvg(color);
        }

        scene->addItem(item);
    }
    else if (name == "polyline")
    {
        Qt::GlobalColor color = scene->isDarkBackground() ? Qt::white : Qt::black;
        UBAbstractGraphicsItem* pathItem = 0;

        if (shapeType == ShapeType::RegularPolygon)
        {
            pathItem = shapeRegularFromSvg(color);
        }
        else
        {
            pathItem = shapePathFromSvg(color, shapeType);
        }

        if (pathItem)
        {
            scene->addItem(pathItem);
        }
    }
}

void UBSvgShapeAdaptor::UBSvgShapeReader::baseAttributesFromSvg(QGraphicsItem* item)
{
    bool hasZValue;
    auto ubZValue = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "z-value");
    qreal zValue = 0;

    if (!ubZValue.isNull())
    {
        zValue = ubZValue.toFloat(&hasZValue);

        if (zValue < -999999)
        {
            zValue = -999999;
        }
    }
    else
    {
        hasZValue = false;
    }

    if (hasZValue)
    {
        UBGraphicsItem::assignZValue(item, zValue);
    }

    UBItem* ubItem = dynamic_cast<UBItem*>(item);

    if (ubItem)
    {
        auto ubUuid = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "uuid");

        if (!ubUuid.isNull())
            ubItem->setUuid(QUuid(ubUuid.toString()));
        else
            ubItem->setUuid(QUuid::createUuid());
    }
}

void UBSvgShapeAdaptor::UBSvgShapeReader::getStyleFromSvg(UBAbstractGraphicsItem* item, const QColor& pDefaultPenColor)
{
    QPen p = item->pen();
    // Stroke color
    QStringView svgStroke = mXmlReader.attributes().value("stroke");
    QColor strokeColor = pDefaultPenColor;
    if (!svgStroke.isNull())
    {
        strokeColor = QColor::fromString(svgStroke);
    }

    QStringView svgStrokeOpacity = mXmlReader.attributes().value("stroke-opacity");
    if (!svgStrokeOpacity.isNull())
    {
        strokeColor.setAlphaF(svgStrokeOpacity.toString().toInt());
    }

    p.setColor(strokeColor);

    // Stroke width/thickness
    QStringView svgStrokeWidth = mXmlReader.attributes().value("stroke-width");
    double strokeWidth = 2;
    if (!svgStrokeWidth.isNull())
    {
        strokeWidth = svgStrokeWidth.toString().toDouble();
    }

    p.setWidthF(strokeWidth);
    p.setCapStyle(Qt::RoundCap);

    // Stroke style
    UBItemStyle style;
    QStringView svgStrokeStyle = mXmlReader.attributes().value("stroke-dasharray");

    if (!svgStrokeStyle.isNull())
    {
        QStringList strokeValues = svgStrokeStyle.toString().split(" ");
        QList<qreal> pattern;

        for (const auto strokeValue : strokeValues)
        {
            pattern << strokeValue.toDouble() / strokeWidth;
        }

        const auto strokeStyle = UBShapeFactory::styleForPattern(pattern);
        p.setStyle(Qt::CustomDashLine);
        p.setDashPattern(pattern);
        style.setLineStyle(strokeStyle);
    }
    else
    {
        p.setStyle(Qt::SolidLine);
        style.setLineStyle(Qt::SolidLine);
    }

    item->setPen(p);

    QBrush b = item->brush();

    // Fill color
    QStringView svgFill1 = mXmlReader.attributes().value("fill");

    QColor brushColor1 = Qt::transparent;
    if (!svgFill1.isNull())
    {
        brushColor1 = QColor::fromString(svgFill1);
    }

    // Fill opacity (transparency)
    QStringView svgOpacity1 = mXmlReader.attributes().value("fill-opacity");
    qreal opacity1 = 1.0; // opaque
    if (!svgOpacity1.isNull())
    {
        opacity1 = svgOpacity1.toString().toFloat();
    }

    brushColor1.setAlphaF(opacity1);
    b.setColor(brushColor1);

    item->setBrush(b);

    // Transform matrix
    QStringView svgTransform = mXmlReader.attributes().value("transform");
    QTransform itemMatrix;
    if (!svgTransform.isNull())
    {
        itemMatrix = UBSvgSubsetAdaptor::fromSvgTransform(svgTransform.toString());
        item->setTransform(itemMatrix);
    }

    QStringView onLight =
        mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "line-on-light-background");
    QStringView onDark =
        mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "line-on-dark-background");

    if (!onLight.isNull() && !onDark.isNull())
    {
        style.setLineColor(QColor::fromString(onLight), QColor::fromString(onDark));
    }

    style.setLineWidth(strokeWidth);

    onLight = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "fill-on-light-background");
    onDark = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "fill-on-dark-background");

    if (!onLight.isNull() && !onDark.isNull())
    {
        style.setFillColor(QColor::fromString(onLight), QColor::fromString(onDark));
    }
    else
    {
        style.setFillColor(Qt::transparent, Qt::transparent);
    }

    item->setItemStyle(style);
}

UB3HEditableGraphicsEllipseItem*
UBSvgShapeAdaptor::UBSvgShapeReader::shapeEllipseFromSvg(const QColor& pDefaultPenColor) // EV-7 - ALTI/AOU - 20131231
{
    UB3HEditableGraphicsEllipseItem* ellipse = new UB3HEditableGraphicsEllipseItem();
    baseAttributesFromSvg(ellipse);

    qreal rx = 100, ry = 100;
    QStringView svgRx = mXmlReader.attributes().value("rx");
    QStringView svgRy = mXmlReader.attributes().value("ry");

    if (!svgRx.isNull())
        rx = svgRx.toString().toFloat();
    if (!svgRy.isNull())
        ry = svgRy.toString().toFloat();

    ellipse->setRect(QRectF(0, 0, 2 * rx, 2 * ry));

    getStyleFromSvg(ellipse, pDefaultPenColor);

    return ellipse;
}

UB1HEditableGraphicsCircleItem* UBSvgShapeAdaptor::UBSvgShapeReader::shapeCircleFromSvg(const QColor& pDefaultPenColor)
{
    UB1HEditableGraphicsCircleItem* circle = new UB1HEditableGraphicsCircleItem();
    baseAttributesFromSvg(circle);

    qreal rx = 100;
    QStringView svgRx = mXmlReader.attributes().value("rx");

    if (!svgRx.isNull())
        rx = svgRx.toString().toFloat();

    circle->setRect(QRectF(0, 0, 2 * rx, 2 * rx));

    getStyleFromSvg(circle, pDefaultPenColor);

    return circle;
}

UB1HEditableGraphicsSquareItem* UBSvgShapeAdaptor::UBSvgShapeReader::shapeSquareFromSvg(const QColor& pDefaultPenColor)
{
    UB1HEditableGraphicsSquareItem* square = new UB1HEditableGraphicsSquareItem();
    baseAttributesFromSvg(square);

    qreal side = 0;
    QStringView svgS = mXmlReader.attributes().value("width");

    if (!svgS.isNull())
        side = svgS.toString().toFloat();

    square->setRect(QRectF(0, 0, side, side));

    getStyleFromSvg(square, pDefaultPenColor);

    return square;
}

UB3HEditableGraphicsRectItem*
UBSvgShapeAdaptor::UBSvgShapeReader::shapeRectFromSvg(const QColor& pDefaultPenColor) // EV-7 - ALTI/AOU - 20131231
{
    UB3HEditableGraphicsRectItem* rect = new UB3HEditableGraphicsRectItem();
    baseAttributesFromSvg(rect);

    qreal w = 100, h = 100;
    QStringView svgW = mXmlReader.attributes().value("width");
    QStringView svgH = mXmlReader.attributes().value("height");

    if (!svgW.isNull())
        w = svgW.toString().toFloat();
    if (!svgH.isNull())
        h = svgH.toString().toFloat();

    rect->setRect(QRectF(0, 0, w, h));

    getStyleFromSvg(rect, pDefaultPenColor);

    return rect;
}

UBAbstractGraphicsPathItem*
UBSvgShapeAdaptor::UBSvgShapeReader::shapePathFromSvg(const QColor& pDefaultPenColor,
                                                      ShapeType type) // EV-7 - ALTI/AOU - 20140102
{
    UBAbstractGraphicsPathItem* pathItem = 0;

    switch (type)
    {
    case ShapeType::Polygon:
        pathItem = new UBEditableGraphicsPolygonItem();
        break;
    case ShapeType::Line:
        pathItem = new UBEditableGraphicsLineItem();
        break;
    default:
        break;
    }

    if (pathItem)
    {
        baseAttributesFromSvg(pathItem);
    }

    QStringView svgPoints = mXmlReader.attributes().value("points");

    if (pathItem && !svgPoints.isNull())
    {
        QStringList ts = svgPoints.toString().split(QLatin1Char(' '), Qt::SkipEmptyParts);

        foreach (const QString sPoint, ts)
        {
            QStringList sCoord = sPoint.split(QLatin1Char(','), Qt::SkipEmptyParts);

            if (sCoord.size() == 2)
            {
                QPointF point;
                point.setX(sCoord.at(0).toFloat());
                point.setY(sCoord.at(1).toFloat());
                pathItem->addPoint(point);
            }
            else if (sCoord.size() == 4)
            {
                // This is the case on system were the "," is used to seperate decimal
                QPointF point;
                QString x = sCoord.at(0) + "." + sCoord.at(1);
                QString y = sCoord.at(2) + "." + sCoord.at(3);
                point.setX(x.toFloat());
                point.setY(y.toFloat());
                pathItem->addPoint(point);
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

    getStyleFromSvg(pathItem, pDefaultPenColor);

    auto polygonItem = dynamic_cast<UBEditableGraphicsPolygonItem*>(pathItem);

    if (polygonItem)
    {
        polygonItem->setIsInCreationMode(false);
    }

    return pathItem;
}

UBEditableGraphicsRegularShapeItem*
UBSvgShapeAdaptor::UBSvgShapeReader::shapeRegularFromSvg(const QColor& pDefaultPenColor)
{
    QStringView svgPoints = mXmlReader.attributes().value("points");

    QStringView nVertices = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "nVertices");
    QStringView startPointX = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "startPointX");
    QStringView startPointY = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "startPointY");

    QStringView cCenterX = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "cCenterX");
    QStringView cCenterY = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "cCenterY");
    QStringView cRadius = mXmlReader.attributes().value(UBSettings::uniboardDocumentNamespaceUri, "cRadius");

    UBEditableGraphicsRegularShapeItem* pathItem = new UBEditableGraphicsRegularShapeItem(
        nVertices.toString().toInt(), QPointF(startPointX.toString().toFloat(), startPointY.toString().toFloat()));
    baseAttributesFromSvg(pathItem);

    pathItem->setCircumscribedCenterCircle(QPointF(cCenterX.toString().toFloat(), cCenterY.toString().toFloat()));
    pathItem->setCircumscribedRadiusCircle(cRadius.toString().toFloat());

    if (!svgPoints.isNull())
    {
        QStringList ts = svgPoints.toString().split(QLatin1Char(' '), Qt::SkipEmptyParts);

        foreach (const QString sPoint, ts)
        {
            QStringList sCoord = sPoint.split(QLatin1Char(','), Qt::SkipEmptyParts);

            if (sCoord.size() == 2)
            {
                QPointF point;
                point.setX(sCoord.at(0).toFloat());
                point.setY(sCoord.at(1).toFloat());
                pathItem->addPoint(point);
            }
            else if (sCoord.size() == 4)
            {
                // This is the case on system were the "," is used to seperate decimal
                QPointF point;
                QString x = sCoord.at(0) + "." + sCoord.at(1);
                QString y = sCoord.at(2) + "." + sCoord.at(3);
                point.setX(x.toFloat());
                point.setY(y.toFloat());
                pathItem->addPoint(point);
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

    getStyleFromSvg(pathItem, pDefaultPenColor);

    return pathItem;
}


UBSvgShapeAdaptor::UBSvgShapeWriter::UBSvgShapeWriter(QXmlStreamWriter& xmlWriter)
    : mXmlWriter{xmlWriter}
{
}

void UBSvgShapeAdaptor::UBSvgShapeWriter::writerExtension(QGraphicsItem* item)
{
    // Is the item a shape Ellipse ?
    UB3HEditableGraphicsEllipseItem* shapeEllipseItem =
        dynamic_cast<UB3HEditableGraphicsEllipseItem*>(item); // EV-7 - ALTI/AOU - 20131231
    if (shapeEllipseItem && shapeEllipseItem->isVisible())
    {
        shapeEllipseToSvg(shapeEllipseItem);
    }

    // Is the item a shape Rect ?
    UB3HEditableGraphicsRectItem* shapeRectItem =
        dynamic_cast<UB3HEditableGraphicsRectItem*>(item); // EV-7 - ALTI/AOU - 20131231
    if (shapeRectItem && shapeRectItem->isVisible())
    {
        shapeRectToSvg(shapeRectItem);
    }

    // Is the item a shape Square ?
    UB1HEditableGraphicsSquareItem* shapeSquareItem =
        dynamic_cast<UB1HEditableGraphicsSquareItem*>(item); // EV-7 - ALTI/AOU - 20131231
    if (shapeSquareItem && shapeSquareItem->isVisible())
    {
        shapeSquareToSvg(shapeSquareItem);
    }

    // Is the item a shape Circle ?
    UB1HEditableGraphicsCircleItem* shapeCircleItem =
        dynamic_cast<UB1HEditableGraphicsCircleItem*>(item); // EV-7 - ALTI/AOU - 20131231
    if (shapeCircleItem && shapeCircleItem->isVisible())
    {
        shapeCircleToSvg(shapeCircleItem);
    }


    // Is the item a shape Path ? (closed polygon, opened polygon, freehand drawing)
    UBAbstractGraphicsPathItem* shapepathItem =
        dynamic_cast<UBAbstractGraphicsPathItem*>(item); // EV-7 - ALTI/AOU - 20140102
    if (shapepathItem && shapepathItem->isVisible())
    {
        shapePathToSvg(shapepathItem);
    }

    // Is the item a shape Regular Polygon ?
    UBEditableGraphicsRegularShapeItem* shapeRegularItem =
        dynamic_cast<UBEditableGraphicsRegularShapeItem*>(item); // EV-7 - ALTI/AOU - 20140102
    if (shapeRegularItem && shapeRegularItem->isVisible())
    {
        shapeRegularToSvg(shapeRegularItem);
    }
}

void UBSvgShapeAdaptor::UBSvgShapeWriter::shapeEllipseToSvg(
    UB3HEditableGraphicsEllipseItem* item) // EV-7 - ALTI/AOU - 20131231
{
    mXmlWriter.writeStartElement("ellipse");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "shapeType",
                              UBShapeFactory::shapeTypeToString(item->shapeType()));

    // SVG <ellipse> tag :
    mXmlWriter.writeAttribute("cx",
                              QString("%1").arg(item->center().x())); // The <ellipse> SVG tag need center coordinates.
                                                                      // Compute them from boundaries of item.
    mXmlWriter.writeAttribute("cy", QString("%1").arg(item->center().y()));
    mXmlWriter.writeAttribute("rx", QString("%1").arg(item->radiusX()));
    mXmlWriter.writeAttribute("ry", QString("%1").arg(item->radiusY()));

    writeAbstractGraphicsItemStyle(item);

    mXmlWriter.writeEndElement();
}

void UBSvgShapeAdaptor::UBSvgShapeWriter::shapeRectToSvg(
    UB3HEditableGraphicsRectItem* item) // EV-7 - ALTI/AOU - 20131231
{
    mXmlWriter.writeStartElement("rect");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "shapeType",
                              UBShapeFactory::shapeTypeToString(item->shapeType()));

    // SVG <shapeRect> tag :
    mXmlWriter.writeAttribute("x", QString::number(item->rect().x()));
    mXmlWriter.writeAttribute("y", QString::number(item->rect().y()));
    mXmlWriter.writeAttribute("width", QString::number(item->rect().width()));
    mXmlWriter.writeAttribute("height", QString::number(item->rect().height()));

    writeAbstractGraphicsItemStyle(item);

    mXmlWriter.writeEndElement();
}

void UBSvgShapeAdaptor::UBSvgShapeWriter::shapeRegularToSvg(UBEditableGraphicsRegularShapeItem* item)
{
    mXmlWriter.writeStartElement("polyline");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "shapeType",
                              UBShapeFactory::shapeTypeToString(item->shapeType()));

    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "nVertices",
                              QString::number(item->nVertices()));
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "startPointX",
                              QString::number(item->startPoint().x()));
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "startPointY",
                              QString::number(item->startPoint().y()));
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "cCenterX",
                              QString::number(item->circumscribedCenterCircle().x()));
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "cCenterY",
                              QString::number(item->circumscribedCenterCircle().y()));
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "cRadius",
                              QString::number(item->circumscribedRadiusCircle()));

    QString sPoints;
    for (int i = 0; i < item->path().elementCount(); ++i)
    {
        QPainterPath::Element element = item->path().elementAt(i);
        sPoints += QString("%1,%2 ").arg(element.x).arg(element.y);
    }

    mXmlWriter.writeAttribute("points", sPoints);

    writeAbstractGraphicsItemStyle(item);

    mXmlWriter.writeEndElement();
}

void UBSvgShapeAdaptor::UBSvgShapeWriter::shapePathToSvg(UBAbstractGraphicsPathItem* item) // EV-7 - ALTI/AOU - 20140102
{
    mXmlWriter.writeStartElement("polyline");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "shapeType",
                              UBShapeFactory::shapeTypeToString(item->shapeType()));

    QString sPoints;
    for (int i = 0; i < item->path().elementCount(); ++i)
    {
        QPainterPath::Element element = item->path().elementAt(i);
        sPoints += QString("%1,%2 ").arg(element.x).arg(element.y);
    }

    mXmlWriter.writeAttribute("points", sPoints);

    writeAbstractGraphicsItemStyle(item);

    mXmlWriter.writeEndElement();
}

void UBSvgShapeAdaptor::UBSvgShapeWriter::shapeSquareToSvg(UB1HEditableGraphicsSquareItem* item)
{
    mXmlWriter.writeStartElement("rect");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "shapeType",
                              UBShapeFactory::shapeTypeToString(item->shapeType()));

    // SVG <shapeRect> tag :
    mXmlWriter.writeAttribute("x", QString::number(item->rect().x()));
    mXmlWriter.writeAttribute("y", QString::number(item->rect().y()));
    mXmlWriter.writeAttribute("width", QString::number(item->rect().width()));
    mXmlWriter.writeAttribute("height", QString::number(item->rect().height()));

    writeAbstractGraphicsItemStyle(item);

    mXmlWriter.writeEndElement();
}

void UBSvgShapeAdaptor::UBSvgShapeWriter::shapeCircleToSvg(UB1HEditableGraphicsCircleItem* item)
{
    mXmlWriter.writeStartElement("ellipse");
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "shapeType",
                              UBShapeFactory::shapeTypeToString(item->shapeType()));

    // SVG <ellipse> tag :
    mXmlWriter.writeAttribute("cx", QString("%1").arg(item->center().x()));
    mXmlWriter.writeAttribute("cy", QString("%1").arg(item->center().y()));
    mXmlWriter.writeAttribute("rx", QString("%1").arg(item->radius()));
    mXmlWriter.writeAttribute("ry", QString("%1").arg(item->radius()));

    writeAbstractGraphicsItemStyle(item);

    mXmlWriter.writeEndElement();
}

void UBSvgShapeAdaptor::UBSvgShapeWriter::writeAbstractGraphicsItemStyle(UBAbstractGraphicsItem* item)
{
    // Stroke :
    if (item->hasStrokeProperty())
    {
        mXmlWriter.writeAttribute("stroke", QString("%1").arg(item->pen().color().name()));
        mXmlWriter.writeAttribute("stroke-width", QString("%1").arg(item->pen().widthF()));
        mXmlWriter.writeAttribute("stroke-linecap", "round");

        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "line-on-light-background",
                                  item->itemStyle().lineColor(false).name(QColor::HexArgb));
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "line-on-dark-background",
                                  item->itemStyle().lineColor(true).name(QColor::HexArgb));

        const auto pattern = UBShapeFactory::dashPattern(item->itemStyle().lineStyle());

        if (!pattern.isEmpty())
        {
            QStringList dashValues;

            for (const auto value : pattern)
            {
                dashValues << QString::number(value * item->pen().widthF());
            }

            mXmlWriter.writeAttribute("stroke-dasharray", dashValues.join(" "));
        }

        mXmlWriter.writeAttribute("stroke-opacity", QString("%1").arg(item->pen().color().alphaF()));
    }

    // Fill :
    if (item->hasFillingProperty())
    {
        mXmlWriter.writeAttribute("fill", QString("%1").arg(item->brush().color().name()));
        mXmlWriter.writeAttribute("fill-opacity", QString("%1").arg(item->brush().color().alphaF()));
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "fill-style",
                                  QString("%1").arg(item->brush().style()));

        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "fill-on-light-background",
                                  item->itemStyle().fillColor(false).name(QColor::HexArgb));
        mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "fill-on-dark-background",
                                  item->itemStyle().fillColor(true).name(QColor::HexArgb));
    }
    else
        mXmlWriter.writeAttribute("fill", "none");

    // z-value
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "z-value", QString("%1").arg(item->zValue()));

    // uuid
    mXmlWriter.writeAttribute(UBSettings::uniboardDocumentNamespaceUri, "uuid",
                              UBStringUtils::toCanonicalUuid(item->uuid()));

    // transform matrix
    mXmlWriter.writeAttribute("transform", UBSvgSubsetAdaptor::toSvgTransform(item->sceneTransform()));
}

UBSvgShapeAdaptor::UBSvgShapeReader::UBSvgShapeReader(QXmlStreamReader& xmlReader)
    : mXmlReader{xmlReader}
{
}
