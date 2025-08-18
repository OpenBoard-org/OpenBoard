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




#ifndef UBGRAPHICSPDFITEM_H_
#define UBGRAPHICSPDFITEM_H_

#include <QtGui>

#include "UBMediaAssetItem.h"

#include "core/UB.h"
#include "pdf/GraphicsPDFItem.h"

class UBGraphicsItemDelegate;
class UBGraphicsPixmapItem;

class UBGraphicsPDFItem: public GraphicsPDFItem, public UBMediaAssetItem, public UBGraphicsItem
{
    public:
        UBGraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem* parent = 0);
        virtual ~UBGraphicsPDFItem();

        enum { Type = UBGraphicsItemType::PDFItemType };

        virtual int type() const override
        {
            return Type;
        }

        virtual QList<QString> mediaAssets() const override;
        virtual void setMediaAsset(const QString& documentPath, const QString& mediaAsset) override;

        virtual UBItem* deepCopy() const override;

        virtual void copyItemParameters(UBItem *copy) const override;

        virtual void setRenderingQuality(RenderingQuality pRenderingQuality) override;

        virtual void setCacheBehavior(CacheBehavior cacheBehavior) override;

        virtual std::shared_ptr<UBGraphicsScene> scene() override;

        virtual UBGraphicsPixmapItem* toPixmapItem() const;

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
        virtual void updateChild() override;
    private slots:
        void OnRequireUpdate();
};

#endif /* UBGRAPHICSPDFITEM_H_ */
