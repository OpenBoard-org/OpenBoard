/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBGraphicsPolygonItem.h"

#include "frameworks/UBGeometryUtils.h"
#include "UBGraphicsScene.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStroke.h"

#include "core/memcheck.h"

UBGraphicsPolygonItem::UBGraphicsPolygonItem (QGraphicsItem * parent)
    : QGraphicsPolygonItem(parent)
    , mHasAlpha(false)
    , mOriginalWidth(-1)
    , mIsNominalLine(false)
    , mStroke(0)
{
    // NOOP
    initialize();
}


UBGraphicsPolygonItem::UBGraphicsPolygonItem (const QPolygonF & polygon, QGraphicsItem * parent)
    : QGraphicsPolygonItem(polygon, parent)
    , mOriginalWidth(-1)
    , mIsNominalLine(false)
    , mStroke(0)
{
    // NOOP
    initialize();
}


UBGraphicsPolygonItem::UBGraphicsPolygonItem (const QLineF& pLine, qreal pWidth)
    : QGraphicsPolygonItem(UBGeometryUtils::lineToPolygon(pLine, pWidth))
    , mOriginalLine(pLine)
    , mOriginalWidth(pWidth)
    , mIsNominalLine(true)
    , mStroke(0)
{
    // NOOP
    initialize();
}

void UBGraphicsPolygonItem::initialize()
{
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::DrawingItem)); //Necessary to set if we want z value to be assigned correctly
}

void UBGraphicsPolygonItem::clearStroke()
{
	if (mStroke!=NULL)
	{
        mStroke->remove(this);
        if (mStroke->polygons().empty())
            delete mStroke;
        mStroke = NULL;
	}
}

UBGraphicsPolygonItem::~UBGraphicsPolygonItem()
{
    clearStroke();
}


void UBGraphicsPolygonItem::setStroke(UBGraphicsStroke* stroke)
{
    clearStroke();

    mStroke = stroke;
    mStroke->addPolygon(this);
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

QPainterPath UBGraphicsPolygonItem::shape() const
{

    QPainterPath path;
    path.addRect(boundingRect());

    return path;

//    static QPainterPath shapePath = QGraphicsPolygonItem::shape();

//    return shapePath;
}


UBGraphicsScene* UBGraphicsPolygonItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsPolygonItem::scene());
}
