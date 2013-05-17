#include "UBSelectionFrame.h"

#include <QtGui>

#include "core/UB.h"
#include "domain/UBItem.h"
#include "board/UBBoardController.h"
#include "core/UBSettings.h"
#include "core/UBApplication.h"

UBSelectionFrame::UBSelectionFrame()
    : mThickness(UBSettings::settings()->objectFrameWidth)
    , mAntiscaleRatio(1.0)
{
    setLocalBrush(QBrush(UBSettings::paletteColor));
    setPen(Qt::NoPen);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    setFlags(QGraphicsItem::ItemIsMovable);

    connect(UBApplication::boardController, SIGNAL(zoomChanged(qreal)), this, SLOT(onZoomChanged(qreal)));
}

void UBSelectionFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    QRectF shRect = option->rect;
    path.addRoundedRect(shRect, mThickness / 2, mThickness / 2);

    if (rect().width() > 1 && rect().height() > 1) {
        QPainterPath extruded;
        extruded.addRect(shRect.adjusted(mThickness, mThickness, (mThickness * -1), (mThickness * -1)));
        path = path.subtracted(extruded);
    }

    painter->fillPath(path, mLocalBrush);
}

QRectF UBSelectionFrame::boundingRect() const
{
    return rect().adjusted(-mThickness, -mThickness, mThickness, mThickness);
}

QPainterPath UBSelectionFrame::shape() const
{
    QPainterPath resShape;
    QRectF ownRect = rect();
    QRectF shRect = ownRect.adjusted(-mThickness, -mThickness, mThickness, mThickness);
    resShape.addRoundedRect(shRect, mThickness / 2, mThickness / 2);

    if (rect().width() > 1 && rect().height() > 1) {
        QPainterPath extruded;
        extruded.addRect(ownRect);
        resShape = resShape.subtracted(extruded);
    }

    return resShape;
}

void UBSelectionFrame::setEnclosedItems(const QList<QGraphicsItem*> pGraphicsItems)
{
    QRectF resultRect;
    mEnclosedtems.clear();
    foreach (QGraphicsItem *nextItem, pGraphicsItems) {
        UBGraphicsItemDelegate *nextDelegate = UBGraphicsItem::Delegate(nextItem);
        if (nextDelegate) {
            mEnclosedtems.append(nextDelegate);
            resultRect |= nextItem->boundingRect();
        }
    }

    setRect(resultRect);

    if (resultRect.isEmpty()) {
        hide();
    }
}

void UBSelectionFrame::updateRect()
{
    QRectF result;
    foreach (UBGraphicsItemDelegate *curDelegateItem, mEnclosedtems) {
        result |= curDelegateItem->delegated()->boundingRect();
    }

    setRect(result);

    if (result.isEmpty()) {
        setVisible(false);
    }
}

void UBSelectionFrame::updateScale()
{
    setScale(-UBApplication::boardController->currentZoom());
}

void UBSelectionFrame::onZoomChanged(qreal pZoom)
{
    mAntiscaleRatio = pZoom;
    updateScale();

}
