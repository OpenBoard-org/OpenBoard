/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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



#ifndef UBBOARDVIEW_H_
#define UBBOARDVIEW_H_

#define CONTROLVIEW_OBJ_NAME "ControlView"

#include <QtGui>
#include <QElapsedTimer>
#include <QGraphicsView>
#include <QQueue>
#include <QRubberBand>

#include "core/UB.h"
#include "domain/UBGraphicsDelegateFrame.h"

class UBBoardController;
class UBGraphicsScene;
class UBGraphicsWidgetItem;
class UBRubberBand;
class UBSnapIndicator;

class UBBoardView : public QGraphicsView
{
    Q_OBJECT

public:

    UBBoardView(UBBoardController* pController, QWidget* pParent = 0, bool isControl = false, bool isDesktop = false);
    UBBoardView(UBBoardController* pController, int pStartLayer, int pEndLayer, QWidget* pParent = 0, bool isControl = false, bool isDesktop = false);
    virtual ~UBBoardView();

    std::shared_ptr<UBGraphicsScene> scene();

    void forcedTabletRelease();

    void setToolCursor(int tool);

    void rubberItems();
    void moveRubberedItems(QPointF movingVector);

    void setMultiselection(bool enable);
    bool isMultipleSelectionEnabled() { return mMultipleSelectionIsEnabled; }

    void setBoxing(const QMargins& margins);
    void updateSnapIndicator(Qt::Corner corner, QPointF snapPoint, double angle = 0);

    // work around for handling tablet events on MAC OS with Qt 4.8.0 and above
#if defined(Q_OS_OSX)
    bool directTabletEvent(QEvent *event);
    QWidget *widgetForTabletEvent(QWidget *w, const QPoint &pos);
#endif
signals:
    void resized(QResizeEvent* event);
    void shown();
    void mouseReleased();
    void painted(const QRectF region);

protected:

    bool itemIsLocked(QGraphicsItem *item);
    bool isUBItem(QGraphicsItem *item); // we should to determine items who is not UB and use general scene behavior for them.
    bool isCppTool(QGraphicsItem *item);
    void handleItemsSelection(QGraphicsItem *item);
    bool itemShouldReceiveMousePressEvent(QGraphicsItem *item);
    bool itemShouldReceiveSuspendedMousePressEvent(QGraphicsItem *item);
    bool itemHaveParentWithType(QGraphicsItem *item, int type);
    bool itemShouldBeMoved(QGraphicsItem *item);
    QGraphicsItem* determineItemToPress(QGraphicsItem *item);
    QGraphicsItem* determineItemToMove(QGraphicsItem *item);
    void handleItemMousePress(QMouseEvent *event);
    void handleItemMouseMove(QMouseEvent *event);

    virtual bool event (QEvent * e);
    bool viewportEvent(QEvent* event) override;

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

    virtual void dropEvent(QDropEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);

    virtual void resizeEvent(QResizeEvent * event);

    virtual void paintEvent(QPaintEvent *event);

    virtual void drawBackground(QPainter *painter, const QRectF &rect);
    virtual void drawForeground(QPainter *painter, const QRectF &rect);

    virtual void scrollContentsBy(int dx, int dy);

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

    bool mVirtualKeyboardActive;
    bool mOkOnWidget;

    bool mWidgetMoved;
    QPointer<UBGraphicsItemDelegate> mMovingItemUndoDelegate;
    QPointF mFirstPressedMousePos;
    QPointF mLastPressedMousePos;
    QList<QPointF> mCornerPoints;

    /* when an item is moved around, the tracking must stop if the object is deleted */
    QGraphicsItem *_movingItem;

    QGraphicsItem *getMovingItem() {
        return _movingItem;
    }

    void setMovingItem(QGraphicsItem *item) {
        // if the current moving item is a qobject, it MUST be disconnected
        if (_movingItem) {
            QObject *moving_item_obj = dynamic_cast<QObject*>(_movingItem);
            if (moving_item_obj != nullptr)
                disconnect(moving_item_obj, &QObject::destroyed, this, &UBBoardView::movingItemDestroyed);
        }

        // attach the new moving item if relevant
        if (item) {
            QObject *item_obj = dynamic_cast<QObject*>(item);
            if (item_obj != nullptr)
                connect(item_obj, &QObject::destroyed, this, &UBBoardView::movingItemDestroyed);
        }
        _movingItem = item;
    }

    QMouseEvent *suspendedMousePressEvent;

    bool moveRubberBand;
    UBRubberBand *mUBRubberBand;

    QList<QGraphicsItem *> mRubberedItems;
    QSet<QGraphicsItem*> mJustSelectedItems;

    int mLongPressInterval;
    QTimer mLongPressTimer;

    bool mIsDragInProgress;
    bool mMultipleSelectionIsEnabled;
    bool bIsControl;
    bool bIsDesktop;
    bool mRubberBandInPlayMode;

    QMargins mMargins{};
    UBSnapIndicator* mSnapIndicator{nullptr};

    static bool hasSelectedParents(QGraphicsItem * item);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    enum class TouchSessionState
    {
        Idle,       // no touch sequence in progress
        Pending,    // first finger down, gesture window still open
        GestureCandidate, // two fingers down, waiting for actual pinch motion
        Active,     // committed to drawing / synthetic mouse
        Gesture,    // two-finger pan/zoom until all fingers lift
        Suppressed  // sequence alive but ignored (pen active, tool change, ...)
    };

    enum class TouchSuppressionMode
    {
        Commit,
        Cancel
    };

