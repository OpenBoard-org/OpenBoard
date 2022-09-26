#include "UBGraphicsLineItem.h"

#include "frameworks/UBGeometryUtils.h"
#include "UBGraphicsScene.h"
#include "core/memcheck.h"


UBGraphicsLineItem::UBGraphicsLineItem (QGraphicsItem * parent)
    : QGraphicsLineItem(parent)
    , mHasAlpha(false)
    , mOriginalWidth(-1)
    , mIsNominalLine(false)
{
    // NOOP
    initialize();
}

UBGraphicsLineItem::UBGraphicsLineItem (const QLineF & line, QGraphicsItem * parent)
    : QGraphicsLineItem(line, parent)
    , mOriginalWidth(-1)
    , mIsNominalLine(false)
{
    // NOOP
    initialize();
}


UBGraphicsLineItem::UBGraphicsLineItem (const QLineF& pLine, qreal pWidth)
    : QGraphicsLineItem(pLine)
    , mOriginalLine(pLine)
    , mOriginalWidth(pWidth)
    , mIsNominalLine(true)
{
    // NOOP
    initialize();
}

UBGraphicsLineItem::UBGraphicsLineItem (const QLineF& pLine, qreal pStartWidth, qreal pEndWidth)
    : QGraphicsLineItem(pLine)
    , mOriginalLine(pLine)
    , mOriginalWidth(pEndWidth)
    , mIsNominalLine(true)
{
    // NOOP
    initialize();
}


void UBGraphicsLineItem::initialize()
{
    //setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::DrawingItem)); //Necessary to set if we want z value to be assigned correctly
    setDelegate(new UBGraphicsItemDelegate(this, 0, GF_COMMON
                                           | GF_RESPECT_RATIO
                                           | GF_REVOLVABLE
                                           | GF_FLIPPABLE_ALL_AXIS));
    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    mStrokeGroup = new UBGraphicsStrokesGroup();
    mStrokeGroup->setTransform(this->transform());
    UBGraphicsItem::assignZValue(mStrokeGroup, this->zValue());

    if(this->transform().isIdentity())
        this->setTransform(mStrokeGroup->transform());

    mStrokeGroup->addToGroup(this);
}

void UBGraphicsLineItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

UBGraphicsLineItem::~UBGraphicsLineItem()
{

}

void UBGraphicsLineItem::setColor(const QColor& pColor)
{
    QPen pen = QPen(pColor);
    pen.setStyle(style());
    if (style()==Qt::PenStyle::DotLine)
    {
        pen.setCapStyle(Qt::PenCapStyle::RoundCap);
    }
    pen.setWidth(mOriginalWidth);
    QGraphicsLineItem::setPen(pen);

    mHasAlpha = (pColor.alphaF() < 1.0);
}


QColor UBGraphicsLineItem::color() const
{
    return QGraphicsLineItem::pen().color();
}

void UBGraphicsLineItem::setStyle(const Qt::PenStyle& style)
{
    QPen pen = QPen(color());
    if(style == Qt::PenStyle::DashLine)
    {
        QVector <qreal> dashes;
        qreal space = 5;
        dashes << 5 << space;
        pen.setDashPattern(dashes);
    } else
    {
        pen.setStyle(style);
        if (style==Qt::PenStyle::DotLine)
        {
            pen.setCapStyle(Qt::PenCapStyle::RoundCap);
        }
    }
    pen.setWidth(mOriginalWidth);
    QGraphicsLineItem::setPen(pen);
}

Qt::PenStyle UBGraphicsLineItem::style() const
{
    return QGraphicsLineItem::pen().style();
}

QList<QPointF> UBGraphicsLineItem::linePoints()
{
    QList<QPointF> points = QList<QPointF>();
    qreal incr = 1/line().length();
    if (incr<0) incr*=-1;
    if (incr>0)
    {
       for (qreal t = 0; t <= 1; t+=incr)
       {
           points.push_back(line().pointAt(t));
       }
    }
    return points;
}

UBItem* UBGraphicsLineItem::deepCopy() const
{
    UBGraphicsLineItem* copy = new UBGraphicsLineItem(line());
    copyItemParameters(copy);
    return copy;
}


void UBGraphicsLineItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsLineItem *cp = dynamic_cast<UBGraphicsLineItem*>(copy);
    if (cp)
    {
        cp->mOriginalLine = this->mOriginalLine;
        cp->mOriginalWidth = this->mOriginalWidth;
        cp->mIsNominalLine = this->mIsNominalLine;

        cp->setTransform(transform());
        cp->setPos(pos());
        cp->setPen(this->pen());
        cp->mHasAlpha = this->mHasAlpha;

        cp->setColorOnDarkBackground(this->colorOnDarkBackground());
        cp->setColorOnLightBackground(this->colorOnLightBackground());

        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setZValue(this->zValue());
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    }
}

void UBGraphicsLineItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    if(mHasAlpha && scene() && scene()->isLightBackground())
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter->setRenderHints(QPainter::Antialiasing);

    QGraphicsLineItem::paint(painter, option, widget);
    Delegate()->postpaint(painter, &styleOption, widget);
}

UBGraphicsScene* UBGraphicsLineItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsLineItem::scene());
}

void UBGraphicsLineItem::SetDelegate()
{
    Delegate()->createControls();
}

QVariant UBGraphicsLineItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = Delegate()->itemChange(change, value);
        UBGraphicsItem *item = dynamic_cast<UBGraphicsItem*>(this);
        if (item)
        {
            item->Delegate()->positionHandles();
        }

    return QGraphicsItem::itemChange(change, newValue);
}

void UBGraphicsLineItem::setStrokesGroup(UBGraphicsStrokesGroup *group)
{
    mStrokeGroup = group;
}
