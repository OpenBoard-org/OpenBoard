/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#ifndef UBBOARDVIEW_H_
#define UBBOARDVIEW_H_

#include <QtGui>
#include "core/UB.h"
#include "domain/UBGraphicsDelegateFrame.h"

class UBBoardController;
class UBAppleWidget;
class UBGraphicsScene;
class UBGraphicsWidgetItem;
class UBRubberBand;

class UBBoardView : public QGraphicsView
{
    Q_OBJECT

    public:

        UBBoardView(UBBoardController* pController, QWidget* pParent = 0);
        UBBoardView(UBBoardController* pController, int pStartLayer, int pEndLayer, QWidget* pParent = 0);
        virtual ~UBBoardView();

        UBGraphicsScene* scene();

        void forcedTabletRelease();

        void setToolCursor(int tool);

        void rubberItems();
        void moveRubberedItems(QPointF movingVector);

    signals:

        void resized(QResizeEvent* event);
        void hidden();
        void shown();
        void clickOnBoard();

    protected:

        bool itemIsLocked(QGraphicsItem *item);
        bool itemShouldReceiveMousePressEvent(QGraphicsItem *item);
        bool itemShouldReceiveSuspendedMousePressEvent(QGraphicsItem *item);
        bool itemHaveParentWithType(QGraphicsItem *item, int type);
        bool itemShouldBeMoved(QGraphicsItem *item);
        QGraphicsItem* determineItemToMove(QGraphicsItem *item);
        void handleItemMousePress(QMouseEvent *event);
        void handleItemMouseMove(QMouseEvent *event);

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

//        virtual void dragEnterEvent(QDragEnterEvent * event);
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

        QList<QUrl> processMimeData(const QMimeData* pMimeData);

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
        bool mOkOnWidget;

        bool mWidgetMoved;
        QPointF mLastPressedMousePos;
        QGraphicsItem *movingItem;
        QMouseEvent *suspendedMousePressEvent;

        bool moveRubberBand;
        UBRubberBand *mUBRubberBand;
        
        QList<QGraphicsItem *> mRubberedItems;
        QSet<QGraphicsItem*> mJustSelectedItems;

        int mLongPressInterval;
        QTimer mLongPressTimer;

        bool mIsDragInProgress;

    private slots:

        void settingChanged(QVariant newValue);

	public slots:

		void virtualKeyboardActivated(bool b);
        void longPressEvent();

};

#endif /* UBBOARDVIEW_H_ */
