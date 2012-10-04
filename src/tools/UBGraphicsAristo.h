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

#ifndef UBGRAPHICSARISTO_H_
#define UBGRAPHICSARISTO_H_

#include "core/UB.h"
#include "domain/UBItem.h"
#include "domain/UBGraphicsScene.h"
#include "tools/UBAbstractDrawRuler.h"

#include <QtGlobal>
#include <QBrush>
#include <QCursor>
#include <QGraphicsPathItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QObject>
#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QStyleOptionGraphicsItem>
#include <QTransform>
#include <QWidget>

class UBGraphicsAristo : public UBAbstractDrawRuler, public QGraphicsPathItem, public UBItem
{
Q_OBJECT

public:
    UBGraphicsAristo();
    virtual ~UBGraphicsAristo();

    enum {
        Type = UBGraphicsItemType::AristoItemType 
    };

    enum Tool {
        None,
        Move,
        Resize,
        Rotate,
        Close,
        MoveMarker,
        HorizontalFlip
    };

    enum Orientation
    {
        Bottom = 0,
        Top,
        Undefined
    };
    
    void setOrientation(Orientation orientation);       

    virtual UBItem* deepCopy() const;
    virtual void copyItemParameters(UBItem *copy) const;

    virtual void StartLine(const QPointF& scenePos, qreal width);
    virtual void DrawLine(const QPointF& position, qreal width);
    virtual void EndLine();

    virtual int type() const
    {
        return Type;
    }
    UBGraphicsScene* scene() const;

protected:
    virtual void paint (QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);

    virtual void rotateAroundCenter(qreal angle);
    virtual void resize(qreal factor);

    virtual QPointF rotationCenter() const;

    virtual QRectF closeButtonRect() const;
    QRectF hFlipRect() const;
    QRectF markerButtonRect() const;
    QRectF resizeButtonRect () const;        
    QRectF rotateRect() const;

    QCursor flipCursor() const;        
    QCursor markerCursor() const;
    QCursor resizeCursor() const;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

private:
    Tool toolFromPos(QPointF pos);

    QTransform calculateRotationTransform();
    void rotateAroundCenter(QTransform& transform, QPointF center);

    void calculatePoints(QRectF bounds);
    QPainterPath determinePath();
    void setItemsPos();

    QBrush fillBrush() const;
    void paintGraduations(QPainter *painter);
    void paintMarker(QPainter *painter);
    void paintProtractorGraduations(QPainter* painter);
    void paintRulerGraduations(QPainter *painter);

    inline qreal radius () const
    {
        return sqrt(((B.x() - A.x())*(B.x() - A.x()))+((B.y() - A.y())*(B.y() - A.y()))) * 9 / 16 - 20;
    }        

    bool mMarking;
    bool mResizing;
    bool mRotating;

    Orientation mOrientation;

    qreal mRotatedAngle;
    qreal mMarkerAngle;
    qreal mStartAngle;

    qreal mSpan;

    QGraphicsSvgItem* mHFlipSvgItem;
    QGraphicsSvgItem* mMarkerSvgItem;
    QGraphicsSvgItem* mResizeSvgItem;
    QGraphicsSvgItem* mRotateSvgItem;      

    QPointF A, B, C;
    
    static const int sArcAngleMargin = 5;
    static const Orientation sDefaultOrientation;        
    static const QRectF sDefaultRect;
};

#endif /* UBGRAPHICSARISTO_H_ */
