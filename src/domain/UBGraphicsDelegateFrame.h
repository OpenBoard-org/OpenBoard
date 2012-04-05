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

#ifndef UBGRAPHICSDELEGATEFRAME_H_
#define UBGRAPHICSDELEGATEFRAME_H_

#include <QtGui>
#include "core/UB.h"
#include "domain/UBAngleWidget.h"

class QGraphicsSceneMouseEvent;
class UBGraphicsItemDelegate;
class QGraphicsSvgItem;


class UBGraphicsDelegateFrame: public QGraphicsRectItem, public QObject
{
    public:

        UBGraphicsDelegateFrame(UBGraphicsItemDelegate* pDelegate, QRectF pRect, qreal pFrameWidth, bool respectRatio = true);
        virtual ~UBGraphicsDelegateFrame();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

        QPainterPath shape() const;

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        void positionHandles();
        void setVisible(bool visible);

                virtual void setAntiScale(qreal pAntiScale);

        enum OperationMode {Scaling, Resizing};
        void setOperationMode(OperationMode pMode) {mOperationMode = pMode;}
        bool isResizing(){return mResizing;}

    private:
        QRectF bottomRightResizeGripRect() const;
        QRectF bottomResizeGripRect() const;
        QRectF leftResizeGripRect() const;
        QRectF rightResizeGripRect() const;
        QRectF topResizeGripRect() const;
        QRectF rotateButtonBounds() const;

        inline bool resizingBottomRight () const { return mCurrentTool == ResizeBottomRight; }
        inline bool resizingBottom ()  const { return mCurrentTool == ResizeBottom; }
        inline bool resizingRight () const { return mCurrentTool == ResizeRight; }
        inline bool resizingLeft () const { return mCurrentTool == ResizeLeft; }
        inline bool resizingTop () const { return mCurrentTool == ResizeTop; }
        inline bool rotating () const { return mCurrentTool == Rotate; }
        inline bool moving () const { return mCurrentTool == Move; }

        QTransform buildTransform ();
        void  updateResizeCursors ();
        void  initializeTransform ();

        enum FrameTool {None, Move, Rotate, ResizeBottomRight, ResizeTop, ResizeRight, ResizeBottom, ResizeLeft};
        FrameTool toolFromPos (QPointF pos);
        void refreshGeometry();

        FrameTool mCurrentTool;
        UBGraphicsItemDelegate* mDelegate;

        bool mVisible;
        qreal mFrameWidth;
        qreal mNominalFrameWidth;
        bool mRespectRatio;

        qreal mAngle;
        qreal mAngleOffset;
        qreal mTotalScaleX;
        qreal mTotalScaleY;
        qreal mScaleX;
        qreal mScaleY;
        qreal mTranslateX;
        qreal mTranslateY;
        qreal mTotalTranslateX;
        qreal mTotalTranslateY;
        qreal mAngleTolerance;
        QRect mAngleRect;

        QPointF mStartingPoint;
        QTransform mInitialTransform;

        QGraphicsSvgItem* mBottomRightResizeGripSvgItem;
        QGraphicsSvgItem* mBottomResizeGripSvgItem;
        QGraphicsSvgItem* mLeftResizeGripSvgItem;
        QGraphicsSvgItem* mRightResizeGripSvgItem;
        QGraphicsSvgItem* mTopResizeGripSvgItem;
        QGraphicsSvgItem* mRotateButton;

        QGraphicsRectItem* mBottomRightResizeGrip;
        QGraphicsRectItem* mBottomResizeGrip;
        QGraphicsRectItem* mLeftResizeGrip;
        QGraphicsRectItem* mRightResizeGrip;
        QGraphicsRectItem* mTopResizeGrip;

        OperationMode mOperationMode;

        QGraphicsItem* delegated();
        bool mMirrorX;
        bool mMirrorY;
        bool mResizing;
        bool mMirroredXAtStart;
        bool mMirroredYAtStart;

        UBAngleWidget *angleWidget;

};
#endif /* UBGRAPHICSDELEGATEFRAME_H_ */
