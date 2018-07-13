#include "UBEditableGraphicsRegularShapeItem.h"

#include <cmath>

#include "UBAbstractHandlesBuilder.h"

UBEditableGraphicsRegularShapeItem::UBEditableGraphicsRegularShapeItem(int nVertices, QPointF startPos, QGraphicsItem * parent)
    : UBAbstractEditableGraphicsShapeItem(parent)
    , mNVertices(nVertices)
    , mStartPoint(startPos)
{
    initializeStrokeProperty();
    initializeFillingProperty();
    createGraphicsRegularPathItem();

    UB1HandleBuilder::buildHandles(mHandles);
    mHandles.at(0)->setParentItem(this);
    mHandles.at(0)->setEditableObject(this);
}

UBEditableGraphicsRegularShapeItem::~UBEditableGraphicsRegularShapeItem()
{

}

void UBEditableGraphicsRegularShapeItem::createGraphicsRegularPathItem()
{
    const qreal PI = 3.14159265359;

    qreal pointDepart = 0.0;
    if (mNVertices % 2 == 0 && mNVertices % 3 == 0)
        pointDepart = PI / 3.0;
    else
    {
        if (mNVertices % 2 == 0)
            pointDepart = PI / 4.0;
        else
            pointDepart = PI/2.0;
    }

    mVertices.clear();

    for (int i=0; i < mNVertices; i++)
    {
        qreal angle = pointDepart + qreal(i)*2.0*PI/qreal(mNVertices);
        mVertices.append(QPair<qreal, qreal>(cos(angle), sin(angle)));
    }
}

void UBEditableGraphicsRegularShapeItem::updatePath(QPointF newPos)
{
    prepareGeometryChange();

    QPainterPath path;

    QPointF diff = newPos - mStartPoint;

    qreal minFace = 0, x = diff.x(), y = diff.y();

    int signX = diff.x() < 0 ? -1 : 1;
    int signY = diff.y() < 0 ? -1 : 1;

    if (x < 0)
        x = -x;
    if (y < 0)
        y = -y;

    minFace = qMin(x, y);

    mCenter = QPointF(mStartPoint.x() + minFace * signX / 2.0, mStartPoint.y() + minFace * signY / 2.0);

    mRadius = minFace / 2.0 ;
    QPointF nextPoint = mCenter - QPointF(mVertices.at(0).first * mRadius, mVertices.at(0).second * mRadius);
    QPointF firstPoint = nextPoint;

    path.moveTo(firstPoint);

    for (int i = 1; i < mNVertices; i++)
    {
        nextPoint = mCenter - QPointF(mVertices.at(i).first * mRadius, mVertices.at(i).second * mRadius);

        path.lineTo(nextPoint);
    }

    path.lineTo(firstPoint);
    setPath(path);
}

void UBEditableGraphicsRegularShapeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    painter->fillPath(path(), painter->brush());
    painter->drawPath(path());

    if(isInEditMode()){
        painter->setBrush(QBrush());
        QPen p;

        p.setStyle(Qt::DashLine);

        p.setColor(QColor(128,128,128));
        p.setWidth(pen().width());

        painter->setPen(p);

        painter->drawEllipse(mCenter, mRadius, mRadius);

        p.setColor(QColor(128, 128, 200));
        painter->setPen(p);

        QPainterPath ccircle;
        ccircle.addEllipse(mCenter, mRadius, mRadius);

        painter->drawRect(ccircle.boundingRect());
    }
}

void UBEditableGraphicsRegularShapeItem::setStartPoint(QPointF pos)
{
    mStartPoint = mapToItem(this, pos);
}

QRectF UBEditableGraphicsRegularShapeItem::boundingRect() const
{
    QRectF retour = adjustBoundingRect(path().boundingRect());

    if(isInEditMode()){
        //add the size of the circle
        QPainterPath circle;
        circle.addEllipse(mCenter, mRadius, mRadius);

        qreal r = mHandles.at(0)->radius();

        retour = circle.boundingRect();
        retour = adjustBoundingRect(retour);
        retour.adjust(0, 0, r, r);
    }

    retour.adjust(-1, -1, 1, 1);

    return retour;
}

void UBEditableGraphicsRegularShapeItem::addPoint(const QPointF & point)
{
    QPainterPath painterPath = path();

    if (painterPath.elementCount() == 0)
    {
        painterPath.moveTo(point); // For the first point added, we must use moveTo().
    }
    else
    {
        painterPath.lineTo(point);
    }

    setPath(painterPath);
}

UBItem *UBEditableGraphicsRegularShapeItem::deepCopy() const
{
    UBEditableGraphicsRegularShapeItem * copy = new UBEditableGraphicsRegularShapeItem();

    copyItemParameters(copy);

    return copy;
}

void UBEditableGraphicsRegularShapeItem::copyItemParameters(UBItem *copy) const
{
    UBAbstractEditableGraphicsShapeItem::copyItemParameters(copy);

    UBEditableGraphicsRegularShapeItem *cp = dynamic_cast<UBEditableGraphicsRegularShapeItem*>(copy);

    cp->mVertices = mVertices;
    cp->mNVertices = mNVertices;
    cp->mCenter = mCenter;
    cp->mRadius = mRadius;
    cp->mStartPoint = mStartPoint;
    cp->setPath(path());
}

void UBEditableGraphicsRegularShapeItem::updateHandle(UBAbstractHandle *handle)
{
    prepareGeometryChange();

    /*Delegate()->showFrame(false);*/

    QPointF diff = handle->pos() - path().boundingRect().topLeft();

    qreal maxSize = handle->radius() * 4;

    if(diff.x() < maxSize){
        handle->setX(handle->pos().x() + (maxSize - diff.x()));
    }

    if(diff.y() < maxSize){
        handle->setY(handle->pos().y() + (maxSize - diff.y()));
    }

    updatePath(handle->pos());

    if(hasGradient()){
        QLinearGradient g(path().boundingRect().topLeft(), path().boundingRect().topRight());

        g.setColorAt(0, brush().gradient()->stops().at(0).second);
        g.setColorAt(1, brush().gradient()->stops().at(1).second);

        setBrush(g);
    }

}

void UBEditableGraphicsRegularShapeItem::onActivateEditionMode()
{
    QPainterPath circle;

    circle.addEllipse(mCenter, mRadius, mRadius);

    mHandles.at(0)->setPos(circle.boundingRect().bottomRight());
}

QPainterPath UBEditableGraphicsRegularShapeItem::shape() const
{
    QPainterPath path;

    if(isInEditMode()){
        path.addRect(boundingRect());
    }else{
        path = this->path();
    }

    return path;

}

QPointF UBEditableGraphicsRegularShapeItem::correctStartPoint() const
{
    //the start point must be always in the top left corner
    //so we have to correct its position if it is not in the
    //top left corner (because the shape has maybe been construct
    //in reverse order)

    QPainterPath circle;

    circle.addEllipse(mCenter, mRadius, mRadius);

    return circle.boundingRect().topLeft();
}
