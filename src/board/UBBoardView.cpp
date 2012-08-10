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
#include "UBBoardView.h"

#include <QtGui>
#include <QtXml>
#include <QListView>

#include "UBDrawingController.h"

#include "frameworks/UBGeometryUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "core/UBSettings.h"
#include "core/UBMimeData.h"
#include "core/UBApplication.h"
#include "core/UBSetting.h"
#include "core/UBPersistenceManager.h"

#include "network/UBHttpGet.h"

#include "gui/UBStylusPalette.h"
#include "gui/UBRubberBand.h"
#include "gui/UBToolWidget.h"
#include "gui/UBResources.h"
#include "gui/UBMainWindow.h"
#include "gui/UBThumbnailWidget.h"
#include "gui/UBTeacherGuideWidgetsTools.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBItem.h"
#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsGroupContainerItem.h"
#include "domain/UBGraphicsStrokesGroup.h"

#include "document/UBDocumentProxy.h"

#include "tools/UBGraphicsCompass.h"
#include "tools/UBGraphicsCache.h"
#include "tools/UBGraphicsTriangle.h"
#include "tools/UBGraphicsProtractor.h"

#include "core/memcheck.h"

UBBoardView::UBBoardView (UBBoardController* pController, QWidget* pParent)
: QGraphicsView (pParent)
, mController (pController)
, mIsCreatingTextZone (false)
, mIsCreatingSceneGrabZone (false)
, mOkOnWidget(false)
, suspendedMousePressEvent(NULL)
, mLongPressInterval(1000)
, mIsDragInProgress(false)
{
  init ();

  mFilterZIndex = false;

  mLongPressTimer.setInterval(mLongPressInterval);
  mLongPressTimer.setSingleShot(true);
}

UBBoardView::UBBoardView (UBBoardController* pController, int pStartLayer, int pEndLayer, QWidget* pParent)
: QGraphicsView (pParent)
, mController (pController)
, suspendedMousePressEvent(NULL)
, mLongPressInterval(1000)
, mIsDragInProgress(false)
{
  init ();

  mStartLayer = pStartLayer;
  mEndLayer = pEndLayer;

  mFilterZIndex = true;  

  mLongPressTimer.setInterval(mLongPressInterval);
  mLongPressTimer.setSingleShot(true);
}

UBBoardView::~UBBoardView () {
  //NOOP
    if (suspendedMousePressEvent)
        delete suspendedMousePressEvent;
}

