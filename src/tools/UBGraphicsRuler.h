
/*
 * UBGraphicsRuler.h
 *
 *  Created on: April 16, 2009
 *      Author: Jerome Marchaud
 */

#ifndef UBGRAPHICSRULER_H_
#define UBGRAPHICSRULER_H_

#include <QtGui>
#include <QtSvg>

#include "core/UB.h"
#include "domain/UBItem.h"
#include "tools/UBAbstractDrawRuler.h"

class UBGraphicsScene;

class UBGraphicsRuler : public UBAbstractDrawRuler, public QGraphicsRectItem, public UBItem
{
    Q_OBJECT;

    public:
        UBGraphicsRuler();
        virtual ~UBGraphicsRuler();

        enum { Type = UBGraphicsItemType::RulerItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

		virtual void StartLine(const QPointF& position, qreal width);
		virtual void DrawLine(const QPointF& position, qreal width);
		virtual void EndLine();

    signals:

        void hidden();

    protected:
		
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        // Events
        virtual void	mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void	mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void	hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void	hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
		virtual void    hoverMoveEvent(QGraphicsSceneHoverEvent *event);

    private:
        // Helpers
        void    fillBackground(QPainter *painter);
        void    paintGraduations(QPainter *painter);
        void    paintRotationCenter(QPainter *painter);
        virtual void    rotateAroundTopLeftOrigin(qreal angle);

		QGraphicsSvgItem* mResizeSvgItem;

        virtual QPointF             topLeftOrigin() const;
        virtual QRectF           resizeButtonRect() const;
        virtual QRectF            closeButtonRect() const;
        virtual QRectF           rotateButtonRect() const;
        virtual UBGraphicsScene*            scene() const;

		int drawLineDirection;

        // Constants
        static const QRect               sDefaultRect;
};

#endif /* UBGRAPHICSRULER_H_ */
