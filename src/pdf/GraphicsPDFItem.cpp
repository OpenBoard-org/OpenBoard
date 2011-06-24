
#include "GraphicsPDFItem.h"

#include <qglobal.h>

#include <QtGui/QStyleOptionGraphicsItem>

#include "core/memcheck.h"

GraphicsPDFItem::GraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem *parentItem)
    : QObject(0), QGraphicsItem(parentItem)
    , mRenderer(renderer)
    , mPageNumber(pageNumber)
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    mRenderer->attach();
}

GraphicsPDFItem::~GraphicsPDFItem()
{
    mRenderer->detach();
}

QRectF GraphicsPDFItem::boundingRect() const
{
    if (!mRenderer->isValid())
    {
        return QRectF();
    }

    return QRectF(QPointF(0, 0), mRenderer->pageSizeF(mPageNumber));
}

void GraphicsPDFItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (!mRenderer || !mRenderer->isValid())
    {
        qWarning("GraphicsPDFItem::paint: Invalid renderer");
        return;
    }

    if (option)
        mRenderer->render(painter, mPageNumber, option->exposedRect);
    else
        qWarning("GraphicsPDFItem::paint: option is null, ignoring painting");
}