void
UBBoardView::init ()
{
  connect (UBSettings::settings ()->boardPenPressureSensitive, SIGNAL (changed (QVariant)),
           this, SLOT (settingChanged (QVariant)));

  connect (UBSettings::settings ()->boardMarkerPressureSensitive, SIGNAL (changed (QVariant)),
           this, SLOT (settingChanged (QVariant)));

  connect (UBSettings::settings ()->boardUseHighResTabletEvent, SIGNAL (changed (QVariant)),
           this, SLOT (settingChanged (QVariant)));

  setWindowFlags (Qt::FramelessWindowHint);
  setFrameStyle (QFrame::NoFrame);
  setRenderHints (QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
  setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
  setAcceptDrops (true);

  setOptimizationFlag (QGraphicsView::IndirectPainting); // enable UBBoardView::drawItems filter

  mTabletStylusIsPressed = false;
  mMouseButtonIsPressed = false;
  mPendingStylusReleaseEvent = false;

  setCacheMode (QGraphicsView::CacheBackground);

  mUsingTabletEraser = false;
  mIsCreatingTextZone = false;
  mRubberBand = 0;
  mUBRubberBand = 0;

  mVirtualKeyboardActive = false;

  settingChanged (QVariant ());

  unsetCursor();

  movingItem = NULL;
  mWidgetMoved = false;
}

UBGraphicsScene*
UBBoardView::scene ()
{
  return qobject_cast<UBGraphicsScene*> (QGraphicsView::scene ());
}

void
UBBoardView::hideEvent (QHideEvent * event)
{
  Q_UNUSED (event);
  emit hidden ();
}

void
UBBoardView::showEvent (QShowEvent * event)
{
  Q_UNUSED (event);
  emit shown ();
}

void
UBBoardView::keyPressEvent (QKeyEvent *event)
{
  // send to the scene anyway
  QApplication::sendEvent (scene (), event);

  if (!event->isAccepted ())
    {
      switch (event->key ())
        {
        case Qt::Key_Up:
        case Qt::Key_PageUp:
        case Qt::Key_Left:
          {
            mController->previousScene ();
            break;
          }

        case Qt::Key_Down:
        case Qt::Key_PageDown:
        case Qt::Key_Right:
        case Qt::Key_Space:
          {
            mController->nextScene ();
            break;
          }

        case Qt::Key_Home:
          {
            mController->firstScene ();
            break;
          }
        case Qt::Key_End:
          {
            mController->lastScene ();
            break;
          }
        case Qt::Key_Insert:
          {
            mController->addScene ();
            break;
          }
        }


      if (event->modifiers () & Qt::ControlModifier) // keep only ctrl/cmd keys
        {
          switch (event->key ())
            {
            case Qt::Key_Plus:
            case Qt::Key_I:
              {
                mController->zoomIn ();
                event->accept ();
                break;
              }
            case Qt::Key_Minus:
            case Qt::Key_O:
              {
                mController->zoomOut ();
                event->accept ();
                break;
              }
            case Qt::Key_0:
              {
                mController->zoomRestore ();
                event->accept ();
                break;
              }
            case Qt::Key_Left:
              {
                mController->handScroll (-100, 0);
                event->accept ();
                break;
              }
            case Qt::Key_Right:
              {
                mController->handScroll (100, 0);
                event->accept ();
                break;
              }
            case Qt::Key_Up:
              {
                mController->handScroll (0, -100);
                event->accept ();
                break;
              }
            case Qt::Key_Down:
              {
                mController->handScroll (0, 100);
                event->accept ();
                break;
              }
            default:
              {
                // NOOP
              }
            }
        }
    }
}

bool
UBBoardView::event (QEvent * e)
{
  if (e->type () == QEvent::Gesture)
    {
      QGestureEvent *gestureEvent = dynamic_cast<QGestureEvent *> (e);
      if (gestureEvent)
        {
          QSwipeGesture* swipe = dynamic_cast<QSwipeGesture*> (gestureEvent->gesture (Qt::SwipeGesture));

          if (swipe)
            {
              if (swipe->horizontalDirection () == QSwipeGesture::Left)
                {
                  mController->previousScene ();
                  gestureEvent->setAccepted (swipe, true);
                }

              if (swipe->horizontalDirection () == QSwipeGesture::Right)
                {
                  mController->nextScene ();
                  gestureEvent->setAccepted (swipe, true);
                }
            }
        }
    }

  return QGraphicsView::event (e);
}

void UBBoardView::tabletEvent (QTabletEvent * event)
{
    if (!mUseHighResTabletEvent) {
        event->setAccepted (false);
        return;
    }

    UBDrawingController *dc = UBDrawingController::drawingController ();

    QPointF tabletPos = event->pos();
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)dc->stylusTool ();

    if (event->type () == QEvent::TabletPress || event->type () == QEvent::TabletEnterProximity) {
        if (event->pointerType () == QTabletEvent::Eraser) {
            dc->setStylusTool (UBStylusTool::Eraser);
            mUsingTabletEraser = true;
        }
        else {
            if (mUsingTabletEraser && currentTool == UBStylusTool::Eraser)
                dc->setStylusTool (dc->latestDrawingTool ());

            mUsingTabletEraser = false;
        }
    }

    // if event are not Pen events, we drop the tablet stuff and route everything through mouse event
    if (currentTool != UBStylusTool::Pen && currentTool != UBStylusTool::Line && currentTool != UBStylusTool::Marker && !mMarkerPressureSensitive){
        event->setAccepted (false);
        return;
    }

    QPointF scenePos = viewportTransform ().inverted ().map (tabletPos);

    qreal pressure = 1.0;
    if (((currentTool == UBStylusTool::Pen || currentTool == UBStylusTool::Line)
         && mPenPressureSensitive)
            || (currentTool == UBStylusTool::Marker && mMarkerPressureSensitive))
        pressure = event->pressure ();


    bool acceptEvent = true;

    switch (event->type ()) {
    case QEvent::TabletPress: {
        mTabletStylusIsPressed = true;
        scene()->inputDevicePress (scenePos, pressure);

        break;
    }
    case QEvent::TabletMove: {
        if (mTabletStylusIsPressed)
            scene ()->inputDeviceMove (scenePos, pressure);

        acceptEvent = false; // rerouted to mouse move

        break;

    }
    case QEvent::TabletRelease: {
        UBStylusTool::Enum currentTool = (UBStylusTool::Enum)dc->stylusTool ();
        scene ()->setToolCursor (currentTool);
        setToolCursor (currentTool);

        scene ()->inputDeviceRelease ();

        mPendingStylusReleaseEvent = false;

        mTabletStylusIsPressed = false;
        mMouseButtonIsPressed = false;

        break;
    }
    default: {
        //NOOP - avoid compiler warning
    }
    }

    // ignore mouse press and mouse move tablet event so that it is rerouted to mouse events,
    // documented in QTabletEvent Class Reference:
    /* The event handler QWidget::tabletEvent() receives all three types of tablet events.
     Qt will first send a tabletEvent then, if it is not accepted, it will send a mouse event. */
    //
    // This is a workaround to the fact that tablet event are not delivered to child widget (like palettes)
    //

    event->setAccepted (acceptEvent);

}

bool UBBoardView::itemIsLocked(QGraphicsItem *item)
{
    if (!item)
        return false;

    if (item->data(UBGraphicsItemData::ItemLocked).toBool())
        return true;

    return itemIsLocked(item->parentItem());

}

bool UBBoardView::itemHaveParentWithType(QGraphicsItem *item, int type)
{
    if (!item)
        return false;

    if (type == item->type())
        return true;
    
    return itemHaveParentWithType(item->parentItem(), type);

}

