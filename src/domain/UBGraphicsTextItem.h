/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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
    Q_OBJECT

    public:
        UBGraphicsTextItem(QGraphicsItem * parent = 0);
        virtual ~UBGraphicsTextItem();

        enum { Type = UBGraphicsItemType::TextItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

        virtual void copyItemParameters(UBItem *copy) const;

        virtual UBGraphicsScene* scene();

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;

        void setTextWidth(qreal width);
        void setTextHeight(qreal height);
        qreal textHeight() const;

        void contentsChanged();

        virtual void resize(qreal w, qreal h);

        virtual QSizeF size() const;

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

        virtual void clearSource(){;}
        virtual void setUuid(const QUuid &pUuid);

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
        qreal mTextHeight;

        int mMultiClickState;
        QTime mLastMousePressTime;
        QString mTypeTextHereLabel;

        QColor mColorOnDarkBackground;
        QColor mColorOnLightBackground;
};

#endif /* UBGRAPHICSTEXTITEM_H_ */
