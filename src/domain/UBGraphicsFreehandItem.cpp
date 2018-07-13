#include "UBGraphicsFreehandItem.h"

#include <customWidgets/UBGraphicsItemAction.h>

UBGraphicsFreehandItem::UBGraphicsFreehandItem(QGraphicsItem *parent) :
    UBAbstractGraphicsPathItem(parent)
    , HANDLE_SIZE(20)
    , mIsInCreationMode(true)
{
    initializeStrokeProperty();
}

void UBGraphicsFreehandItem::setClosed(bool closed)
{
    if (closed)
        initializeFillingProperty();
}

void UBGraphicsFreehandItem::addPoint(const QPointF & point)
{
    prepareGeometryChange();

    QPainterPath painterPath = path();
    if (painterPath.elementCount() == 0)
    {
        painterPath.moveTo(point); // For the first point added, we must use moveTo().                
        mStartEndPoint[0] = point;
    }
    else
    {
        painterPath.lineTo(point);
        mStartEndPoint[1] = point;
    }

    setPath(painterPath);

    update();
}

UBItem *UBGraphicsFreehandItem::deepCopy() const
{
    UBGraphicsFreehandItem * copy = new UBGraphicsFreehandItem();

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsFreehandItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsFreehandItem *cp = dynamic_cast<UBGraphicsFreehandItem*>(copy);
    if (cp)
    {
        cp->setPath(QPainterPath(this->path()));
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));

        if(Delegate()->action()){
            if(Delegate()->action()->linkType() == eLinkToAudio){
                UBGraphicsItemPlayAudioAction* audioAction = dynamic_cast<UBGraphicsItemPlayAudioAction*>(Delegate()->action());
                UBGraphicsItemPlayAudioAction* action = new UBGraphicsItemPlayAudioAction(audioAction->fullPath());
                cp->Delegate()->setAction(action);
            }
            else
                cp->Delegate()->setAction(Delegate()->action());
        }


        cp->setBrush(brush());
        cp->setPen(pen());

        cp->setIsInCreationMode(false); // don't show handles for the copy.

		// Apply arrow type to the copy :
        cp->setStartArrowType(startArrowType());
        cp->setEndArrowType(endArrowType());
    }
}

void UBGraphicsFreehandItem::setIsInCreationMode(bool mode)
{
    mIsInCreationMode = mode;
}

/**
 * @brief UBGraphicsFreehandItem::arrowAngle
 * @return Angle of arrow to draw at extremity of the line.
 */
qreal UBGraphicsFreehandItem::arrowAngle(ArrowPosition arrowPosition)
{
    qreal angle = 0;

    // Moyenne de l'angle sur les derniers points
    int nbElements = path().elementCount();

    // When the global direction is, for exemple, to 300 degres,
    // and one of the last angles point to 0 degres,
    // we must not use 0 : we have to use 360 instead.
    // Each 0 value will be replaced by 360 if necessary.

    QPainterPath smallPath;
    int nbEchantillons = 0;
    int nbZero = 0;

    if (arrowPosition == EndArrow)
    {
        int nbPoints = qMin(10, nbElements); // last Elements
        QPainterPath::Element firstElement = path().elementAt(nbElements - nbPoints);
        smallPath.moveTo(firstElement.x, firstElement.y);
        for(int i=nbPoints-1; i>0; i--)
        {
            QPainterPath::Element element = path().elementAt(nbElements - i);
            smallPath.lineTo(element.x, element.y);
            qreal a = smallPath.angleAtPercent(1);

            if (qRound(a) == 0){ // count zeros appart.
                nbZero++;
            }
            else{
                angle += a;
                nbEchantillons++;
            }
        }
    }
    else if (arrowPosition == StartArrow)
    {
        int nbPoints = qMin(10, nbElements); // first Elements
        QPainterPath::Element firstElement = path().elementAt(0);
        smallPath.moveTo(firstElement.x, firstElement.y);
        for(int i=1; i<nbPoints; i++)
        {
            QPainterPath::Element element = path().elementAt(i);
            smallPath.lineTo(element.x, element.y);
            qreal a = smallPath.angleAtPercent(1);

            if (qRound(a) == 0){ // count zeros appart.
                nbZero++;
            }
            else{
                angle += a;
                nbEchantillons++;
            }
        }
    }

    qreal moyenne = 0;
    if (nbEchantillons>0){
        moyenne = angle/nbEchantillons; // global direction (without taking account of zeros).
    }
    angle += nbZero*(moyenne>180?360:0); // Change 0 to 360, if necessary.

    angle /= (nbEchantillons+nbZero);

    return -angle;
}

QRectF UBGraphicsFreehandItem::boundingRect() const
{
    QRectF rect = UBAbstractGraphicsPathItem::boundingRect();

    int enlarge = 0;

    if (mIsInCreationMode)//gerer les poignees aux extremites
        enlarge += HANDLE_SIZE/2;

    rect.adjust(-enlarge, -enlarge, enlarge, enlarge);

    return rect;
}

void UBGraphicsFreehandItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    UBAbstractGraphicsItem::setStyle(painter);       

    painter->drawPath(path());

    if (mIsInCreationMode)
    {
        int hsize = HANDLE_SIZE/2;
        painter->drawEllipse(mStartEndPoint[0].x() - hsize, mStartEndPoint[0].y() - hsize, HANDLE_SIZE, HANDLE_SIZE);

        if(path().elementCount() >= 2)
            painter->drawEllipse(mStartEndPoint[1].x() - hsize, mStartEndPoint[1].y() - hsize, HANDLE_SIZE, HANDLE_SIZE);
    }

    drawArrows();
}