bool UBBoardView::itemShouldReceiveMousePressEvent(QGraphicsItem *item)
{
    if (!item) 
        return true;

    if (item == scene()->backgroundObject())
        return false;

    if (itemIsLocked(item))
        return false;


    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController()->stylusTool();

    if ((currentTool == UBStylusTool::Play) && UBGraphicsGroupContainerItem::Type == movingItem->type())
    {
        movingItem = NULL;
        return false;
    }

    switch(item->type())
    {

    case UBGraphicsMediaItem::Type:
        return false;

    case UBGraphicsPixmapItem::Type:
    case UBGraphicsTextItem::Type:
        if ((currentTool == UBStylusTool::Selector) && item->isSelected())
            return true;
        if ((currentTool == UBStylusTool::Selector) && item->parentItem() && item->parentItem()->isSelected())
            return true;
        if (currentTool != UBStylusTool::Selector)
            return false;
        break;

    case UBGraphicsGroupContainerItem::Type:
        return (currentTool == UBStylusTool::Selector);

    case UBGraphicsW3CWidgetItem::Type:
        if (currentTool == UBStylusTool::Selector && item->parentItem() && item->parentItem()->isSelected()) 
            return true;
        if (currentTool == UBStylusTool::Selector && item->isSelected()) 
            return true;
        if (currentTool == UBStylusTool::Play)
            return true;
        break;

    default: 
        return true;
    }

    return false;
}

bool UBBoardView::itemShouldReceiveSuspendedMousePressEvent(QGraphicsItem *item)
{
    if (!item) 
        return false;

    if (item == scene()->backgroundObject())
        return false;

    if (itemIsLocked(item))
        return false;

    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController()->stylusTool();
   
    switch(item->type())
    {
    case UBGraphicsPixmapItem::Type:
    case UBGraphicsTextItem::Type:
    case UBGraphicsW3CWidgetItem::Type:
        if (currentTool == UBStylusTool::Selector && !item->isSelected() && item->parentItem())
            return true;
        if (currentTool == UBStylusTool::Selector && item->isSelected())
            return true;
        break;
    
    case UBGraphicsMediaItem::Type:
        return true;

    default:
        return false;
    }
    return false;
}

bool UBBoardView::itemShouldBeMoved(QGraphicsItem *item)
{
    if (!item) 
        return false;

    if (item == scene()->backgroundObject())
        return false;

    if (!(mMouseButtonIsPressed || mTabletStylusIsPressed))
        return false;
        
    if (movingItem->data(UBGraphicsItemData::ItemLocked).toBool())
        return false;

    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController()->stylusTool();  
   
    if (movingItem->parentItem() && !movingItem->isSelected() && movingItem->parentItem()->isSelected())
        return false;

    switch(item->type())
    {
    case UBGraphicsGroupContainerItem::Type:
        return true;

    case UBGraphicsW3CWidgetItem::Type:
        if(currentTool == UBStylusTool::Selector && item->isSelected()) 
            return false;
        if(currentTool == UBStylusTool::Play)
            return false;

    case UBGraphicsPixmapItem::Type:
        if (item->isSelected())
            return false;
    case UBGraphicsMediaItem::Type:
    case UBGraphicsStrokesGroup::Type: 
        return true;
    case UBGraphicsTextItem::Type:
        return !item->isSelected();
    }
    return false;
}

// determine item to interacts: item self or it's container.
QGraphicsItem* UBBoardView::determineItemToMove(QGraphicsItem *item)
{
    if(item)
    {
        UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController()->stylusTool();  
        if ((UBStylusTool::Play == currentTool) && (UBGraphicsW3CWidgetItem::Type == item->type()))
                return item;

        if(item->parentItem() && UBGraphicsGroupContainerItem::Type == item->parentItem()->type())
        {
            if (UBStylusTool::Play == currentTool && item->parentItem()->isSelected())
                return item->parentItem();
            
            if (item->parentItem()->isSelected())
                return item;
                
               
            if (item->isSelected())
                return NULL;

            return item->parentItem();
        }

        if(item->parentItem() && UBGraphicsStrokesGroup::Type == item->parentItem()->type())    
            return determineItemToMove(item->parentItem());    
    }

    return item;
}

void UBBoardView::handleItemMousePress(QMouseEvent *event)
{
    mLastPressedMousePos = mapToScene(event->pos());


    if (movingItem && QGraphicsSvgItem::Type !=  movingItem->type()
        && UBGraphicsDelegateFrame::Type !=  movingItem->type())
    {
        foreach(QGraphicsItem *item, scene()->selectedItems())
        {
            if (item != movingItem)
            {
                item->setSelected(false);
            }
        }
    }

    if (itemShouldReceiveMousePressEvent(movingItem))
        QGraphicsView::mousePressEvent (event);
    else
    {
        if (movingItem)
            movingItem->clearFocus();

        if (suspendedMousePressEvent)
        {
            delete suspendedMousePressEvent;
            suspendedMousePressEvent = NULL;
        }
    
        if (itemShouldReceiveSuspendedMousePressEvent(movingItem))
        {
            suspendedMousePressEvent = new QMouseEvent(event->type(), event->pos(), event->button(), event->buttons(), event->modifiers());
        }
    }
}

