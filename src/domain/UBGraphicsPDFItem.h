
#ifndef UBGRAPHICSPDFITEM_H_
#define UBGRAPHICSPDFITEM_H_

#include <QtGui>

#include "UBItem.h"

#include "core/UB.h"
#include "pdf/GraphicsPDFItem.h"

class UBGraphicsItemDelegate;
class UBGraphicsPixmapItem;

class UBGraphicsPDFItem: public GraphicsPDFItem, public UBItem, public UBGraphicsItem
{
    public:
        UBGraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem* parent = 0);
        virtual ~UBGraphicsPDFItem();

        enum { Type = UBGraphicsItemType::PDFItemType };

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

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        UBGraphicsItemDelegate* mDelegate;
};

#endif /* UBGRAPHICSPDFITEM_H_ */
