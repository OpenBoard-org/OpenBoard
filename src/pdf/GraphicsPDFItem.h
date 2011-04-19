
#ifndef GRAPHICSPDFITEM_H
#define GRAPHICSPDFITEM_H

#include <QtGui/QGraphicsItem>
#include <QtCore/QObject>

#include "PDFRenderer.h"

class GraphicsPDFItem : public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

    public:
        GraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem *parentItem = 0);
        virtual ~GraphicsPDFItem();

        virtual QRectF boundingRect() const;

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        int pageNumber() const { return mPageNumber; }
        QUuid fileUuid() const { return mRenderer->fileUuid(); }
        QByteArray fileData() const { return mRenderer->fileData(); }

    protected:
        PDFRenderer *mRenderer;
        int mPageNumber;
};

#endif // GRAPHICSPDFITEM_H
