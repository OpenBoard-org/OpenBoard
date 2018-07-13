#include "UB1HEditableGraphicsSquareItem.h"

#include "customWidgets/UBGraphicsItemAction.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

UB1HEditableGraphicsSquareItem::UB1HEditableGraphicsSquareItem(QGraphicsItem* parent)
    : UB1HEditableGraphicsBasicShapeItem(parent)
{
    // Rect has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    hIsNeg = false;
    wIsNeg = false;
}

UB1HEditableGraphicsSquareItem::~UB1HEditableGraphicsSquareItem()
{

}

UBItem *UB1HEditableGraphicsSquareItem::deepCopy() const
{
    UB1HEditableGraphicsSquareItem* copy = new UB1HEditableGraphicsSquareItem();

    copyItemParameters(copy);

    return copy;
}

void UB1HEditableGraphicsSquareItem::copyItemParameters(UBItem *copy) const
{
    UB1HEditableGraphicsBasicShapeItem::copyItemParameters(copy);

    UB1HEditableGraphicsSquareItem *cp = dynamic_cast<UB1HEditableGraphicsSquareItem*>(copy);

    if(!cp) return;

    cp->mSide = mSide;
    cp->hIsNeg = hIsNeg;
    cp->wIsNeg = wIsNeg;
}

void UB1HEditableGraphicsSquareItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    int h = hIsNeg ? -mSide : mSide;
    int w = wIsNeg ? -mSide : mSide;

    painter->drawRect(0, 0, w, h);
}

QPainterPath UB1HEditableGraphicsSquareItem::shape() const
{
    QPainterPath path;

    if(isInEditMode()){
        path.addRect(boundingRect());
    }else{
        int h = hIsNeg ? -mSide : mSide;
        int w = wIsNeg ? -mSide : mSide;

        path.addRect(0, 0, w, h);
    }

    return path;
}

void UB1HEditableGraphicsSquareItem::onActivateEditionMode()
{
    getHandle()->setPos(mSide, mSide);
}

void UB1HEditableGraphicsSquareItem::updateHandle(UBAbstractHandle *handle)
{
    prepareGeometryChange();

    qreal maxSize = handle->radius() * 4;

    qreal side = qMin(handle->pos().x(), handle->pos().y());

    if(side > maxSize){
        mSide = side;
    }

    getHandle()->setPos(mSide, mSide);

    if(hasGradient()){
        QLinearGradient g(QPointF(), QPointF(mSide, 0));

        g.setColorAt(0, brush().gradient()->stops().at(0).second);
        g.setColorAt(1, brush().gradient()->stops().at(1).second);

        setBrush(g);
    }
}

QRectF UB1HEditableGraphicsSquareItem::boundingRect() const
{
    int x = wIsNeg ? -mSide : 0;
    int y = hIsNeg ? -mSide : 0;

    QRectF rect(x, y, mSide, mSide);

    rect = adjustBoundingRect(rect);

    if(isInEditMode()){
        qreal r = mHandles.at(0)->radius();
        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UB1HEditableGraphicsSquareItem::setRect(QRectF rect)
{
    prepareGeometryChange();

    setPos(rect.topLeft());

    qreal w = rect.width();
    qreal h = rect.height();

    wIsNeg = w < 0;
    hIsNeg = h < 0;

    if(wIsNeg) w = -w;
    if(hIsNeg) h = -h;

    mSide = qMin(w, h);

    if(hasGradient()){
        QLinearGradient g(QPointF(), QPointF(mSide, 0));

        g.setColorAt(0, brush().gradient()->stops().at(0).second);
        g.setColorAt(1, brush().gradient()->stops().at(1).second);

        setBrush(g);
    }
}

QRectF UB1HEditableGraphicsSquareItem::rect() const
{
    QRectF r;
    r.setTopLeft(pos());

    r.setWidth(wIsNeg ? -mSide : mSide);
    r.setHeight(hIsNeg ? -mSide : mSide);

    return r;
}
