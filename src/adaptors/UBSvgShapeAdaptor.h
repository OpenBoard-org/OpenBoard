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


#pragma once

#include "UBSvgSubsetAdaptor.h"

#include "core/UB.h"

// forward
class UB1HEditableGraphicsCircleItem;
class UB1HEditableGraphicsSquareItem;
class UB3HEditableGraphicsEllipseItem;
class UB3HEditableGraphicsRectItem;
class UBAbstractGraphicsItem;
class UBAbstractGraphicsPathItem;
class UBEditableGraphicsRegularShapeItem;

class UBSvgShapeAdaptor : public UBSvgSubsetAdaptor::UBSvgAdaptorExtension
{
public:
    static void registerExtension();

    UBSvgShapeAdaptor();

    virtual UBSvgSubsetAdaptor::UBSvgReaderExtension* createSvgReaderExtension(QXmlStreamReader& xmlReader) override;
    virtual UBSvgSubsetAdaptor::UBSvgWriterExtension* createSvgWriterExtension(QXmlStreamWriter& xmlWriter) override;

    class UBSvgShapeReader : public UBSvgSubsetAdaptor::UBSvgReaderExtension
    {
    public:
        UBSvgShapeReader(QXmlStreamReader& xmlReader);

        virtual void readerExtension(std::shared_ptr<UBGraphicsScene> scene) override;

    private:
        void baseAttributesFromSvg(QGraphicsItem* item);
        void getStyleFromSvg(UBAbstractGraphicsItem* item, const QColor& pDefaultPenColor);
        UB3HEditableGraphicsEllipseItem* shapeEllipseFromSvg(const QColor& pDefaultPenColor);
        UB1HEditableGraphicsCircleItem* shapeCircleFromSvg(const QColor& pDefaultPenColor);
        UB1HEditableGraphicsSquareItem* shapeSquareFromSvg(const QColor& pDefaultPenColor);
        UB3HEditableGraphicsRectItem* shapeRectFromSvg(const QColor& pDefaultPenColor);
        UBAbstractGraphicsPathItem* shapePathFromSvg(const QColor& pDefaultPenColor, ShapeType type);
        UBEditableGraphicsRegularShapeItem* shapeRegularFromSvg(const QColor& pDefaultPenColor);

    private:
        QXmlStreamReader& mXmlReader;
    };

    class UBSvgShapeWriter : public UBSvgSubsetAdaptor::UBSvgWriterExtension
    {
    public:
        UBSvgShapeWriter(QXmlStreamWriter& xmlWriter);

        virtual void writerExtension(QGraphicsItem* item) override;

    private:
        void shapeEllipseToSvg(UB3HEditableGraphicsEllipseItem* item);
        void shapeRectToSvg(UB3HEditableGraphicsRectItem* item);
        void shapeRegularToSvg(UBEditableGraphicsRegularShapeItem* item);
        void shapePathToSvg(UBAbstractGraphicsPathItem* item);
        void shapeSquareToSvg(UB1HEditableGraphicsSquareItem* item);
        void shapeCircleToSvg(UB1HEditableGraphicsCircleItem* item);
        void writeAbstractGraphicsItemStyle(UBAbstractGraphicsItem* item);

    private:
        QXmlStreamWriter& mXmlWriter;
    };
};
