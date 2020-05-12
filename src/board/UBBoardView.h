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



#ifndef UBBOARDVIEW_H_
#define UBBOARDVIEW_H_

#define CONTROLVIEW_OBJ_NAME "ControlView"

#include <QtGui>
#include <QGraphicsView>
#include <QRubberBand>

#include "core/UB.h"
#include "domain/UBGraphicsDelegateFrame.h"

class UBBoardController;
class UBGraphicsScene;
class UBGraphicsWidgetItem;
class UBRubberBand;

class UBBoardView : public QGraphicsView
{
    Q_OBJECT

public:

    UBBoardView(UBBoardController* pController, QWidget* pParent = 0, bool isControl = false, bool isDesktop = false);
    UBBoardView(UBBoardController* pController, int pStartLayer, int pEndLayer, QWidget* pParent = 0, bool isControl = false, bool isDesktop = false);
    virtual ~UBBoardView();

    UBGraphicsScene* scene();

    void forcedTabletRelease();

    void setToolCursor(int tool);

    void rubberItems();
    void moveRubberedItems(QPointF movingVector);

    void setMultiselection(bool enable);
    bool isMultipleSelectionEnabled() { return mMultipleSelectionIsEnabled; }
    // work around for handling tablet events on MAC OS with Qt 4.8.0 and above
#if defined(Q_OS_OSX)
    bool directTabletEvent(QEvent *event);
    QWidget *widgetForTabletEvent(QWidget *w, const QPoint &pos);
#endif
signals:
    void resized(QResizeEvent* event);
    void shown();
    void mouseReleased();

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

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
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

    virtual void drawBackground(QPainter *painter, const QRectF &rect);

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

    static bool hasSelectedParents(QGraphicsItem * item);

private slots:
    void settingChanged(QVariant newValue);
    void movingItemDestroyed(QObject* item = nullptr);

public slots:
    void virtualKeyboardActivated(bool b);
    void longPressEvent();

};

#endif /* UBBOARDVIEW_H_ */
