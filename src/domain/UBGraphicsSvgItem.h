/*
 * UBGraphicsSvgItem.h
 *
 *  Created on: Oct 25, 2008
 *      Author: luc
 */

#ifndef UBGRAPHICSSVGITEM_H_
#define UBGRAPHICSSVGITEM_H_

#include <QtGui>
#include <QtSvg>

#include "UBItem.h"

#include "core/UB.h"

class UBGraphicsItemDelegate;
class UBGraphicsPixmapItem;

class UBGraphicsSvgItem: public QGraphicsSvgItem, public UBItem, public UBGraphicsItem
{
    public:
        UBGraphicsSvgItem(const QString& pFile, QGraphicsItem* parent = 0);
        UBGraphicsSvgItem(const QByteArray& pFileData, QGraphicsItem* parent = 0);

        void init();

        virtual ~UBGraphicsSvgItem();

        QByteArray fileData() const;

        void setFileData(const QByteArray& pFileData)
        {
            mFileData = pFileData;
        }

        enum { Type = UBGraphicsItemType::SvgItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

        virtual void setRenderingQuality(RenderingQuality pRenderingQuality);

        virtual UBGraphicsScene* scene();

        virtual void remove();

        virtual UBGraphicsPixmapItem* toPixmapItem() const;

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        UBGraphicsItemDelegate* mDelegate;

        QByteArray mFileData;
};

#endif /* UBGRAPHICSSVGITEM_H_ */