void UBBoardView::handleItemMouseMove(QMouseEvent *event)
{
    if (movingItem)
        movingItem = determineItemToMove(movingItem);

    if (movingItem && itemShouldBeMoved(movingItem) && (mMouseButtonIsPressed || mTabletStylusIsPressed))       
    {
        QPointF scenePos = mapToScene(event->pos());
        QPointF newPos = movingItem->pos() + scenePos - mLastPressedMousePos;
        movingItem->setPos(newPos);
        mLastPressedMousePos = scenePos;
        mWidgetMoved = true;
        event->accept();
    }
    else 
    {
        QPointF posBeforeMove;
        QPointF posAfterMove;

        if (movingItem)
            posBeforeMove = movingItem->pos();

        QGraphicsView::mouseMoveEvent (event);

        if (movingItem)
          posAfterMove = movingItem->pos();

        mWidgetMoved = ((posAfterMove-posBeforeMove).manhattanLength() != 0);

        // a cludge for terminate moving of w3c widgets.
        if (mWidgetMoved && UBGraphicsW3CWidgetItem::Type == movingItem->type())
            movingItem->setPos(posBeforeMove);

    }
}

void UBBoardView::rubberItems()
{
    if (mUBRubberBand)
        mRubberedItems = items(mUBRubberBand->geometry());

    foreach(QGraphicsItem *item, mRubberedItems)
    {
        if (item->parentItem() && UBGraphicsGroupContainerItem::Type == item->parentItem()->type())
            mRubberedItems.removeOne(item);
    }
}

void UBBoardView::moveRubberedItems(QPointF movingVector)
{
    QRectF invalidateRect = scene()->itemsBoundingRect();

    foreach (QGraphicsItem *item, mRubberedItems) 
    {

        if (item->type() == UBGraphicsW3CWidgetItem::Type
              || item->type() == UBGraphicsPixmapItem::Type
              || item->type() == UBGraphicsMediaItem::Type
              || item->type() == UBGraphicsSvgItem::Type
              || item->type() == UBGraphicsTextItem::Type
              || item->type() == UBGraphicsStrokesGroup::Type
              || item->type() == UBGraphicsGroupContainerItem::Type) 
        {
              item->setPos(item->pos()+movingVector);       
        }    
    }

    scene()->invalidate(invalidateRect);
}

void UBBoardView::longPressEvent()
{
   UBDrawingController *drawingController = UBDrawingController::drawingController();
   UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

   
   disconnect(&mLongPressTimer, SIGNAL(timeout()), this, SLOT(longPressEvent()));

   if (UBStylusTool::Selector == currentTool)
   {
        drawingController->setStylusTool(UBStylusTool::Play);
   }
   else
   if (currentTool == UBStylusTool::Play)
   {
        drawingController->setStylusTool(UBStylusTool::Selector);
   }
   else
   if (UBStylusTool::Eraser == currentTool)
   {
       UBApplication::boardController->paletteManager()->toggleErasePalette(true);
   }

}

void UBBoardView::mousePressEvent (QMouseEvent *event)
{
    mIsDragInProgress = false;

    if (isAbsurdPoint (event->pos ()))
    {
        event->accept ();
        return;
    }

    mMouseDownPos = event->pos ();

    movingItem = scene()->itemAt(this->mapToScene(event->posF().toPoint()));
    if (!movingItem)
        emit clickOnBoard();

    if (event->button () == Qt::LeftButton && isInteractive ())
    {

        UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

        if (!mTabletStylusIsPressed)
            mMouseButtonIsPressed = true;

        if (currentTool == UBStylusTool::ZoomIn)
        {
            mController->zoomIn (mapToScene (event->pos ()));
            event->accept ();
        }
        else if (currentTool == UBStylusTool::ZoomOut)
        {
            mController->zoomOut (mapToScene (event->pos ()));
            event->accept ();
        }
        else if (currentTool == UBStylusTool::Hand)
        {
            viewport ()->setCursor (QCursor (Qt::ClosedHandCursor));
            mPreviousPoint = event->posF ();
            event->accept ();
        }
        else if (currentTool == UBStylusTool::Selector || currentTool == UBStylusTool::Play)
        {
            connect(&mLongPressTimer, SIGNAL(timeout()), this, SLOT(longPressEvent()));
            if (!movingItem && !mController->cacheIsVisible())
                mLongPressTimer.start();
            
            if (!movingItem) {
                // Rubberband selection implementation
                if (!mUBRubberBand) {
                    mUBRubberBand = new UBRubberBand(QRubberBand::Rectangle, this);
                }
                mUBRubberBand->setGeometry (QRect (mMouseDownPos, QSize ()));
                mUBRubberBand->show();
            }
            else
            {
                if(mUBRubberBand)
                    mUBRubberBand->hide();
            }

            handleItemMousePress(event);
            event->accept();
        }
        else if (currentTool == UBStylusTool::Text)
        {
            int frameWidth = UBSettings::settings ()->objectFrameWidth;
            QRectF fuzzyRect (0, 0, frameWidth * 4, frameWidth * 4);
            fuzzyRect.moveCenter (mapToScene (mMouseDownPos));

            UBGraphicsTextItem* foundTextItem = 0;
            QListIterator<QGraphicsItem *> it (scene ()->items (fuzzyRect));

            while (it.hasNext () && !foundTextItem)
            {
                foundTextItem = qgraphicsitem_cast<UBGraphicsTextItem*>(it.next ());
            }

            if (foundTextItem)
            {
                mIsCreatingTextZone = false;
                QGraphicsView::mousePressEvent (event);
            }
            else
            {
                scene ()->deselectAllItems ();

                if (!mRubberBand)
                    mRubberBand = new UBRubberBand (QRubberBand::Rectangle, this);
                mRubberBand->setGeometry (QRect (mMouseDownPos, QSize ()));
                mRubberBand->show ();
                mIsCreatingTextZone = true;

                event->accept ();
            }
        }
        else if (currentTool == UBStylusTool::Capture)
        {
            scene ()->deselectAllItems ();

            if (!mRubberBand)
                mRubberBand = new UBRubberBand (QRubberBand::Rectangle, this);

            mRubberBand->setGeometry (QRect (mMouseDownPos, QSize ()));
            mRubberBand->show ();
            mIsCreatingSceneGrabZone = true;

            event->accept ();
        }
        else
        {
            if(UBDrawingController::drawingController()->mActiveRuler==NULL)
            {
                viewport()->setCursor (QCursor (Qt::BlankCursor));
            }

            if (scene () && !mTabletStylusIsPressed)
            {
                if (currentTool == UBStylusTool::Eraser)
                {
                    connect(&mLongPressTimer, SIGNAL(timeout()), this, SLOT(longPressEvent()));
                    mLongPressTimer.start();
                }
                scene ()->inputDevicePress (mapToScene (UBGeometryUtils::pointConstrainedInRect (event->pos (), rect ())));
            }
            event->accept ();
        }
    }
}

