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

    onZoomChanged(UBApplication::boardController->currentZoom());
}

void UBSelectionFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    QRectF shRect = option->rect;
    path.addRoundedRect(shRect, adjThickness() / 2, adjThickness() / 2);

    if (rect().width() > 1 && rect().height() > 1) {
        QPainterPath extruded;
        extruded.addRect(shRect.adjusted(adjThickness(), adjThickness(), (adjThickness() * -1), (adjThickness() * -1)));
        path = path.subtracted(extruded);
    }

    painter->fillPath(path, mLocalBrush);
}

QRectF UBSelectionFrame::boundingRect() const
{
    return rect().adjusted(-adjThickness(), -adjThickness(), adjThickness(), adjThickness());
}

QPainterPath UBSelectionFrame::shape() const
{
    QPainterPath resShape;
    QRectF ownRect = rect();
    QRectF shRect = ownRect.adjusted(-adjThickness(), -adjThickness(), adjThickness(), adjThickness());
    resShape.addRoundedRect(shRect, adjThickness() / 2, adjThickness() / 2);

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
    mButtons.append(mZOrderUpButton);
    mButtons.append(mZOrderDownButton);

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
    qDebug() << "Pzoom" << pZoom;
    qDebug() << "Board current zoom" << UBApplication::boardController->currentZoom();
    qDebug() << "UBApplication::boardController->systemScaleFactor()" << UBApplication::boardController->systemScaleFactor();
    mAntiscaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * pZoom);
//    updateScale();
//    QTransform tr;
//    tr.scale(mAntiscaleRatio, mAntiscaleRatio);

//    this->setTransform(tr);
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
    tr.scale(mAntiscaleRatio, mAntiscaleRatio);
    mDeleteButton->setParentItem(this);
    mDeleteButton->setTransform(tr);

    QRectF frRect = boundingRect();

    qreal topX = frRect.left() - mDeleteButton->renderer()->viewBox().width() * mAntiscaleRatio / 2;
    qreal topY = frRect.top() - mDeleteButton->renderer()->viewBox().height() * mAntiscaleRatio / 2;

    qreal bottomX = topX;
    qreal bottomY = frRect.bottom() - mDeleteButton->renderer()->viewBox().height() * mAntiscaleRatio / 2;

    mDeleteButton->setPos(topX, topY);
    mDeleteButton->show();

    int i = 1, j = 0, k = 0;
    while ((i + j + k) < mButtons.size())  {
        DelegateButton* button = mButtons[i + j];

        if (button->getSection() == Qt::TopLeftSection) {
            button->setParentItem(this);
            button->setPos(topX + (i++ * 1.6 * adjThickness()), topY);
            button->setTransform(tr);
        } else if (button->getSection() == Qt::BottomLeftSection) {
            button->setParentItem(this);
            button->setPos(bottomX + (++j * 1.6 * adjThickness()), bottomY);
            button->setTransform(tr);
        } else {
            ++k;
        }
            button->show();
    }
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


