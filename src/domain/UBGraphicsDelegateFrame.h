/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBGRAPHICSDELEGATEFRAME_H_
#define UBGRAPHICSDELEGATEFRAME_H_

#include <QtGui>
#include "core/UB.h"

class QGraphicsSceneMouseEvent;
class UBGraphicsItemDelegate;
class QGraphicsSvgItem;


class UBGraphicsDelegateFrame: public QGraphicsRectItem, public QObject
{
    public:

        UBGraphicsDelegateFrame(UBGraphicsItemDelegate* pDelegate, QRectF pRect, qreal pFrameWidth, bool respectRatio = true, bool hasTitleBar = false);
        virtual ~UBGraphicsDelegateFrame();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        QPainterPath shape() const;

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        QPointF getFixedPointFromPos();
        QSizeF getResizeVector(qreal moveX, qreal moveY);
        QSizeF resizeDelegate(qreal moveX, qreal moveY);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        void positionHandles();
        void setVisible(bool visible);

        virtual void setAntiScale(qreal pAntiScale);

        enum OperationMode {Scaling, Resizing, ResizingHorizontally};
        void setOperationMode(OperationMode pMode) {mOperationMode = pMode;}
        bool isResizing(){return mResizing;}
        void moveLinkedItems(QLineF movingVector, bool bLinked = false);
        void prepareFramesToMove(QList<UBGraphicsDelegateFrame *> framesToMove);
        void prepareLinkedFrameToMove();
        QList<UBGraphicsDelegateFrame *> getLinkedFrames();

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
        void setCursorFromAngle(QString angle);
        bool canResizeBottomRight(qreal width, qreal height, qreal scaleFactor);

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
        QSizeF mOriginalSize;
        QPointF mFixedPoint;

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
        bool mFlippedX;
        bool mFlippedY;
        bool mMirrorX;
        bool mMirrorY;
        bool mResizing;
        bool mMirroredXAtStart;
        bool mMirroredYAtStart;
        qreal mTitleBarHeight;
        qreal mNominalTitleBarHeight;

        QList<UBGraphicsDelegateFrame *> mLinkedFrames;
};
#endif /* UBGRAPHICSDELEGATEFRAME_H_ */