void
UBBoardView::mouseMoveEvent (QMouseEvent *event)
{
  if(!mIsDragInProgress && ((mapToScene(event->pos()) - mLastPressedMousePos).manhattanLength() < QApplication::startDragDistance())) 
  {
      return;
  }

  mIsDragInProgress = true;
  UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

  mLongPressTimer.stop();

  if (isAbsurdPoint (event->pos ()))
    {
      event->accept ();
      return;
    }

  if (currentTool == UBStylusTool::Hand && (mMouseButtonIsPressed || mTabletStylusIsPressed))
    {
      QPointF eventPosition = event->posF ();
      qreal dx = eventPosition.x () - mPreviousPoint.x ();
      qreal dy = eventPosition.y () - mPreviousPoint.y ();
      mController->handScroll (dx, dy);
      mPreviousPoint = eventPosition;
      event->accept ();
    }
  else if (currentTool == UBStylusTool::Selector || currentTool == UBStylusTool::Play)
  {
      if((event->pos() - mLastPressedMousePos).manhattanLength() < QApplication::startDragDistance()) {
          return;
      }

      if (!movingItem && (mMouseButtonIsPressed || mTabletStylusIsPressed) && mUBRubberBand && mUBRubberBand->isVisible()) {

          QRect bandRect(mMouseDownPos, event->pos());

          bandRect = bandRect.normalized();

          mUBRubberBand->setGeometry(bandRect);

          QList<QGraphicsItem *> rubberItems = items(bandRect);
          foreach (QGraphicsItem *item, mJustSelectedItems) {
              if (!rubberItems.contains(item)) {
                  item->setSelected(false);
                  mJustSelectedItems.remove(item);
              }
          }

          if (currentTool == UBStylusTool::Selector)
          foreach (QGraphicsItem *item, items(bandRect)) {

              if (item->type() == UBGraphicsW3CWidgetItem::Type
                      || item->type() == UBGraphicsPixmapItem::Type
                      || item->type() == UBGraphicsMediaItem::Type
                      || item->type() == UBGraphicsSvgItem::Type
                      || item->type() == UBGraphicsTextItem::Type
                      || item->type() == UBGraphicsStrokesGroup::Type
                      || item->type() == UBGraphicsGroupContainerItem::Type) {

                  if (!mJustSelectedItems.contains(item)) {
                      item->setSelected(true);
                      mJustSelectedItems.insert(item);
                  }
              }
          }
      }

      handleItemMouseMove(event);
    }
  else if ((UBDrawingController::drawingController()->isDrawingTool())
    && !mMouseButtonIsPressed)
  {
      QGraphicsView::mouseMoveEvent (event);
  }
  else if (currentTool == UBStylusTool::Text || currentTool == UBStylusTool::Capture)
    {
      if (mRubberBand && (mIsCreatingTextZone || mIsCreatingSceneGrabZone))
        {
          mRubberBand->setGeometry (QRect (mMouseDownPos, event->pos ()).normalized ());
          event->accept ();
        }
      else
        {
          QGraphicsView::mouseMoveEvent (event);
        }
    }
  else
    {
      if (!mTabletStylusIsPressed && scene ())
      {           
          scene ()->inputDeviceMove (mapToScene (UBGeometryUtils::pointConstrainedInRect (event->pos (), rect ())), mMouseButtonIsPressed);
      }
      event->accept ();
    }

    if((event->pos() - mLastPressedMousePos).manhattanLength() < QApplication::startDragDistance()) 
        mWidgetMoved = true;
}

