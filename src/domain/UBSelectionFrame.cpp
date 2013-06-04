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
    setFlags(QGraphicsItem::ItemSendsGeometryChanges /*| QGraphicsItem::ItemIsSelectable*/ | ItemIsMovable);

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
    QRegion resultRegion;
    mEnclosedtems.clear();
    foreach (QGraphicsItem *nextItem, pGraphicsItems) {
        UBGraphicsItemDelegate *nextDelegate = UBGraphicsItem::Delegate(nextItem);
        if (nextDelegate) {
            mEnclosedtems.append(nextDelegate);
            resultRegion |= nextItem->boundingRegion(nextItem->sceneTransform());
        }
    }

    QRectF resultRect = resultRegion.boundingRect();
    setRect(resultRect);

    if (resultRect.isEmpty()) {
        hide();
    }
}

void UBSelectionFrame::updateRect()
{
    QRegion resultRegion;
    foreach (UBGraphicsItemDelegate *curDelegateItem, mEnclosedtems) {
        resultRegion |= curDelegateItem->delegated()->boundingRegion(curDelegateItem->delegated()->sceneTransform());
    }

    QRectF result = resultRegion.boundingRect();
    setRect(result);

    if (result.isEmpty()) {
        setVisible(false);
    }
}

void UBSelectionFrame::updateScale()
{
    setScale(-UBApplication::boardController->currentZoom());
}

void UBSelectionFrame::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mPressedPos = mLastMovedPos = event->pos();
    mLastTranslateOffset = QPointF();

//    foreach (UBGraphicsItemDelegate *curDelegate, mEnclosedtems) {
//        qDebug() << "TransformBefore" << curDelegate->delegated()->transform();
//     }

//    QGraphicsRectItem::mousePressEvent(event);
}

void UBSelectionFrame::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF dp = event->pos() - mPressedPos;

    foreach (UBGraphicsItemDelegate *curDelegate, mEnclosedtems) {

        QGraphicsItem *item = curDelegate->delegated();
        QTransform ownTransform = item->transform();
        QTransform dTransform(
                    ownTransform.m11()
                    , ownTransform.m12()
                    , ownTransform.m13()

                    , ownTransform.m21()
                    , ownTransform.m22()
                    , ownTransform.m23()

                    , ownTransform.m31() + (dp - mLastTranslateOffset).x()
                    , ownTransform.m32() + (dp - mLastTranslateOffset).y()
                    , ownTransform.m33()
                    );

        item->setTransform(dTransform);
    }

    updateRect();
    mLastMovedPos = event->pos();
    mLastTranslateOffset = dp;
}

void UBSelectionFrame::mouseReleaseEvent(QGraphicsSceneMouseEvent */*event*/)
{
    mPressedPos = mLastMovedPos = QPointF();
//    foreach (UBGraphicsItemDelegate *curDelegate, mEnclosedtems) {
//        qDebug() << "TransformBefore" << curDelegate->delegated()->transform();
//    }
//    QGraphicsRectItem::mouseReleaseEvent(event);
}

void UBSelectionFrame::onZoomChanged(qreal pZoom)
{
    mAntiscaleRatio = pZoom;
    updateScale();
}

void UBSelectionFrame::translateItem(QGraphicsItem */*item*/, const QPointF &/*translatePoint*/)
{
}