    struct TouchSession
    {
        TouchSessionState state = TouchSessionState::Idle;
        QHash<int, int> touchToPointer;
        QHash<int, QPointF> touchPositions;
        QHash<int, QPointF> touchPressPositions;
        QHash<int, QPointF> lastDeliveredPositions;

        int firstTouchId = -1;
        ulong firstTouchPressTimestamp = 0;
        qreal firstTouchTravelPx = 0;
        int toolAtStart = -1;
        bool sceneWasModified = false;  // the scene's modified flag when the session started
        bool committedChanges = false;  // this session pushed scene changes onto the undo stack

        bool deferredMousePending = false;
        bool syntheticMousePressed = false;
        quint64 syntheticMouseSequenceId = 0;
        QPointF deferredMousePos;
        QPointF deferredMouseLastPos;
        Qt::KeyboardModifiers deferredMouseModifiers = Qt::NoModifier;

        int gestureTouchIds[2] = {-1, -1};
        QPointF gestureInitialPos[2];
        QPointF gestureLastPos[2];
    };

    struct SyntheticMouseEvent
    {
        QEvent::Type type = QEvent::None;
        QPointF viewportPos;
        Qt::KeyboardModifiers modifiers = Qt::NoModifier;
        quint64 sequenceId = 0;
        bool cancel = false;
    };

    bool handleTouchEvent(QTouchEvent* event);
    void touchPointPressed(const QEventPoint& point, QTouchEvent* event);
    void touchPointMoved(const QEventPoint& point, QTouchEvent* event);
    void touchPointReleased(const QEventPoint& point, QTouchEvent* event);
    void promotePendingSession(bool deliverDeferredMove = true);
    bool addDirectTouchPointer(int touchId, const QPointF& viewportPos,
                               Qt::KeyboardModifiers modifiers);
    void deliverBufferedTouchMoves(Qt::KeyboardModifiers modifiers);
    void engageGesture(int firstTouchId, int secondTouchId);
    void updateGesture();

    // Session teardown comes in three layers:
    //  - suppressTouchSession() commits or cancels all scene / synthetic-mouse
    //    work, but keeps tracking fingers until they lift (state Suppressed);
    //  - finalizeTouchSession() also flushes the synthetic mouse queue, so all
    //    effects of the session are applied when it returns;
    //  - endTouchSession() resets the bookkeeping once no fingers remain.
    //
    // abandonTouchSession() is the bail-out for when the platform has taken the
    // touch stream away from us (hide, deactivate, focus loss, teardown): the
    // matching TouchEnd will never arrive, so the session must be torn all the
    // way down rather than parked in Suppressed, which would leave
    // canvasMouseLockedOut() true forever and swallow every later mouse event.
    void abandonTouchSession(TouchSuppressionMode mode = TouchSuppressionMode::Commit);
    void endTouchSession();
    void suppressTouchSession(TouchSuppressionMode mode);
    void finalizeTouchSession(TouchSuppressionMode mode = TouchSuppressionMode::Commit);
    void queueSyntheticMouse(QEvent::Type type, const QPointF& viewportPos,
                             Qt::KeyboardModifiers modifiers, bool cancel = false);
    void dispatchSyntheticMouseEvent(const SyntheticMouseEvent& event);
    void releaseMouseGrabberSilently(QGraphicsItem* mouseGrabber, QMouseEvent& releaseEvent);
    void flushSyntheticMouseEvents();
    bool cancelSyntheticMouse();
    void discardQueuedSyntheticMouse(quint64 sequenceId);
    QPointF touchScenePos(const QPointF& viewportPos);
    bool touchIsInkTool(int tool) const;
    bool touchDrawingEnabled() const;
    bool touchGesturesEnabled() const;
    bool suppressCanvasMouseForTouch(QMouseEvent* event);
    bool canvasMouseLockedOut() const;
    void cancelActiveCanvasMouseForTouch();
    bool drawingRecentlyOccurred() const;
    void recordDrawingActivity();
    int gestureWindowMs() const;
    int gestureClassificationWindowMs() const;
    int gestureQuietTimeMs() const;
    qreal gestureMoveThresholdPx() const;
    qreal gestureMaxSeparationPx() const;
    qreal drawingActivityThresholdPx() const;
    qreal pinchActivationThresholdPx() const;
    int mouseLockoutAfterTouchMs() const;

    TouchSession mTouchSession;
    QTimer mTouchPromotionTimer;
    // Catch-all for a touch sequence that stops being delivered without a
    // TouchEnd; restarted on every touch event, so it only fires after a live
    // session has gone completely silent.
    QTimer mTouchWatchdogTimer;
    bool mMultitouchEnabled{false};
    int mMultitouchMode{0};
    bool mSuppressSystemTouchMouseSequence{false};
    QElapsedTimer mLastDrawingActivity;
    QElapsedTimer mLastTouchInput;
    int mNextTouchPointerId{1};
    QQueue<SyntheticMouseEvent> mSyntheticMouseQueue;
    QTimer mSyntheticMouseDispatchTimer;
    bool mDispatchingSyntheticMouseEvent{false};
    quint64 mNextSyntheticMouseSequenceId{1};
    quint64 mDeliveredSyntheticMouseSequenceId{0};
#endif

private slots:
    void settingChanged(QVariant newValue);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void multitouchSettingChanged(QVariant newValue);
#endif
    void movingItemDestroyed(QObject* item = nullptr);

public slots:
    void virtualKeyboardActivated(bool b);
    void longPressEvent();

};

#endif /* UBBOARDVIEW_H_ */
