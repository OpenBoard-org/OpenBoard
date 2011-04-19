
#include "UBGraphicsPolygonItem.h"

#include "frameworks/UBGeometryUtils.h"
#include "UBGraphicsScene.h"


UBGraphicsPolygonItem::UBGraphicsPolygonItem (QGraphicsItem * parent)
    : QGraphicsPolygonItem(parent)
    , mHasAlpha(false)
    , mOriginalWidth(-1)
    , mIsNominalLine(false)
    , mStroke(0)
{
    // NOOP
}


UBGraphicsPolygonItem::UBGraphicsPolygonItem (const QPolygonF & polygon, QGraphicsItem * parent)
    : QGraphicsPolygonItem(polygon, parent)
    , mOriginalWidth(-1)
    , mIsNominalLine(false)
    , mStroke(0)
{
    // NOOP
}


UBGraphicsPolygonItem::UBGraphicsPolygonItem (const QLineF& pLine, qreal pWidth)
    : QGraphicsPolygonItem(UBGeometryUtils::lineToPolygon(pLine, pWidth))
    , mOriginalLine(pLine)
    , mOriginalWidth(pWidth)
    , mIsNominalLine(true)
    , mStroke(0)
{
    // NOOP
}


UBGraphicsPolygonItem::~UBGraphicsPolygonItem()
{
    // NOOP
}


void UBGraphicsPolygonItem::setStroke(UBGraphicsStroke* stroke)
{
    mStroke = stroke;

}

UBGraphicsStroke* UBGraphicsPolygonItem::stroke() const
{
    return mStroke;
}


void UBGraphicsPolygonItem::setColor(const QColor& pColor)
{
    QGraphicsPolygonItem::setBrush(QBrush(pColor));

    if (pColor.alphaF() >= 1.0)
    {
        mHasAlpha = false;
        setPen(Qt::NoPen);
    }
    else
    {
        mHasAlpha = true;

        QColor penColor = pColor;

        // trick QT antialiasing
        // TODO UB 4.x see if we can do better ... it does not behave well with 16 bit color depth
        qreal trickAlpha = pColor.alphaF();

        if (trickAlpha >= 0.2 && trickAlpha < 0.6)
        {
            trickAlpha /= 12;
        }
        else if (trickAlpha < 0.8)
        {
            trickAlpha /= 5;
        }
        else if (trickAlpha < 1.0)
        {
            trickAlpha /= 2;
        }

        penColor.setAlphaF(trickAlpha);
        QGraphicsPolygonItem::setPen(QPen(penColor));
    }
}


QColor UBGraphicsPolygonItem::color() const
{
    return QGraphicsPolygonItem::brush().color();
}


UBItem* UBGraphicsPolygonItem::deepCopy() const
{
    UBGraphicsPolygonItem* copy = deepCopy(this->polygon());
    copy->mOriginalLine = this->mOriginalLine;
    copy->mOriginalWidth = this->mOriginalWidth;
    copy->mIsNominalLine = this->mIsNominalLine;

    return copy;
}


UBGraphicsPolygonItem* UBGraphicsPolygonItem::deepCopy(const QPolygonF& pol) const
{
    UBGraphicsPolygonItem* copy = new UBGraphicsPolygonItem(pol);

    copy->mOriginalLine = QLineF();
    copy->mOriginalWidth = -1;
    copy->mIsNominalLine = false;

    copy->setStroke(this->stroke());
    copy->setGroup(this->group());
    copy->setBrush(this->brush());
    copy->setPen(this->pen());
    copy->mHasAlpha = this->mHasAlpha;

    copy->setZValue(this->zValue());

    copy->setColorOnDarkBackground(this->colorOnDarkBackground());
    copy->setColorOnLightBackground(this->colorOnLightBackground());

    copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));

    // TODO UB 4.7 ... complete all members ?

    return copy;

}


void UBGraphicsPolygonItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{

    if(mHasAlpha && scene() && scene()->isLightBackground())
    {
        painter->setCompositionMode(QPainter::CompositionMode_Darken);
    }

    QGraphicsPolygonItem::paint(painter, option, widget);
}


UBGraphicsScene* UBGraphicsPolygonItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsPolygonItem::scene());
}








