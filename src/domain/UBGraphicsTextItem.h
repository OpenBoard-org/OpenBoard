/*
 * UBGraphicsTextItem.h
 *
 *  Created on: 30 mars 2009
 *      Author: Luc
 */

#ifndef UBGRAPHICSTEXTITEM_H_
#define UBGRAPHICSTEXTITEM_H_

#include <QtGui>
#include "UBItem.h"
#include "core/UB.h"
#include "UBResizableGraphicsItem.h"

class UBGraphicsItemDelegate;
class UBGraphicsScene;

class UBGraphicsTextItem : public QGraphicsTextItem, public UBItem, public UBResizableGraphicsItem, public UBGraphicsItem
{
    Q_OBJECT;

    public:
        UBGraphicsTextItem(QGraphicsItem * parent = 0);
        virtual ~UBGraphicsTextItem();

        enum { Type = UBGraphicsItemType::TextItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;
        virtual UBGraphicsScene* scene();

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;

        void setTextWidth(qreal width);
        void setTextHeight(qreal height);
        qreal textHeight() const;

        void contentsChanged();

        virtual void resize(qreal w, qreal h);

        virtual QSizeF size() const;

        virtual void remove();

        static QColor lastUsedTextColor;

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

    signals:
        void textUndoCommandAdded(UBGraphicsTextItem *textItem);

    private slots:
        void undoCommandAdded();
        void documentSizeChanged(const QSizeF & newSize);

    private:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    private:
        UBGraphicsItemDelegate *mDelegate;
        qreal mTextHeight;

        int mMultiClickState;
        QTime mLastMousePressTime;
        QString mTypeTextHereLabel;

        QColor mColorOnDarkBackground;
        QColor mColorOnLightBackground;


};

#endif /* UBGRAPHICSTEXTITEM_H_ */
