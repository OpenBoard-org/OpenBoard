/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBTOOLWIDGET_H_
#define UBTOOLWIDGET_H_

#include <QtGui>
#include <QtWebKit>
#include "core/UB.h"

class UBGraphicsScene;
class UBGraphicsWidgetItem;

class UBToolWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        UBToolWidget(const QUrl& pUrl, QGraphicsItem *pParent = 0);
        UBToolWidget(UBGraphicsWidgetItem* pGraphicsWidgetItem, QGraphicsItem *pParent = 0);
        virtual ~UBToolWidget();

        UBGraphicsWidgetItem* graphicsWidgetItem() const;
        QPointF naturalCenter() const;

        void centerOn(const QPointF& pos);
        void remove();

        virtual UBGraphicsScene* scene();
        virtual QPointF pos() const; 
        virtual void setPos(const QPointF &point);
        virtual void setPos(qreal x, qreal y);
        virtual int type() const;
        
        enum 
        { 
            Type = UBGraphicsItemType::ToolWidgetItemType 
        };

    protected:
        void initialize();

        virtual bool event(QEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);      

    private slots:
        void javaScriptWindowObjectCleared();

    protected:
        bool mShouldMoveWidget;
        int mContentMargin;
        int mFrameWidth;
        QGraphicsWebView *mGraphicsWebView;        
        UBGraphicsWidgetItem *mGraphicsWidgetItem;
        QPointF mMousePressPos;

        static QPixmap *sClosePixmap;
        static QPixmap *sUnpinPixmap;
};

#endif /* UBTOOLWIDGET_H_ */
