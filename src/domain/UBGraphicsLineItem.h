#ifndef UBGRAPHICSLINEITEM_H
#define UBGRAPHICSLINEITEM_H

#include <QtGui>
#include "core/UB.h"
#include "UBItem.h"
#include "domain/UBGraphicsStrokesGroup.h"

class UBItem;
class UBGraphicsScene;

class UBGraphicsLineItem : public QGraphicsLineItem, public UBItem, public UBGraphicsItem
{

    public:

        UBGraphicsLineItem(QGraphicsItem * parent = 0 );
        UBGraphicsLineItem(const QLineF& line, qreal pWidth);
        UBGraphicsLineItem(const QLineF& pLine, qreal pStartWidth, qreal pEndWidth);
        UBGraphicsLineItem(const QLineF & line, QGraphicsItem * parent = 0);

        ~UBGraphicsLineItem();

        void initialize();

        void setUuid(const QUuid &pUuid);

        void setColor(const QColor& color);
        void setStyle(const Qt::PenStyle& style);

        QColor color() const;
        Qt::PenStyle style() const;

        virtual UBGraphicsScene* scene();

        enum { Type = UBGraphicsItemType::LineItemType };

        virtual int type() const
        {
            return Type;
        }

        void setLine(const QLineF pLine)
        {
            mIsNominalLine = false;
            QGraphicsLineItem::setLine(pLine);
        }

        virtual UBItem* deepCopy() const;

        virtual void copyItemParameters(UBItem *copy) const;

        QLineF originalLine() { return mOriginalLine;}
        qreal originalWidth() { return mOriginalWidth;}
        bool isNominalLine() {return mIsNominalLine;}

        void setNominalLine(bool isNominalLine) { mIsNominalLine = isNominalLine; }

        QList<QPointF> linePoints();

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

        void SetDelegate();

        UBGraphicsStrokesGroup* StrokeGroup()
        {
            return mStrokeGroup;
        }

        void setStrokesGroup(UBGraphicsStrokesGroup *group);

    protected:
        void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    private:

        bool mHasAlpha;

        QLineF mOriginalLine;
        qreal mOriginalWidth;
        bool mIsNominalLine;

        QColor mColorOnDarkBackground;
        QColor mColorOnLightBackground;

        UBGraphicsStrokesGroup* mStrokeGroup;

};

#endif // UBGRAPHICSLINEITEM_H