void
UBBoardView::mouseReleaseEvent (QMouseEvent *event)
{
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

  setToolCursor (currentTool);
  // first propagate device release to the scene
  if (scene ())
    scene ()->inputDeviceRelease ();

  if (currentTool == UBStylusTool::Selector)
  {
      if (mWidgetMoved)
      {
          mWidgetMoved = false;
          movingItem = NULL;
      }
      else 
      if (movingItem)
      {         
          if (suspendedMousePressEvent && !movingItem->data(UBGraphicsItemData::ItemLocked).toBool())       
          {
              QGraphicsView::mousePressEvent(suspendedMousePressEvent);     // suspendedMousePressEvent is deleted by old Qt event loop
              movingItem = NULL;
              delete suspendedMousePressEvent;
              suspendedMousePressEvent = NULL;                       
          }
          else
          {
             if (   QGraphicsSvgItem::Type !=  movingItem->type()
                 && UBGraphicsDelegateFrame::Type !=  movingItem->type())
                 movingItem->setSelected(true);
          }
      }

      if (mUBRubberBand && mUBRubberBand->isVisible()) {
          mUBRubberBand->hide();
      }

      QGraphicsView::mouseReleaseEvent (event);
  }
  else if (currentTool == UBStylusTool::Play)
  {
      if (mWidgetMoved)
      {
          movingItem = NULL;
          mWidgetMoved = false;
      }
      else
      {
          if (suspendedMousePressEvent && movingItem && !movingItem->data(UBGraphicsItemData::ItemLocked).toBool())       
          {
              QGraphicsView::mousePressEvent(suspendedMousePressEvent);     // suspendedMousePressEvent is deleted by old Qt event loop
              movingItem = NULL;
              delete suspendedMousePressEvent;
              suspendedMousePressEvent = NULL;
          }

          QGraphicsView::mouseReleaseEvent (event);
      }
  }
  else if (currentTool == UBStylusTool::Text)
    {
      if (mRubberBand)
        mRubberBand->hide ();

      if (scene () && mRubberBand && mIsCreatingTextZone)
        {
          QRect rubberRect = mRubberBand->geometry ();

          UBGraphicsTextItem* textItem = scene()->addTextHtml ("", mapToScene (rubberRect.topLeft ()));
          event->accept ();

          UBDrawingController::drawingController ()->setStylusTool (UBStylusTool::Selector);

          textItem->setSelected (true);
          textItem->setFocus();
        }
      else
        {
          QGraphicsView::mouseReleaseEvent (event);
        }

      mIsCreatingTextZone = false;
    }
  else if (currentTool == UBStylusTool::Capture)
    {
      if (mRubberBand)
        mRubberBand->hide ();

      if (scene () && mRubberBand && mIsCreatingSceneGrabZone && mRubberBand->geometry ().width () > 16)
        {
          QRect rect = mRubberBand->geometry ();
          QPointF sceneTopLeft = mapToScene (rect.topLeft ());
          QPointF sceneBottomRight = mapToScene (rect.bottomRight ());
          QRectF sceneRect (sceneTopLeft, sceneBottomRight);

          mController->grabScene (sceneRect);

          event->accept ();
        }
      else
        {
          QGraphicsView::mouseReleaseEvent (event);
        }

      mIsCreatingSceneGrabZone = false;
    }
  else
    {
      if (mPendingStylusReleaseEvent || mMouseButtonIsPressed)
        {
          event->accept ();
        }
    }

  mMouseButtonIsPressed = false;
  mPendingStylusReleaseEvent = false;
  mTabletStylusIsPressed = false;
  movingItem = NULL;

  mLongPressTimer.stop();
}

void
UBBoardView::forcedTabletRelease ()
{

  if (mMouseButtonIsPressed || mTabletStylusIsPressed || mPendingStylusReleaseEvent)
    {
      qWarning () << "dirty mouse/tablet state:";
      qWarning () << "mMouseButtonIsPressed =" << mMouseButtonIsPressed;
      qWarning () << "mTabletStylusIsPressed = " << mTabletStylusIsPressed;
      qWarning () << "mPendingStylusReleaseEvent" << mPendingStylusReleaseEvent;
      qWarning () << "forcing device release";

      scene ()->inputDeviceRelease ();

      mMouseButtonIsPressed = false;
      mTabletStylusIsPressed = false;
      mPendingStylusReleaseEvent = false;
    }
}

void
UBBoardView::mouseDoubleClickEvent (QMouseEvent *event)
{
  // We don't want a double click, we want two clicks
  mousePressEvent (event);
}

