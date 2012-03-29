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
#ifndef UBGRAPHICSPOLYGONITEM_H
#define UBGRAPHICSPOLYGONITEM_H

#include <QtGui>

#include "core/UB.h"
#include "UBItem.h"

class UBItem;
class UBGraphicsScene;
class UBGraphicsStroke;

class UBGraphicsPolygonItem : public QGraphicsPolygonItem, public UBItem
{

    public:

        UBGraphicsPolygonItem(QGraphicsItem * parent = 0 );
        UBGraphicsPolygonItem(const QLineF& line, qreal pWidth);
        UBGraphicsPolygonItem(const QPolygonF & polygon, QGraphicsItem * parent = 0);

        ~UBGraphicsPolygonItem();

        void initialize();

        void setColor(const QColor& color);

        QColor color() const;

        virtual UBGraphicsScene* scene();

        inline void subtract(UBGraphicsPolygonItem *pi)
        {
            if (boundingRect().intersects(pi->boundingRect()))
            {
                QPolygonF subtractedPolygon = polygon().subtracted(pi->polygon());

                if (polygon() != subtractedPolygon)
                {
                    mIsNominalLine = false;
                    QGraphicsPolygonItem::setPolygon(subtractedPolygon);
                }
            }
        }

        inline void subtractIntersecting(UBGraphicsPolygonItem *pi)
        {
            QPolygonF subtractedPolygon = polygon().subtracted(pi->polygon());

            if (polygon() != subtractedPolygon)
            {
                mIsNominalLine = false;
                QGraphicsPolygonItem::setPolygon(subtractedPolygon);
            }
        }

        enum { Type = UBGraphicsItemType::PolygonItemType };

        virtual int type() const
        {
            return Type;
        }

        void setPolygon(const QPolygonF pPolygon)
        {
            mIsNominalLine = false;
            QGraphicsPolygonItem::setPolygon(pPolygon);
        }

        virtual UBItem* deepCopy() const;

        // optimisation (eraser)
        UBGraphicsPolygonItem* deepCopy(const QPolygonF& pol) const;

        QLineF originalLine() { return mOriginalLine;}
        qreal originalWidth() { return mOriginalWidth;}
        bool isNominalLine() {return mIsNominalLine;}

        QColor colorOnDarkBackground() const
        {
            return mColorOnDarkBackground;
        }

        void setColorOnDarkBackground(QColor pColorOnDarkBackground)
        {
            mColorOnDarkBackground = pColorOnDarkBackground;
        }

        QColor colorOnLightBackground() const
        {
            return mColorOnLightBackground;
        }

        void setColorOnLightBackground(QColor pColorOnLightBackground)
        {
            mColorOnLightBackground = pColorOnLightBackground;
        }

        void setStroke(UBGraphicsStroke* stroke);
        UBGraphicsStroke* stroke() const;

    protected:
        void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);


    private:

        void clearStroke();

        bool mHasAlpha;

        QLineF mOriginalLine;
        qreal mOriginalWidth;
        bool mIsNominalLine;

        QColor mColorOnDarkBackground;
        QColor mColorOnLightBackground;

        UBGraphicsStroke* mStroke;

};

#endif // UBGRAPHICSPOLYGONITEM_H
