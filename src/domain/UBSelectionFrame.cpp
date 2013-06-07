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
    , mDeleteButton(0)
    , mDuplicateButton(0)
    , mZOrderUpButton(0)
    , mZOrderDownButton(0)
{
    setLocalBrush(QBrush(UBSettings::paletteColor));
    setPen(Qt::NoPen);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsSelectable | ItemIsMovable);


    mDeleteButton = new DelegateButton(":/images/close.svg", this, 0, Qt::TopLeftSection);
    mButtons << mDeleteButton;
    connect(mDeleteButton, SIGNAL(clicked()), this, SLOT(remove()));

    mDuplicateButton = new DelegateButton(":/images/duplicate.svg", this, 0, Qt::TopLeftSection);
    //            connect(mDuplicateButton, SIGNAL(clicked(bool)), this, SLOT(duplicate()));
    mButtons << mDuplicateButton;

    mZOrderUpButton = new DelegateButton(":/images/z_layer_up.svg", this, 0, Qt::BottomLeftSection);
    mZOrderUpButton->setShowProgressIndicator(true);
    //        connect(mZOrderUpButton, SIGNAL(clicked()), this, SLOT(increaseZLevelUp()));
    //        connect(mZOrderUpButton, SIGNAL(longClicked()), this, SLOT(increaseZlevelTop()));
    mButtons << mZOrderUpButton;

    mZOrderDownButton = new DelegateButton(":/images/z_layer_down.svg", this, 0, Qt::BottomLeftSection);
    mZOrderDownButton->setShowProgressIndicator(true);
    //        connect(mZOrderDownButton, SIGNAL(clicked()), this, SLOT(increaseZLevelDown()));
    //        connect(mZOrderDownButton, SIGNAL(longClicked()), this, SLOT(increaseZlevelBottom()));
    mButtons << mZOrderDownButton;


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
    mButtons.clear();
    mButtons.append(mDeleteButton);

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

    placeButtons();


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

    placeButtons();

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
    mPressedPos = mLastMovedPos = mLastTranslateOffset = QPointF();
//    foreach (UBGraphicsItemDelegate *curDelegate, mEnclosedtems) {
//        qDebug() << "TransformBefore" << curDelegate->delegated()->transform();
//    }
//    QGraphicsRectItem::mouseReleaseEvent(event);
}

void UBSelectionFrame::onZoomChanged(qreal pZoom)
{
//    mAntiscaleRatio = pZoom;
//    updateScale();
}

void UBSelectionFrame::remove()
{
    foreach (UBGraphicsItemDelegate *d, mEnclosedtems) {
        d->remove(true);
    }

    updateRect();
}

void UBSelectionFrame::translateItem(QGraphicsItem */*item*/, const QPointF &/*translatePoint*/)
{
}

void UBSelectionFrame::placeButtons()
{
    QTransform tr;
//    tr.scale(mAntiScaleRatio, mAntiScaleRatio);

    mDeleteButton->setParentItem(this);
//    mDeleteButton->setTransform(tr);

    qreal topX = rect().left() - mDeleteButton->renderer()->viewBox().width() /** mAntiScaleRatio*/ * 1.2;
    qreal topY = rect().top() - mDeleteButton->renderer()->viewBox().height() /** mAntiScaleRatio*/ * 1.2;

    qreal bottomX = rect().left() - mDeleteButton->renderer()->viewBox().width() /** mAntiScaleRatio*/ / 2;
    qreal bottomY = rect().bottom() - mDeleteButton->renderer()->viewBox().height() /** mAntiScaleRatio*/ / 2;

    mDeleteButton->setPos(topX, topY);
//    mDeleteButton->setPos(0, 0);

//    if (!mDeleteButton->scene())
//    {
//        if (scene())
//            scene()->addItem(mDeleteButton);
//    }
    mDeleteButton->show();

//    if (showUpdated)
//        mDeleteButton->show();

//    int i = 1, j = 0, k = 0;
//    while ((i + j + k) < mButtons.size())  {
//        DelegateButton* button = mButtons[i + j];

//        if (button->getSection() == Qt::TopLeftSection) {
//            button->setParentItem(mFrame);
//            button->setPos(topX + (i++ * 1.6 * mFrameWidth * mAntiScaleRatio), topY);
//            button->setTransform(tr);
//        } else if (button->getSection() == Qt::BottomLeftSection) {
//            button->setParentItem(mFrame);
//            button->setPos(bottomX + (++j * 1.6 * mFrameWidth * mAntiScaleRatio), bottomY);
//            button->setTransform(tr);
//        } else if (button->getSection() == Qt::TitleBarArea || button->getSection() == Qt::NoSection){
//            ++k;
//        }
//        if (!button->scene())
//        {
//            if (mDelegated->scene())
//                mDelegated->scene()->addItem(button);
//        }
//        if (showUpdated) {
//            button->show();
//            button->setZValue(delegated()->zValue());
//        }
//    }
}

void UBSelectionFrame::clearButtons()
{
    foreach (DelegateButton *b, mButtons)
    {
        b->setParentItem(0);
        b->hide();
    }

    mButtons.clear();
}