void
UBBoardView::wheelEvent (QWheelEvent *wheelEvent)
{
  if (isInteractive () && wheelEvent->orientation () == Qt::Vertical)
    {
      // Too many wheelEvent are sent, how should we handle them to "smoothly" zoom ?
      // something like zoom( pow(zoomFactor, event->delta() / 120) )

      // use DateTime man, store last event time, and if if less than 300ms than this is one big scroll
      // and move scroll with one const speed.
        // so, you no will related with scroll event count
    }

    QList<QGraphicsItem *> selItemsList = scene()->selectedItems();
    // if NO have selected items, than no need process mouse wheel. just exist
    if( selItemsList.count() > 0 )
    {
        // only one selected item possible, so we will work with first item only
        QGraphicsItem * selItem = selItemsList[0];

        // get items list under mouse cursor
        QPointF scenePos = mapToScene(wheelEvent->pos());
        QList<QGraphicsItem *> itemsList = scene()->items(scenePos);

        QBool isSlectedAndMouseHower = itemsList.contains(selItem);
        if(isSlectedAndMouseHower)
        {
            QGraphicsView::wheelEvent(wheelEvent);
            wheelEvent->accept();
        }

    }

}

void
UBBoardView::leaveEvent (QEvent * event)
{
  if (scene ())
    scene ()->leaveEvent (event);

  QGraphicsView::leaveEvent (event);
}

void
UBBoardView::drawItems (QPainter *painter, int numItems,
                        QGraphicsItem* items[],
                        const QStyleOptionGraphicsItem options[])
{
  if (!mFilterZIndex)
    {
      QGraphicsView::drawItems (painter, numItems, items, options);
    }
  else
    {
      int count = 0;

      QGraphicsItem** itemsFiltered = new QGraphicsItem*[numItems];
      QStyleOptionGraphicsItem *optionsFiltered = new QStyleOptionGraphicsItem[numItems];

      for (int i = 0; i < numItems; i++)
        {
          if (shouldDisplayItem (items[i]))
            {
              itemsFiltered[count] = items[i];
              optionsFiltered[count] = options[i];
              count++;
            }
        }

      QGraphicsView::drawItems (painter, count, itemsFiltered, optionsFiltered);

      delete[] optionsFiltered;
      delete[] itemsFiltered;
    }
}


void UBBoardView::dragMoveEvent (QDragMoveEvent *event)
{
    QGraphicsItem* graphicsItemAtPos = itemAt(event->pos().x(),event->pos().y());
    UBGraphicsWidgetItem* graphicsWidget = dynamic_cast<UBGraphicsWidgetItem*>(graphicsItemAtPos);

    if (graphicsWidget) {
        if (graphicsWidget->acceptDrops()) {
            if (!mOkOnWidget) {
                if (!graphicsWidget->isDropableData(event->mimeData())) {
                    mOkOnWidget = false;
                    event->ignore();
                    return;
                } else {
                    mOkOnWidget = true;
                }
            }
            QPoint newPoint(graphicsWidget->mapFromScene(mapToScene(event->pos())).toPoint());
            QDragMoveEvent newEvent(newPoint, event->dropAction(), event->mimeData(), event->mouseButtons(), event->keyboardModifiers());
            QApplication::sendEvent(graphicsWidget->widgetWebView(),&newEvent);
        } else {
            mOkOnWidget = false;
            event->ignore();
        }
    }  else {
        event->acceptProposedAction();
        mOkOnWidget = false;
    }
}

void UBBoardView::dropEvent (QDropEvent *event)
{
    mOkOnWidget = false;
    QGraphicsItem* graphicsItemAtPos = itemAt(event->pos().x(),event->pos().y());
    UBGraphicsWidgetItem* graphicsWidget = dynamic_cast<UBGraphicsWidgetItem*>(graphicsItemAtPos);

    if (graphicsWidget && graphicsWidget->acceptDrops()) {

        graphicsWidget->processDropEvent(event);
        event->acceptProposedAction();

    } else if (!event->source()
               || dynamic_cast<UBThumbnailWidget *>(event->source())
               || dynamic_cast<QWebView*>(event->source())
               || dynamic_cast<UBTGMediaWidget*>(event->source())
               || dynamic_cast<QListView *>(event->source())
               || dynamic_cast<UBTGDraggableTreeItem*>(event->source())) {

        mController->processMimeData (event->mimeData (), mapToScene (event->pos ()));
        event->acceptProposedAction();
    }
}

void
UBBoardView::resizeEvent (QResizeEvent * event)
{
  const qreal maxWidth = width () * 10;
  const qreal maxHeight = height () * 10;

  setSceneRect (-(maxWidth / 2), -(maxHeight / 2), maxWidth, maxHeight);
  centerOn (0, 0);

  emit resized (event);
}

