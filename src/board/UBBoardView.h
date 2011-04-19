/*
 * UBBoardView.h
 *
 *  Created on: Sep 1, 2008
 *      Author: luc
 */

#ifndef UBBOARDVIEW_H_
#define UBBOARDVIEW_H_

#include <QtGui>

#include "core/UB.h"

class UBBoardController;
class UBAppleWidget;
class UBGraphicsScene;

class UBBoardView : public QGraphicsView
{
    Q_OBJECT;

    public:

        UBBoardView(UBBoardController* pController, QWidget* pParent = 0);
        UBBoardView(UBBoardController* pController, int pStartLayer, int pEndLayer, QWidget* pParent = 0);
        virtual ~UBBoardView();

        UBGraphicsScene* scene();

        void forcedTabletRelease();

        void setToolCursor(int tool);

    signals:

        void resized(QResizeEvent* event);
        void hidden();
        void shown();

    protected:

        virtual bool event (QEvent * e);

        virtual void keyPressEvent(QKeyEvent *event);
        virtual void tabletEvent(QTabletEvent * event);
        virtual void mouseDoubleClickEvent(QMouseEvent *event);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);
        virtual void wheelEvent(QWheelEvent *event);
        virtual void leaveEvent ( QEvent * event);

        virtual void focusOutEvent ( QFocusEvent * event );

        virtual void drawItems(QPainter *painter, int numItems,
                                    QGraphicsItem *items[],
                                    const QStyleOptionGraphicsItem options[]);

        virtual void dragEnterEvent(QDragEnterEvent * event);
        virtual void dropEvent(QDropEvent *event);
        virtual void dragMoveEvent(QDragMoveEvent *event);

        virtual void resizeEvent(QResizeEvent * event);

        virtual void drawBackground(QPainter *painter, const QRectF &rect);

        virtual void showEvent(QShowEvent * event);
        virtual void hideEvent(QHideEvent * event);

    private:

        void init();

        inline bool shouldDisplayItem(QGraphicsItem *item)
        {
            bool ok;
            int itemLayerType = item->data(UBGraphicsItemData::ItemLayerType).toInt(&ok);
            return (ok && (itemLayerType >= mStartLayer && itemLayerType <= mEndLayer));
        }


        UBBoardController* mController;

        int mStartLayer, mEndLayer;
        bool mFilterZIndex;

        bool mTabletStylusIsPressed;
        bool mUsingTabletEraser;

        bool mPendingStylusReleaseEvent;

        bool mMouseButtonIsPressed;
        QPointF mPreviousPoint;
        QPoint mMouseDownPos;

        bool mPenPressureSensitive;
        bool mMarkerPressureSensitive;
        bool mUseHighResTabletEvent;

        QRubberBand *mRubberBand;
        bool mIsCreatingTextZone;
        bool mIsCreatingSceneGrabZone;

        bool isAbsurdPoint(QPoint point);

		bool mVirtualKeyboardActive;

    private slots:

        void settingChanged(QVariant newValue);

	public slots:

		void virtualKeyboardActivated(bool b);

};

#endif /* UBBOARDVIEW_H_ */