void
UBBoardView::drawBackground (QPainter *painter, const QRectF &rect)
{
  if (testAttribute (Qt::WA_TranslucentBackground))
    {
      QGraphicsView::drawBackground (painter, rect);
      return;
    }

  bool darkBackground = scene () && scene ()->isDarkBackground ();

  if (darkBackground)
    {
      painter->fillRect (rect, QBrush (QColor (Qt::black)));
    }
  else
    {
      painter->fillRect (rect, QBrush (QColor (Qt::white)));
    }

  if (transform ().m11 () > 0.5)
    {
      QColor bgCrossColor;

      if (darkBackground)
        bgCrossColor = UBSettings::crossDarkBackground;
      else
        bgCrossColor = UBSettings::crossLightBackground;

      if (transform ().m11 () < 1.0)
        {
          int alpha = 255 * transform ().m11 () / 2;
          bgCrossColor.setAlpha (alpha); // fade the crossing on small zooms
        }

      painter->setPen (bgCrossColor);

      if (scene () && scene ()->isCrossedBackground ())
        {
          qreal firstY = ((int) (rect.y () / UBSettings::crossSize)) * UBSettings::crossSize;

          for (qreal yPos = firstY; yPos < rect.y () + rect.height (); yPos += UBSettings::crossSize)
            {
              painter->drawLine (rect.x (), yPos, rect.x () + rect.width (), yPos);
            }

          qreal firstX = ((int) (rect.x () / UBSettings::crossSize)) * UBSettings::crossSize;

          for (qreal xPos = firstX; xPos < rect.x () + rect.width (); xPos += UBSettings::crossSize)
            {
              painter->drawLine (xPos, rect.y (), xPos, rect.y () + rect.height ());
            }
        }
    }

  if (!mFilterZIndex && scene ())
    {
      QSize pageNominalSize = scene ()->nominalSize ();

      if (pageNominalSize.isValid ())
        {
          qreal penWidth = 8.0 / transform ().m11 ();

          QRectF pageRect (pageNominalSize.width () / -2, pageNominalSize.height () / -2
                           , pageNominalSize.width (), pageNominalSize.height ());

          pageRect.adjust (-penWidth / 2, -penWidth / 2, penWidth / 2, penWidth / 2);

          QColor docSizeColor;

          if (darkBackground)
            docSizeColor = UBSettings::documentSizeMarkColorDarkBackground;
          else
            docSizeColor = UBSettings::documentSizeMarkColorLightBackground;

          QPen pen (docSizeColor);
          pen.setWidth (penWidth);
          painter->setPen (pen);
          painter->drawRect (pageRect);
        }
    }
}

void
UBBoardView::settingChanged (QVariant newValue)
{
  Q_UNUSED (newValue);

  mPenPressureSensitive = UBSettings::settings ()->boardPenPressureSensitive->get ().toBool ();
  mMarkerPressureSensitive = UBSettings::settings ()->boardMarkerPressureSensitive->get ().toBool ();
  mUseHighResTabletEvent = UBSettings::settings ()->boardUseHighResTabletEvent->get ().toBool ();
}

void UBBoardView::virtualKeyboardActivated(bool b)
{
    UBPlatformUtils::setWindowNonActivableFlag(this, b);
    mVirtualKeyboardActive = b;
    setInteractive(!b);
}


// Apple remote desktop sends funny events when the transmission is bad

bool UBBoardView::isAbsurdPoint(QPoint point)
{
    QDesktopWidget *desktop = qApp->desktop ();
    bool isValidPoint = false;

    for (int i = 0; i < desktop->numScreens (); i++)
    {
      QRect screenRect = desktop->screenGeometry (i);
      isValidPoint = isValidPoint || screenRect.contains (point);
    }

    return !isValidPoint;
}

void
UBBoardView::focusOutEvent (QFocusEvent * event)
{
  Q_UNUSED (event);
}

void
UBBoardView::setToolCursor (int tool)
{
  QWidget *controlViewport = viewport ();
  switch (tool)
    {
    case UBStylusTool::Pen:
      controlViewport->setCursor (UBResources::resources ()->penCursor);
      break;
    case UBStylusTool::Eraser:
      controlViewport->setCursor (UBResources::resources ()->eraserCursor);
      scene()->hideEraser();
      break;
    case UBStylusTool::Marker:
      controlViewport->setCursor (UBResources::resources ()->markerCursor);
      break;
    case UBStylusTool::Pointer:
      controlViewport->setCursor (UBResources::resources ()->pointerCursor);
      break;
    case UBStylusTool::Hand:
      controlViewport->setCursor (UBResources::resources ()->handCursor);
      break;
    case UBStylusTool::ZoomIn:
      controlViewport->setCursor (UBResources::resources ()->zoomInCursor);
      break;
    case UBStylusTool::ZoomOut:
      controlViewport->setCursor (UBResources::resources ()->zoomOutCursor);
      break;
    case UBStylusTool::Selector:
      controlViewport->setCursor (UBResources::resources ()->arrowCursor);
      break;
    case UBStylusTool::Play:
      controlViewport->setCursor (UBResources::resources ()->playCursor);
      break;
    case UBStylusTool::Line:
      controlViewport->setCursor (UBResources::resources ()->penCursor);
      break;
    case UBStylusTool::Text:
      controlViewport->setCursor (UBResources::resources ()->textCursor);
      break;
    case UBStylusTool::Capture:
      controlViewport->setCursor (UBResources::resources ()->penCursor);
      break;
    default:
      Q_ASSERT (false);
      //failsafe
      controlViewport->setCursor (UBResources::resources ()->penCursor);
    }
}

