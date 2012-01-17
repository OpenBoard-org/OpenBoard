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

#include "UBGraphicsScene.h"

#include <QtGui>
#include <QtWebKit>
#include <QtSvg>
#include <QGraphicsView>

#include "frameworks/UBGeometryUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"
#include "core/UBPersistenceManager.h"

#include "gui/UBMagnifer.h"

#include "tools/UBGraphicsRuler.h"
#include "tools/UBGraphicsProtractor.h"
#include "tools/UBGraphicsCompass.h"
#include "tools/UBGraphicsTriangle.h"
#include "tools/UBGraphicsCurtainItem.h"
#include "tools/UBGraphicsCache.h"

#include "document/UBDocumentProxy.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"

#include "UBGraphicsItemUndoCommand.h"
#include "UBGraphicsTextItemUndoCommand.h"
#include "UBGraphicsProxyWidget.h"
#include "UBGraphicsPixmapItem.h"
#include "UBGraphicsSvgItem.h"
#include "UBGraphicsPolygonItem.h"
#include "UBGraphicsVideoItem.h"
#include "UBGraphicsAudioItem.h"
#include "UBGraphicsWidgetItem.h"
#include "UBGraphicsPDFItem.h"
#include "UBGraphicsTextItem.h"

#include "UBAppleWidget.h"
#include "UBW3CWidget.h"
#include "UBGraphicsStroke.h"

#include "core/memcheck.h"

qreal UBGraphicsScene::backgroundLayerStart = -20000000.0;
qreal UBGraphicsScene::objectLayerStart = -10000000.0;
qreal UBGraphicsScene::drawingLayerStart = 0.0;
qreal UBGraphicsScene::toolLayerStart = 10000000.0;

qreal UBGraphicsScene::toolOffsetRuler = 100;
qreal UBGraphicsScene::toolOffsetProtractor = 100;
qreal UBGraphicsScene::toolOffsetTriangle = 100;
qreal UBGraphicsScene::toolOffsetCompass = 100;
qreal UBGraphicsScene::toolOffsetEraser = 200;

qreal UBGraphicsScene::toolOffsetCurtain = 1000;
qreal UBGraphicsScene::toolOffsetPointer = 1100;

qreal UBGraphicsScene::toolOffsetCache = 1000;//Didier please define offset you want

UBGraphicsScene::UBGraphicsScene(UBDocumentProxy* parent)
    : UBCoreGraphicsScene(parent)
    , mEraser(0)
    , mPointer(0)
    , mDocument(parent)
    , mDarkBackground(false)
    , mCrossedBackground(false)
    , mIsModified(true)
    , mBackgroundObject(0)
    , mPreviousWidth(0)
    , mInputDeviceIsPressed(false)
    , mArcPolygonItem(0)
    , mRenderingContext(Screen)
    , mCurrentStroke(0)
    , mShouldUseOMP(true)
    , mItemCount(0)
    , enableUndoRedoStack(true)
    , magniferControlViewWidget(0)
    , magniferDisplayViewWidget(0)

{

#ifdef __ppc__
    mShouldUseOMP = false;
#elif defined(Q_WS_MAC)
    mShouldUseOMP = QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5;
#endif

    setDocument(parent);

    mDrawingZIndex = drawingLayerStart;
    mObjectZIndex = objectLayerStart;

    mEraser = new QGraphicsEllipseItem(); // mem : owned and destroyed by the scene
    mEraser->setRect(QRect(0, 0, 0, 0));
    mEraser->setVisible(false);

    mEraser->setZValue(/*toolLayerStart + toolOffsetEraser*/2);
    mEraser->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mTools << mEraser;
    addItem(mEraser);

    mPointer = new QGraphicsEllipseItem();  // mem : owned and destroyed by the scene
    mPointer->setRect(QRect(0, 0, 20, 20));
    mPointer->setVisible(false);

    mPointer->setPen(Qt::NoPen);
    mPointer->setBrush(QBrush(QColor(255, 0, 0, 186)));

    mPointer->setZValue( /*toolLayerStart + toolOffsetPointer*/ 2);
    mPointer->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    mTools << mPointer;
    addItem(mPointer);

    if (UBApplication::applicationController)
    {
        setViewState(SceneViewState(1,
            UBApplication::applicationController->initialHScroll(),
            UBApplication::applicationController->initialVScroll()));
    }

    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChangedProcessing()));


}

UBGraphicsScene::~UBGraphicsScene()
{
    DisposeMagnifierQWidgets();

    if (mCurrentStroke)
        if (mCurrentStroke->polygons().empty())
            delete mCurrentStroke;
}

void UBGraphicsScene::selectionChangedProcessing()
{
    if (selectedItems().count())
        UBApplication::showMessage("ZValue is " + QString::number(selectedItems().first()->zValue(), 'f'));


    QList<QGraphicsItem *> allItemsList = items();
    qreal maxZ = 0.;
    for( int i = 0; i < allItemsList.size(); i++ ) {
        QGraphicsItem *nextItem = allItemsList.at(i);
        //Temporary stub. Due to ugly z-order implementation I need to do this (sankore 360)
        //z-order behavior should be reimplemented and this stub should be deleted
        if (nextItem == mBackgroundObject)
            continue;
        //Temporary stub end (sankore 360)
        if (nextItem->zValue() > maxZ)
            maxZ = nextItem->zValue();
        nextItem->setZValue(nextItem->data(UBGraphicsItemData::ItemOwnZValue).toReal());
//        nextItem->setZValue(qreal(1));
    }
    QList<QGraphicsItem *> selItemsList = selectedItems();
    for( int i = 0; i < selItemsList.size(); i++ ) {
        QGraphicsItem *nextItem = selItemsList.at(i);
        nextItem->setZValue(maxZ + 0.0001);
    }
}

// MARK: -
// MARK: Mouse/Tablet events handling

bool UBGraphicsScene::inputDevicePress(const QPointF& scenePos, const qreal& pressure)
{
    //mMesure1Ms = 0;
    //mMesure2Ms = 0;

    bool accepted = false;

    if (mInputDeviceIsPressed)
    {
        qWarning() << "scene received input device pressed, without input device release, muting event as input device move";
        accepted = inputDeviceMove(scenePos, pressure);
    }
    else
    {
        mInputDeviceIsPressed = true;

        UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController()->stylusTool();

        if (UBDrawingController::drawingController()->isDrawingTool())
        {
            qreal width = 0;

            // delete current stroke, if not assigned to any polygon
            if (mCurrentStroke)
                if (mCurrentStroke->polygons().empty())
                    delete mCurrentStroke;

            mCurrentStroke = new UBGraphicsStroke();

            if (currentTool != UBStylusTool::Line)
                width = UBDrawingController::drawingController()->currentToolWidth() * pressure;
            else
                width = UBDrawingController::drawingController()->currentToolWidth(); //ignore pressure for line tool

            width /= UBApplication::boardController->systemScaleFactor();
            width /= UBApplication::boardController->currentZoom();

            mAddedItems.clear();
            mRemovedItems.clear();

			if (UBDrawingController::drawingController()->mActiveRuler)
			{
				UBDrawingController::drawingController()->mActiveRuler->StartLine(
					scenePos, width);
			}
			else
			{
				moveTo(scenePos);
				drawLineTo(scenePos, width,
					UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Line);
			}
            accepted = true;
        }
        else if (currentTool == UBStylusTool::Eraser)
        {
            mAddedItems.clear();
            mRemovedItems.clear();
            moveTo(scenePos);

            qreal eraserWidth = UBSettings::settings()->currentEraserWidth();
            eraserWidth /= UBApplication::boardController->systemScaleFactor();
            eraserWidth /= UBApplication::boardController->currentZoom();

            eraseLineTo(scenePos, eraserWidth);
            drawEraser(scenePos, true);

            accepted = true;
        }
        else if (currentTool == UBStylusTool::Pointer)
        {
            drawPointer(scenePos, true);
            accepted = true;
        }
    }

    return accepted;
}


bool UBGraphicsScene::inputDeviceMove(const QPointF& scenePos, const qreal& pressure)
{
    bool accepted = false;

    UBDrawingController *dc = UBDrawingController::drawingController();
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)dc->stylusTool();

    QPointF position = QPointF(scenePos);

    if (currentTool == UBStylusTool::Eraser)
    {
        drawEraser(position);
        accepted = true;
    }

    if (mInputDeviceIsPressed)
    {
        if (dc->isDrawingTool())
        {
            qreal width = 0;

            if (currentTool != UBStylusTool::Line)
                width = dc->currentToolWidth() * pressure;
            else
                width = dc->currentToolWidth();//ignore pressure for line tool

            width /= UBApplication::boardController->systemScaleFactor();
            width /= UBApplication::boardController->currentZoom();

			if (dc->mActiveRuler)
			{
				dc->mActiveRuler->DrawLine(position, width);
			}
			else
			{
	            if (currentTool == UBStylusTool::Line)
		        {
                            // TODO:    Verify this beautiful implementation and check if
                            //          it is possible to optimize it
			        QLineF radius(mPreviousPoint, position);
				    qreal angle = radius.angle();
					angle = qRound(angle / 45) * 45;
	                qreal radiusLength = radius.length();
		            QPointF newPosition(
			            mPreviousPoint.x() + radiusLength * cos((angle * PI) / 180),
				        mPreviousPoint.y() - radiusLength * sin((angle * PI) / 180));
					QLineF chord(position, newPosition);
                                        if (chord.length() < qMin((int)16, (int)(radiusLength / 20)))
						position = newPosition;
				}

				drawLineTo(position, width,
					UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Line);
			}
        }
        else if (currentTool == UBStylusTool::Eraser)
        {
            qreal eraserWidth = UBSettings::settings()->currentEraserWidth();
            eraserWidth /= UBApplication::boardController->systemScaleFactor();
            eraserWidth /= UBApplication::boardController->currentZoom();

            eraseLineTo(position, eraserWidth);
        }
        else if (currentTool == UBStylusTool::Pointer)
        {
            drawPointer(position);
        }

        accepted = true;
    }

    return accepted;
}


bool UBGraphicsScene::inputDeviceRelease()
{
    /*
    if (mMesure1Ms > 0 ||  mMesure2Ms > 0)
    {
        qWarning() << "---------------------------";
        qWarning() << "mMesure1Ms: " << mMesure1Ms;
        qWarning() << "mMesure2Ms: " << mMesure2Ms;

        mMesure1Ms = 0;
        mMesure2Ms = 0;
    }
    */

    bool accepted = false;



    if (mPointer)
    {
        mPointer->hide();
        accepted = true;
    }

    UBDrawingController *dc = UBDrawingController::drawingController();
    if (dc->isDrawingTool()) 
    {
        if (mCurrentStroke)
        {
            if (mCurrentStroke->polygons().empty())
                delete mCurrentStroke;
            mCurrentStroke = 0;
        } 
    } 
   
    if (mRemovedItems.size() > 0 || mAddedItems.size() > 0)
    {

        if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
            UBGraphicsItemUndoCommand* udcmd = new UBGraphicsItemUndoCommand(this, mRemovedItems, mAddedItems); //deleted by the undoStack

            if(UBApplication::undoStack)
                UBApplication::undoStack->push(udcmd);
        }

        mRemovedItems.clear();
        mAddedItems.clear();
        accepted = true;
    }

    mInputDeviceIsPressed = false;

    setDocumentUpdated();

    return accepted;
}

// MARK: -

void UBGraphicsScene::drawEraser(const QPointF &pPoint, bool isFirstDraw)
{
    qreal eraserWidth = UBSettings::settings()->currentEraserWidth();
    eraserWidth /= UBApplication::boardController->systemScaleFactor();
    eraserWidth /= UBApplication::boardController->currentZoom();

    qreal eraserRadius = eraserWidth / 2;

    // TODO UB 4.x optimize - no need to do that every time we move it
    if (mEraser)
    {
        mEraser->setRect(QRectF(pPoint.x() - eraserRadius, pPoint.y() - eraserRadius, eraserWidth, eraserWidth));

        if(isFirstDraw)
        {
            qreal maxZ = 0.;
            QList<QGraphicsItem *> allItemsList = items();
            for( int i = 0; i < allItemsList.size(); i++ )
            {
                QGraphicsItem *nextItem = allItemsList.at(i);
                qreal zValue = nextItem->zValue();
                if (zValue > maxZ)
                    maxZ = zValue;
                nextItem->setZValue(nextItem->data(UBGraphicsItemData::ItemOwnZValue).toReal());
            }

            mEraser->setZValue(maxZ + 0.0001);
            mEraser->show();
        }
    }
}

void UBGraphicsScene::drawPointer(const QPointF &pPoint, bool isFirstDraw)
{
    qreal pointerDiameter = UBSettings::pointerDiameter / UBApplication::boardController->currentZoom();
    qreal pointerRadius = pointerDiameter / 2;

    // TODO UB 4.x optimize - no need to do that every time we move it
    if (mPointer)
    {
        mPointer->setRect(QRectF(pPoint.x() - pointerRadius,
                pPoint.y() - pointerRadius,
                pointerDiameter,
                pointerDiameter));

        if(isFirstDraw)
        {
            qreal maxZ = 0.;
            QList<QGraphicsItem *> allItemsList = items();
            for( int i = 0; i < allItemsList.size(); i++ )
            {
                QGraphicsItem *nextItem = allItemsList.at(i);
                qreal zValue = nextItem->zValue();
                if (zValue > maxZ)
                    maxZ = zValue;
                nextItem->setZValue(nextItem->data(UBGraphicsItemData::ItemOwnZValue).toReal());
            }

            mPointer->setZValue(maxZ + 0.0001);
            mPointer->show();
        }
    }
}

// call this function when user release mouse button in Magnifier mode
void UBGraphicsScene::DisposeMagnifierQWidgets()
{
    if(magniferControlViewWidget) 
    {
        magniferControlViewWidget->hide();
        magniferControlViewWidget->setParent(0);
        delete magniferControlViewWidget;
        magniferControlViewWidget = NULL;
    }

    if(magniferDisplayViewWidget) 
    {
        magniferDisplayViewWidget->hide();
        magniferDisplayViewWidget->setParent(0);
        delete magniferDisplayViewWidget;
        magniferDisplayViewWidget = NULL;
    }

    // some time have crash here on access to app (when call from destructor when close sankore app)
    // so i just add try/catch section here
    try
    {
        UBApplication::app()->restoreOverrideCursor();
    }
    catch (...)
    {
    }
    
}

void UBGraphicsScene::moveTo(const QPointF &pPoint)
{
    mPreviousPoint = pPoint;
    mPreviousWidth = -1.0;
    mPreviousPolygonItems.clear();
    mArcPolygonItem = 0;
}


void UBGraphicsScene::drawLineTo(const QPointF &pEndPoint, const qreal &pWidth, bool bLineStyle)
{
    if (mPreviousWidth == -1.0)
        mPreviousWidth = pWidth;

    UBGraphicsPolygonItem *polygonItem = lineToPolygonItem(QLineF(mPreviousPoint, pEndPoint), pWidth);

    if (!polygonItem->brush().isOpaque())
    {
        for (int i = 0; i < mPreviousPolygonItems.size(); i++)
        {
            UBGraphicsPolygonItem* previous = mPreviousPolygonItems.value(i);
            polygonItem->subtract(previous);
        }
    }

	if (bLineStyle)
    {
        QSetIterator<QGraphicsItem*> itItems(mAddedItems);

        while (itItems.hasNext())
        {
            QGraphicsItem* item = itItems.next();
            removeItem(item);
        }
        mAddedItems.clear();
    }

    mAddedItems.insert(polygonItem);

    if (mCurrentStroke)
    {
        polygonItem->setStroke(mCurrentStroke);
    }

    addItem(polygonItem);

    mPreviousPolygonItems.append(polygonItem);

	if (!bLineStyle)
    {
        mPreviousPoint = pEndPoint;
        mPreviousWidth = pWidth;
    }
}


void UBGraphicsScene::eraseLineTo(const QPointF &pEndPoint, const qreal &pWidth)
{
    const QLineF line(mPreviousPoint, pEndPoint);

    const QPolygonF eraserPolygon = UBGeometryUtils::lineToPolygon(line, pWidth);
    const QRectF eraserBoundingRect = eraserPolygon.boundingRect();
    const QRectF eraserInnerRect = UBGeometryUtils::lineToInnerRect(line, pWidth);

    QPainterPath eraserPathVar;
    eraserPathVar.addPolygon(eraserPolygon);
    const QPainterPath eraserPath = eraserPathVar;

    QList<QGraphicsItem*> collidItems = items(eraserBoundingRect, Qt::IntersectsItemBoundingRect);

    QSet<QGraphicsItem*> toBeAddedItems;
    QSet<QGraphicsItem*> toBeRemovedItems;

    int collidItemsSize = collidItems.size();

    toBeAddedItems.reserve(collidItemsSize);
    toBeRemovedItems.reserve(collidItemsSize);

    if (mShouldUseOMP)
    {
#pragma omp parallel for
        for (int i = 0; i < collidItemsSize; i++)
        {
            UBGraphicsPolygonItem *collidingPolygonItem
                = qgraphicsitem_cast<UBGraphicsPolygonItem*> (collidItems.at(i));

            if (collidingPolygonItem)
            {
                if(eraserInnerRect.contains(collidingPolygonItem->boundingRect()))
                {
#pragma omp critical
                    toBeRemovedItems << collidingPolygonItem;
                }
                else
                {
                    QPolygonF collidingPolygon = collidingPolygonItem->polygon();
                    QPainterPath collidingPath;
                    collidingPath.addPolygon(collidingPolygon);

                    QPainterPath croppedPath = collidingPath.subtracted(eraserPath);
                    QPainterPath croppedPathSimplified = croppedPath.simplified();

                    if (croppedPath == collidingPath)
                    {
                        // NOOP
                    }
                    else if (croppedPathSimplified.isEmpty())
                    {
#pragma omp critical
                        toBeRemovedItems << collidingPolygonItem;
                    }
                    else
                    {
                        foreach(const QPolygonF &pol, croppedPathSimplified.toFillPolygons())
                        {
                            UBGraphicsPolygonItem* croppedPolygonItem = collidingPolygonItem->deepCopy(pol);
#pragma omp critical
                            toBeAddedItems << croppedPolygonItem;
                        }
#pragma omp critical
                        toBeRemovedItems << collidingPolygonItem;
                    }
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < collidItemsSize; i++)
        {
            UBGraphicsPolygonItem *collidingPolygonItem
                = qgraphicsitem_cast<UBGraphicsPolygonItem*> (collidItems.at(i));

            if (collidingPolygonItem)
            {
                if(eraserInnerRect.contains(collidingPolygonItem->boundingRect()))
                {
                    toBeRemovedItems << collidingPolygonItem;
                }
                else
                {
                    QPolygonF collidingPolygon = collidingPolygonItem->polygon();
                    QPainterPath collidingPath;
                    collidingPath.addPolygon(collidingPolygon);

                    QPainterPath croppedPath = collidingPath.subtracted(eraserPath);
                    QPainterPath croppedPathSimplified = croppedPath.simplified();

                    if (croppedPath == collidingPath)
                    {
                        // NOOP
                    }
                    else if (croppedPathSimplified.isEmpty())
                    {
                        toBeRemovedItems << collidingPolygonItem;
                    }
                    else
                    {
                        foreach(const QPolygonF &pol, croppedPathSimplified.toFillPolygons())
                        {
                            UBGraphicsPolygonItem* croppedPolygonItem = collidingPolygonItem->deepCopy(pol);
                            toBeAddedItems << croppedPolygonItem;
                        }

                        toBeRemovedItems << collidingPolygonItem;
                    }
                }
            }
        }
    }

    addItems(toBeAddedItems);
    mAddedItems += toBeAddedItems;

    removeItems(toBeRemovedItems);
    mRemovedItems += toBeRemovedItems;

    mPreviousPoint = pEndPoint;
}


void UBGraphicsScene::drawArcTo(const QPointF& pCenterPoint, qreal pSpanAngle)
{
    if (mArcPolygonItem)
    {
        mAddedItems.remove(mArcPolygonItem);
        removeItem(mArcPolygonItem);
        mArcPolygonItem = 0;
    }
    qreal penWidth = UBSettings::settings()->currentPenWidth();
    penWidth /= UBApplication::boardController->systemScaleFactor();
    penWidth /= UBApplication::boardController->currentZoom();

    mArcPolygonItem = arcToPolygonItem(QLineF(pCenterPoint, mPreviousPoint), pSpanAngle, penWidth);
    mAddedItems.insert(mArcPolygonItem);
    addItem(mArcPolygonItem);

    setDocumentUpdated();
}


qreal UBGraphicsScene::getNextDrawingZIndex()
{
    mDrawingZIndex = mDrawingZIndex + 1.0;
    return mDrawingZIndex;
}


qreal UBGraphicsScene::getNextObjectZIndex()
{
    mObjectZIndex = mObjectZIndex + 1.0;
    return mObjectZIndex;
}


void UBGraphicsScene::setBackground(bool pIsDark, bool pIsCrossed)
{
    bool needRepaint = false;

    if (mDarkBackground != pIsDark)
    {
        mDarkBackground = pIsDark;

        if (mEraser)
        {
            if (mDarkBackground)
            {
                mEraser->setBrush(UBSettings::eraserBrushDarkBackground);
                mEraser->setPen(UBSettings::eraserPenDarkBackground);
            }
            else
            {
                mEraser->setBrush(UBSettings::eraserBrushLightBackground);
                mEraser->setPen(UBSettings::eraserPenLightBackground);
            }
        }

        recolorAllItems();

        needRepaint = true;
        setModified(true);
    }

    if (mCrossedBackground != pIsCrossed)
    {
        mCrossedBackground = pIsCrossed;
        needRepaint = true;
        setModified(true);
    }

    if (needRepaint)
    {
        foreach(QGraphicsView* view, views())
        {
            view->resetCachedContent();
        }
    }
}


void UBGraphicsScene::recolorAllItems()
{
    QMap<QGraphicsView*, QGraphicsView::ViewportUpdateMode> previousUpdateModes;
    foreach(QGraphicsView* view, views())
    {
        previousUpdateModes.insert(view, view->viewportUpdateMode());
        view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    }

    for(int i = 0; i < mFastAccessItems.size(); i++)
    {
        UBGraphicsPolygonItem *polygonItem = qgraphicsitem_cast<UBGraphicsPolygonItem*> (mFastAccessItems.at(i));

        if (polygonItem)
        {
            QColor color;

            if (mDarkBackground)
            {
                color = polygonItem->colorOnDarkBackground();
            }
            else
            {
                color = polygonItem->colorOnLightBackground();
            }

            polygonItem->setColor(color);
            continue;
        }
    }

    foreach(QGraphicsView* view, views())
    {
        view->setViewportUpdateMode(previousUpdateModes.value(view));
    }
}


UBGraphicsPolygonItem* UBGraphicsScene::lineToPolygonItem(const QLineF &pLine, const qreal &pWidth)
{
    UBGraphicsPolygonItem *polygonItem = new UBGraphicsPolygonItem(pLine, pWidth);

    initPolygonItem(polygonItem);

    return polygonItem;
}


void UBGraphicsScene::initPolygonItem(UBGraphicsPolygonItem* polygonItem)
{
    QColor colorOnDarkBG;
    QColor colorOnLightBG;

    if (UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Marker)
    {
        colorOnDarkBG = UBApplication::boardController->markerColorOnDarkBackground();
        colorOnLightBG = UBApplication::boardController->markerColorOnLightBackground();
    }
    else // settings->stylusTool() == UBStylusTool::Pen + failsafe
    {
        colorOnDarkBG = UBApplication::boardController->penColorOnDarkBackground();
        colorOnLightBG = UBApplication::boardController->penColorOnLightBackground();
    }

    if (mDarkBackground)
    {
        polygonItem->setColor(colorOnDarkBG);
    }
    else
    {
        polygonItem->setColor(colorOnLightBG);
    }

    polygonItem->setColorOnDarkBackground(colorOnDarkBG);
    polygonItem->setColorOnLightBackground(colorOnLightBG);

    polygonItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Graphic));

//    polygonItem->setZValue(getNextDrawingZIndex());
    UBGraphicsItem::assignZValue(polygonItem, getNextDrawingZIndex());
}


UBGraphicsPolygonItem* UBGraphicsScene::arcToPolygonItem(const QLineF& pStartRadius, qreal pSpanAngle, qreal pWidth)
{
    QPolygonF polygon = UBGeometryUtils::arcToPolygon(pStartRadius, pSpanAngle, pWidth);

    return polygonToPolygonItem(polygon);
}


UBGraphicsPolygonItem* UBGraphicsScene::polygonToPolygonItem(const QPolygonF pPolygon)
{
    UBGraphicsPolygonItem *polygonItem = new UBGraphicsPolygonItem(pPolygon);

    initPolygonItem(polygonItem);

    return polygonItem;
}


void UBGraphicsScene::hideEraser()
{
    if (mEraser)
        mEraser->hide();
}


void UBGraphicsScene::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    hideEraser();
}


UBGraphicsScene* UBGraphicsScene::sceneDeepCopy() const
{
    UBGraphicsScene* copy = new UBGraphicsScene(this->document());

    copy->setBackground(this->isDarkBackground(), this->isCrossedBackground());

    copy->mDrawingZIndex = this->mDrawingZIndex;
    copy->mObjectZIndex = this->mObjectZIndex;
    copy->setSceneRect(this->sceneRect());

    if (this->mNominalSize.isValid())
        copy->setNominalSize(this->mNominalSize);

    QListIterator<QGraphicsItem*> itItems(this->mFastAccessItems);

    QMap<UBGraphicsStroke*, UBGraphicsStroke*> groupClone;

    while (itItems.hasNext())
    {
        QGraphicsItem* item = itItems.next();
        QGraphicsItem* cloneItem = 0;

        UBItem* ubItem = dynamic_cast<UBItem*>(item);
        UBGraphicsStroke* stroke = dynamic_cast<UBGraphicsStroke*>(item);

        if (ubItem && !stroke)
        {
            cloneItem = dynamic_cast<QGraphicsItem*>(ubItem->deepCopy());
        }

        if (cloneItem)
        {
            copy->addItem(cloneItem);

            if (isBackgroundObject(item))
            {
                copy->setAsBackgroundObject(cloneItem);
            }

            if (this->mTools.contains(item))
            {
                copy->mTools << cloneItem;
            }

            UBGraphicsPolygonItem* polygon = dynamic_cast<UBGraphicsPolygonItem*>(item);

            if(polygon)
            {
                UBGraphicsStroke* stroke = dynamic_cast<UBGraphicsStroke*>(item->parentItem());

                if (stroke)
                {
                    UBGraphicsStroke* cloneStroke = groupClone.value(stroke);

                    if (!cloneStroke)
                    {
                        cloneStroke = stroke->deepCopy();
                        groupClone.insert(stroke, cloneStroke);
                    }

                    polygon->setStroke(cloneStroke);
                }
            }
        }
    }

    // TODO UB 4.7 ... complete all members ?

    return copy;
}


UBItem* UBGraphicsScene::deepCopy() const
{
    return sceneDeepCopy();
}


void UBGraphicsScene::clearItemsAndAnnotations()
{
    deselectAllItems();

    QSet<QGraphicsItem*> emptyList;
    QSet<QGraphicsItem*> removedItems;

    QListIterator<QGraphicsItem*> itItems(mFastAccessItems);

    while (itItems.hasNext())
    {
        QGraphicsItem* item = itItems.next();

        if(!mTools.contains(item) && !isBackgroundObject(item))
        {
            removeItem(item);
            removedItems << item;
        }
    }

    // force refresh, QT is a bit lazy and take a lot of time (nb item ^2 ?) to trigger repaint
    update(sceneRect());

    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, removedItems, emptyList);
        UBApplication::undoStack->push(uc);
    }

    setDocumentUpdated();
}


void UBGraphicsScene::clearItems()
{
    deselectAllItems();

    QSet<QGraphicsItem*> emptyList;
    QSet<QGraphicsItem*> removedItems;

    QListIterator<QGraphicsItem*> itItems(mFastAccessItems);

    while (itItems.hasNext())
    {
        QGraphicsItem* item = itItems.next();

        if (!item->parentItem())
        {
            UBGraphicsPolygonItem* pi = qgraphicsitem_cast<UBGraphicsPolygonItem*>(item);

            if(!pi && !mTools.contains(item) && !isBackgroundObject(item))
            {
                removeItem(item);
                removedItems << item;
            }
        }
    }

    // force refresh, QT is a bit lazy and take a lot of time (nb item ^2 ?) to trigger repaint
    update(sceneRect());


    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, removedItems, emptyList);
        UBApplication::undoStack->push(uc);
    }

    setDocumentUpdated();
}


void UBGraphicsScene::clearAnnotations()
{
    QSet<QGraphicsItem*> emptyList;
    QSet<QGraphicsItem*> removedItems;

    QListIterator<QGraphicsItem*> itItems(mFastAccessItems);

    while (itItems.hasNext())
    {
        QGraphicsItem* item = itItems.next();
        UBGraphicsPolygonItem* pi = qgraphicsitem_cast<UBGraphicsPolygonItem*>(item);
        if (pi)
        {
            removeItem(item);
            removedItems << item;
        }
    }

    // force refresh, QT is a bit lazy and take a lot of time (nb item ^2 ?) to trigger repaint
    update(sceneRect());


    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, removedItems, emptyList);
        UBApplication::undoStack->push(uc);
    }

    setDocumentUpdated();
}


UBGraphicsPixmapItem* UBGraphicsScene::addPixmap(const QPixmap& pPixmap, const QPointF& pPos, qreal pScaleFactor, bool pUseAnimation)
{
    UBGraphicsPixmapItem* pixmapItem = new UBGraphicsPixmapItem();

    pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
//    pixmapItem->setZValue(getNextObjectZIndex());
    UBGraphicsItem::assignZValue(pixmapItem, getNextObjectZIndex());

    pixmapItem->setPixmap(pPixmap);

    QPointF half(pPixmap.width() * pScaleFactor / 2, pPixmap.height()  * pScaleFactor / 2);
    pixmapItem->setPos(pPos - half);

    addItem(pixmapItem);

    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, 0, pixmapItem);
        UBApplication::undoStack->push(uc);
    }

    pixmapItem->scale(pScaleFactor, pScaleFactor);

    if (pUseAnimation)
    {
        pixmapItem->setOpacity(0);

        QPropertyAnimation *animation = new QPropertyAnimation(pixmapItem, "opacity");
        animation->setDuration(1000);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);

        animation->start();
    }

    pixmapItem->show();
    setDocumentUpdated();

    return pixmapItem;
}


void UBGraphicsScene::textUndoCommandAdded(UBGraphicsTextItem *textItem)
{
    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsTextItemUndoCommand* uc = new UBGraphicsTextItemUndoCommand(textItem);
        UBApplication::undoStack->push(uc);
    }
}


UBGraphicsVideoItem* UBGraphicsScene::addVideo(const QUrl& pVideoFileUrl, bool shouldPlayAsap, const QPointF& pPos)
{
    UBGraphicsVideoItem* videoItem = new UBGraphicsVideoItem(pVideoFileUrl);

    videoItem->setPos(pPos);

    videoItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    videoItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
//    videoItem->setZValue(getNextObjectZIndex());
    UBGraphicsItem::assignZValue(videoItem, getNextObjectZIndex());

    addItem(videoItem);

    videoItem->show();

    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, 0, videoItem);
        UBApplication::undoStack->push(uc);
    }

    videoItem->mediaObject()->play();

    if (!shouldPlayAsap)
    {
        videoItem->mediaObject()->pause();
        videoItem->mediaObject()->seek(0);
    }

    setDocumentUpdated();

    return videoItem;
}

UBGraphicsAudioItem* UBGraphicsScene::addAudio(const QUrl& pAudioFileUrl, bool shouldPlayAsap, const QPointF& pPos)
{
    UBGraphicsAudioItem* audioItem = new UBGraphicsAudioItem(pAudioFileUrl);

    audioItem->setPos(pPos);

    audioItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    audioItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
//    audioItem->setZValue(getNextObjectZIndex());
    UBGraphicsItem::assignZValue(audioItem, getNextObjectZIndex());

    addItem(audioItem);

    audioItem->show();

    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, 0, audioItem);
        UBApplication::undoStack->push(uc);
    }

    audioItem->mediaObject()->play();

    if (!shouldPlayAsap)
    {
        audioItem->mediaObject()->pause();
        audioItem->mediaObject()->seek(0);
    }

    setDocumentUpdated();

    return audioItem;
}


UBGraphicsWidgetItem* UBGraphicsScene::addWidget(const QUrl& pWidgetUrl, const QPointF& pPos)
{
    int widgetType = UBAbstractWidget::widgetType(pWidgetUrl);

    if(widgetType == UBWidgetType::Apple)
    {
        return addAppleWidget(pWidgetUrl, pPos);
    }
    else if(widgetType == UBWidgetType::W3C)
    {
        return addW3CWidget(pWidgetUrl, pPos);
    }
    else
    {
        qDebug() << "UBGraphicsScene::addWidget: Unknown widget Type";
        return 0;
    }
}


UBGraphicsAppleWidgetItem* UBGraphicsScene::addAppleWidget(const QUrl& pWidgetUrl, const QPointF& pPos)
{
    UBGraphicsAppleWidgetItem *appleWidget = new UBGraphicsAppleWidgetItem(pWidgetUrl);

    addGraphicsWidget(appleWidget, pPos);

    return appleWidget;
}


UBGraphicsW3CWidgetItem* UBGraphicsScene::addW3CWidget(const QUrl& pWidgetUrl, const QPointF& pPos, int widgetType)
{
    UBGraphicsW3CWidgetItem *w3CWidget = new UBGraphicsW3CWidgetItem(pWidgetUrl, 0, widgetType);

    addGraphicsWidget(w3CWidget, pPos);

    return w3CWidget;
}

void UBGraphicsScene::addGraphicsWidget(UBGraphicsWidgetItem* graphicsWidget, const QPointF& pPos)
{
    graphicsWidget->setFlag(QGraphicsItem::ItemIsSelectable, true);
//    graphicsWidget->setZValue(getNextObjectZIndex());
    UBGraphicsItem::assignZValue(graphicsWidget, getNextObjectZIndex());

    addItem(graphicsWidget);

    qreal ssf = 1 / UBApplication::boardController->systemScaleFactor();

    graphicsWidget->scale(ssf, ssf);

    graphicsWidget->setPos(QPointF(pPos.x() - graphicsWidget->boundingRect().width() / 2,
        pPos.y() - graphicsWidget->boundingRect().height() / 2));

    if (graphicsWidget->widgetWebView()->canBeContent())
    {
//        graphicsWidget->widgetWebView()->loadMainHtml();

        graphicsWidget->setSelected(true);
        if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
            UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, 0, graphicsWidget);
            UBApplication::undoStack->push(uc);
        }

        setDocumentUpdated();
    }
    else
    {
        UBApplication::boardController->moveGraphicsWidgetToControlView(graphicsWidget);
    }
}


UBGraphicsW3CWidgetItem* UBGraphicsScene::addOEmbed(const QUrl& pContentUrl, const QPointF& pPos)
{
    QStringList widgetPaths = UBPersistenceManager::persistenceManager()->allWidgets(UBSettings::settings()->uniboardInteractiveLibraryDirectory());

    UBGraphicsW3CWidgetItem *widget = 0;

    foreach(QString widgetPath, widgetPaths)
    {
        if (widgetPath.contains("VideoPicker"))
        {
            widget = addW3CWidget(QUrl::fromLocalFile(widgetPath), pPos);

            if (widget)
            {
                widget->setPreference("oembedUrl", pContentUrl.toString());
                setDocumentUpdated();
                break;
            }
        }
    }

    return widget;
}

UBGraphicsSvgItem* UBGraphicsScene::addSvg(const QUrl& pSvgFileUrl, const QPointF& pPos)
{
    QString path = pSvgFileUrl.toLocalFile();

    UBGraphicsSvgItem *svgItem = new UBGraphicsSvgItem(path);

    svgItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    svgItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
//    svgItem->setZValue(getNextObjectZIndex());
    UBGraphicsItem::assignZValue(svgItem, getNextObjectZIndex());

    qreal sscale = 1 / UBApplication::boardController->systemScaleFactor();
    svgItem->scale(sscale, sscale);

    QPointF half(svgItem->boundingRect().width() / 2, svgItem->boundingRect().height() / 2);
    svgItem->setPos(pPos - half);
    svgItem->show();

    addItem(svgItem);

    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, 0, svgItem);
        UBApplication::undoStack->push(uc);
    }

    setDocumentUpdated();

    return svgItem;
}


UBGraphicsTextItem* UBGraphicsScene::addText(const QString& pString, const QPointF& pTopLeft)
{
    return addTextWithFont(pString, pTopLeft, UBSettings::settings()->fontPixelSize()
            , UBSettings::settings()->fontFamily(), UBSettings::settings()->isBoldFont()
            , UBSettings::settings()->isItalicFont());
}


UBGraphicsTextItem* UBGraphicsScene::addTextWithFont(const QString& pString, const QPointF& pTopLeft
            , int pointSize, const QString& fontFamily, bool bold, bool italic)
{
    UBGraphicsTextItem *textItem = new UBGraphicsTextItem();
    textItem->setPlainText(pString);
//    textItem->setZValue(getNextObjectZIndex());
    UBGraphicsItem::assignZValue(textItem, getNextObjectZIndex());

    QFont font = textItem->font();

    if (fontFamily == "")
    {
        font = QFont(UBSettings::settings()->fontFamily());
    }
    else
    {
        font = QFont(fontFamily);
    }

    if (pointSize < 1)
    {
        font.setPixelSize(UBSettings::settings()->fontPixelSize());
    }
    else
    {
        font.setPointSize(pointSize);
    }

    font.setBold(bold);
    font.setItalic(italic);
    textItem->setFont(font);

    QFontMetrics fi(font);
    QRect br = fi.boundingRect(pString);

    textItem->setTextWidth(qMax((qreal)br.width() + 50, (qreal)200));
    textItem->setTextHeight(br.height());

    addItem(textItem);

    textItem->setPos(pTopLeft);

    textItem->show();

    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, 0, textItem);
        UBApplication::undoStack->push(uc);
    }

    connect(textItem, SIGNAL(textUndoCommandAdded(UBGraphicsTextItem *)), this, SLOT(textUndoCommandAdded(UBGraphicsTextItem *)));

    textItem->setSelected(true);
    textItem->setFocus();

    setDocumentUpdated();

    return textItem;
}
UBGraphicsTextItem *UBGraphicsScene::addTextHtml(const QString &pString, const QPointF& pTopLeft)
{
    UBGraphicsTextItem *textItem = new UBGraphicsTextItem();
    textItem->setPlainText("");
    textItem->setHtml(pString);
//    textItem->setZValue(getNextObjectZIndex());
    UBGraphicsItem::assignZValue(textItem, getNextObjectZIndex());

    addItem(textItem);
    textItem->show();

    if (enableUndoRedoStack) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(this, 0, textItem);
        UBApplication::undoStack->push(uc);
    }

    connect(textItem, SIGNAL(textUndoCommandAdded(UBGraphicsTextItem *)),
            this,     SLOT(textUndoCommandAdded(UBGraphicsTextItem *)));

    textItem->setFocus();

    setDocumentUpdated();
    textItem->setPos(pTopLeft);

    return textItem;
}

void UBGraphicsScene::addItem(QGraphicsItem* item)
{
    setModified(true);
    UBCoreGraphicsScene::addItem(item);

    if (!mTools.contains(item))
      ++mItemCount;

    mFastAccessItems << item;
}

void UBGraphicsScene::addItems(const QSet<QGraphicsItem*>& items)
{
    setModified(true);

    foreach(QGraphicsItem* item, items)
        UBCoreGraphicsScene::addItem(item);

    mItemCount += items.size();

    mFastAccessItems += items.toList();
}

void UBGraphicsScene::removeItem(QGraphicsItem* item)
{
    setModified(true);
    UBCoreGraphicsScene::removeItem(item);

    if (!mTools.contains(item))
      --mItemCount;

    mFastAccessItems.removeAll(item);
}


void UBGraphicsScene::removeItems(const QSet<QGraphicsItem*>& items)
{
    setModified(true);

    foreach(QGraphicsItem* item, items)
        UBCoreGraphicsScene::removeItem(item);

    mItemCount -= items.size();

    foreach(QGraphicsItem* item, items)
        mFastAccessItems.removeAll(item);
}


void UBGraphicsScene::deselectAllItems()
{
    foreach(QGraphicsItem *gi, selectedItems ())
    {
        gi->setSelected(false);
    }
}

bool UBGraphicsScene::isEmpty() const
{
    return mItemCount == 0;
}

QGraphicsItem* UBGraphicsScene::setAsBackgroundObject(QGraphicsItem* item, bool pAdaptTransformation, bool pExpand)
{
    if (mBackgroundObject)
    {
        removeItem(mBackgroundObject);
        mBackgroundObject = 0;
    }

    if (item)
    {
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        item->setFlag(QGraphicsItem::ItemIsMovable, false);
        item->setAcceptedMouseButtons(Qt::NoButton);
        item->setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::FixedBackground);

//        item->setZValue(backgroundLayerStart);
        UBGraphicsItem::assignZValue(item, backgroundLayerStart);

        if (pAdaptTransformation)
        {
            item = scaleToFitDocumentSize(item, true, 0, pExpand);
        }

        if (item->scene() != this)
            addItem(item);

        mBackgroundObject = item;

    }

    return item;
}


QRectF UBGraphicsScene::normalizedSceneRect(qreal ratio)
{

    QRectF normalizedRect(nominalSize().width() / -2, nominalSize().height() / -2,
        nominalSize().width(), nominalSize().height());

    foreach(QGraphicsItem* gi, mFastAccessItems)
    {
        if(gi && gi->isVisible() && !mTools.contains(gi))
        {
            normalizedRect = normalizedRect.united(gi->sceneBoundingRect());
        }
    }

    if (ratio > 0.0)
    {
        qreal normalizedRectRatio = normalizedRect.width() / normalizedRect.height();

        if (normalizedRectRatio > ratio)
        {
            //the normalized rect is too wide, we increase height
            qreal newHeight = normalizedRect.width() / ratio;
            qreal offset = (newHeight - normalizedRect.height()) / 2;
            normalizedRect.setY(normalizedRect.y() - offset);
            normalizedRect.setHeight(newHeight);
        }
        else if (normalizedRectRatio < ratio)
        {
            //the normalized rect is too high, we increase the width
            qreal newWidth = normalizedRect.height() * ratio;
            qreal offset = (newWidth - normalizedRect.width()) / 2;
            normalizedRect.setX(normalizedRect.x() - offset);
            normalizedRect.setWidth(newWidth);
        }
    }

    return normalizedRect;
}


void UBGraphicsScene::setDocument(UBDocumentProxy* pDocument)
{
    if (pDocument != mDocument)
    {
        if (mDocument)
        {
            setModified(true);
        }

        mDocument = pDocument;
        setParent(pDocument);
    }
}


QGraphicsItem* UBGraphicsScene::scaleToFitDocumentSize(QGraphicsItem* item, bool center, int margin, bool expand)
{
    int maxWidth = mNominalSize.width() - (margin * 2);
    int maxHeight = mNominalSize.height() - (margin * 2);

    QRectF size = item->sceneBoundingRect();

    if (expand || size.width() > maxWidth || size.height() > maxHeight)
    {
        qreal ratio = qMin(maxWidth / size.width(), maxHeight / size.height());

        item->scale(ratio, ratio);

        if(center)
        {
            item->setPos(item->sceneBoundingRect().width() / -2.0,
                item->sceneBoundingRect().height() / -2.0);
        }
    }

    return item;
}


void UBGraphicsScene::addRuler(QPointF center)
{
    UBGraphicsRuler* ruler = new UBGraphicsRuler(); // mem : owned and destroyed by the scene
    mTools << ruler;
    QRectF rect = ruler->rect();
    ruler->setRect(center.x() - rect.width()/2, center.y() - rect.height()/2, rect.width(), rect.height());

//    ruler->setZValue(toolLayerStart + toolOffsetRuler);
    UBGraphicsItem::assignZValue(ruler, toolLayerStart + toolOffsetRuler);

    ruler->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    addItem(ruler);

    ruler->setVisible(true);
    setModified(true);
}


void UBGraphicsScene::addProtractor(QPointF center)
{
    // Protractor

    UBGraphicsProtractor* protractor = new UBGraphicsProtractor(); // mem : owned and destroyed by the scene
    mTools << protractor;

//    protractor->setZValue(toolLayerStart + toolOffsetProtractor);
    UBGraphicsItem::assignZValue(protractor, toolLayerStart + toolOffsetProtractor);

    protractor->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    addItem(protractor);

    QPointF itemSceneCenter = protractor->sceneBoundingRect().center();
    protractor->moveBy(center.x() - itemSceneCenter.x(), center.y() - itemSceneCenter.y());

    protractor->setVisible(true);
    setModified(true);
}

void UBGraphicsScene::addTriangle(QPointF center)
{
    // Triangle

    UBGraphicsTriangle* triangle = new UBGraphicsTriangle(); // mem : owned and destroyed by the scene
    mTools << triangle;

//    triangle->setZValue(toolLayerStart + toolOffsetProtractor);
    UBGraphicsItem::assignZValue(triangle, toolLayerStart + toolOffsetTriangle);

    triangle->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    addItem(triangle);

    QPointF itemSceneCenter = triangle->sceneBoundingRect().center();
    triangle->moveBy(center.x() - itemSceneCenter.x(), center.y() - itemSceneCenter.y());

    triangle->setVisible(true);
    setModified(true);
}

void UBGraphicsScene::addMagnifier(UBMagnifierParams params)
{
    // can have only one magnifier at one time
    if(magniferControlViewWidget) return;

    QWidget *cContainer = (QWidget*)(UBApplication::boardController->controlContainer());
    QGraphicsView *cView = (QGraphicsView*)UBApplication::boardController->controlView();
    QGraphicsView *dView = (QGraphicsView*)UBApplication::boardController->displayView();

    QPoint dvZeroPoint = dView->mapToGlobal(QPoint(0,0));

    int cvW = cView->width();
    int dvW = dView->width();
    qreal wCoeff = (qreal)dvW / (qreal)cvW;

    int cvH = cView->height();
    int dvH = dView->height();
    qreal hCoeff = (qreal)dvH / (qreal)cvH;

    QPoint ccPoint(params.x,params.y);
    QPoint globalPoint = cContainer->mapToGlobal(ccPoint);
    QPoint cvPoint = cView->mapFromGlobal(globalPoint);
    QPoint dvPoint( cvPoint.x() * wCoeff + dvZeroPoint.x(), cvPoint.y() * hCoeff + dvZeroPoint.y());

    magniferControlViewWidget = new UBMagnifier((QWidget*)(UBApplication::boardController->controlContainer()), true);
    magniferControlViewWidget->setGrabView((QGraphicsView*)UBApplication::boardController->controlView());
    magniferControlViewWidget->setMoveView((QWidget*)(UBApplication::boardController->controlContainer()));
    magniferControlViewWidget->setSize(params.sizePercentFromScene);
    magniferControlViewWidget->setZoom(params.zoom);

    magniferDisplayViewWidget = new UBMagnifier((QWidget*)(UBApplication::boardController->displayView()), false);
    magniferDisplayViewWidget->setGrabView((QGraphicsView*)UBApplication::boardController->controlView());
    magniferDisplayViewWidget->setMoveView((QGraphicsView*)UBApplication::boardController->displayView());
    magniferDisplayViewWidget->setSize(params.sizePercentFromScene);
    magniferDisplayViewWidget->setZoom(params.zoom);

    magniferControlViewWidget->grabNMove(globalPoint, globalPoint, true);
    magniferDisplayViewWidget->grabNMove(globalPoint, dvPoint, true);
    magniferControlViewWidget->show();
    magniferDisplayViewWidget->show();

    connect(magniferControlViewWidget, SIGNAL(magnifierMoved_Signal(QPoint)), this, SLOT(moveMagnifier(QPoint)));
    connect(magniferControlViewWidget, SIGNAL(magnifierClose_Signal()), this, SLOT(closeMagnifier()));
    connect(magniferControlViewWidget, SIGNAL(magnifierZoomIn_Signal()), this, SLOT(zoomInMagnifier()));
    connect(magniferControlViewWidget, SIGNAL(magnifierZoomOut_Signal()), this, SLOT(zoomOutMagnifier()));
    connect(magniferControlViewWidget, SIGNAL(magnifierResized_Signal(qreal)), this, SLOT(resizedMagnifier(qreal)));

    setModified(true);
}

void UBGraphicsScene::moveMagnifier(QPoint newPos)
{
    QWidget *cContainer = (QWidget*)(UBApplication::boardController->controlContainer());
    QGraphicsView *cView = (QGraphicsView*)UBApplication::boardController->controlView();
    QGraphicsView *dView = (QGraphicsView*)UBApplication::boardController->displayView();

    QPoint dvZeroPoint = dView->mapToGlobal(QPoint(0,0));

    int cvW = cView->width();
    int dvW = dView->width();
    qreal wCoeff = (qreal)dvW / (qreal)cvW;

    int cvH = cView->height();
    int dvH = dView->height();
    qreal hCoeff = (qreal)dvH / (qreal)cvH;

    QPoint globalPoint = cContainer->mapToGlobal(newPos);
    QPoint cvPoint = cView->mapFromGlobal(globalPoint);
    QPoint dvPoint( cvPoint.x() * wCoeff + dvZeroPoint.x(), cvPoint.y() * hCoeff + dvZeroPoint.y());

    magniferControlViewWidget->grabNMove(globalPoint, globalPoint, false, false);
    magniferDisplayViewWidget->grabNMove(globalPoint, dvPoint, false, true);

    setModified(true);
}

void UBGraphicsScene::closeMagnifier()
{
    DisposeMagnifierQWidgets();
}

void UBGraphicsScene::zoomInMagnifier()
{
    if(magniferControlViewWidget->params.zoom < 8)
    {
        magniferControlViewWidget->setZoom(magniferControlViewWidget->params.zoom + 0.5);
        magniferDisplayViewWidget->setZoom(magniferDisplayViewWidget->params.zoom + 0.5);
    }
}

void UBGraphicsScene::zoomOutMagnifier()
{
    if(magniferControlViewWidget->params.zoom > 1)
    {
        magniferControlViewWidget->setZoom(magniferControlViewWidget->params.zoom - 0.5);
        magniferDisplayViewWidget->setZoom(magniferDisplayViewWidget->params.zoom - 0.5);
    }
}

void UBGraphicsScene::resizedMagnifier(qreal newPercent)
{
    if(newPercent > 18 && newPercent < 50)
    {
        magniferControlViewWidget->setSize(newPercent);
        magniferControlViewWidget->grabPoint();
        magniferDisplayViewWidget->setSize(newPercent);
        magniferDisplayViewWidget->grabPoint();
    }
}

void UBGraphicsScene::addCompass(QPointF center)
{
    UBGraphicsCompass* compass = new UBGraphicsCompass(); // mem : owned and destroyed by the scene
    mTools << compass;
    addItem(compass);

    QRectF rect = compass->rect();
    compass->setRect(center.x() - rect.width() / 2, center.y() - rect.height() / 2, rect.width(), rect.height());

//    compass->setZValue(toolLayerStart + toolOffsetCompass);
    UBGraphicsItem::assignZValue(compass, toolLayerStart + toolOffsetCompass);

    compass->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    compass->setVisible(true);
    setModified(true);
}

void UBGraphicsScene::addCache()
{
    UBGraphicsCache* cache = new UBGraphicsCache();
    mTools << cache;

    addItem(cache);
    cache->setVisible(true);
    cache->setSelected(true);
    UBApplication::boardController->notifyCache(true);
    UBApplication::boardController->notifyPageChanged();
}

void UBGraphicsScene::addMask(const QPointF &center)
{ 
    UBGraphicsCurtainItem* curtain = new UBGraphicsCurtainItem(); // mem : owned and destroyed by the scene
    mTools << curtain;
    QGraphicsView* view;

    if(UBApplication::applicationController->displayManager()->hasDisplay())
        view = (QGraphicsView*)UBApplication::boardController->displayView();
    else
        view = (QGraphicsView*)UBApplication::boardController->controlView();

    //    curtain->setZValue(toolLayerStart + toolOffsetCurtain);
    UBGraphicsItem::assignZValue(curtain, toolLayerStart + toolOffsetCurtain);
	
    QRectF rect = UBApplication::boardController->activeScene()->normalizedSceneRect();
    rect.setSize(QSizeF(rect.width()/2, rect.height()/2));

    QPointF origin = center.isNull() ? rect.bottomRight() : center;
    curtain->setRect(rect.translated(origin - rect.topLeft() / 2));

	addItem(curtain);

    curtain->setVisible(true);
    curtain->setSelected(true);
    setModified(true);
}


void UBGraphicsScene::setRenderingQuality(UBItem::RenderingQuality pRenderingQuality)
{
    QListIterator<QGraphicsItem*> itItems(mFastAccessItems);

    while (itItems.hasNext())
    {
        QGraphicsItem *gItem =  itItems.next();

        UBItem *ubItem = dynamic_cast<UBItem*>(gItem);

        if (ubItem)
        {
            ubItem->setRenderingQuality(pRenderingQuality);
        }
    }
}


QList<QUrl> UBGraphicsScene::relativeDependencies() const
{
    QList<QUrl> relativePathes;

    QListIterator<QGraphicsItem*> itItems(mFastAccessItems);

    while (itItems.hasNext())
    {
        UBGraphicsVideoItem *videoItem = qgraphicsitem_cast<UBGraphicsVideoItem*> (itItems.next());

        if (videoItem && videoItem->mediaFileUrl().isRelative())
        {
            relativePathes << videoItem->mediaFileUrl();
        }
    }

    return relativePathes;
}


QSize UBGraphicsScene::nominalSize()
{
    if (mDocument && !mNominalSize.isValid())
    {
        mNominalSize = mDocument->defaultDocumentSize();
    }

    return mNominalSize;
}


void UBGraphicsScene::setNominalSize(const QSize& pSize)
{
    if (nominalSize() != pSize)
    {
        mNominalSize = pSize;
        emit pageSizeChanged();

        if(mDocument)
            mDocument->setDefaultDocumentSize(pSize);

    }
}


void UBGraphicsScene::setNominalSize(int pWidth, int pHeight)
{
     setNominalSize(QSize(pWidth, pHeight));
}


QGraphicsItem* UBGraphicsScene::rootItem(QGraphicsItem* item) const
{
    QGraphicsItem* root = item;

    while (root->parentItem())
    {
        root = root->parentItem();
    }

    return root;
}


void UBGraphicsScene::drawItems (QPainter * painter, int numItems,
        QGraphicsItem * items[], const QStyleOptionGraphicsItem options[], QWidget * widget)
{
    if (mRenderingContext == NonScreen || mRenderingContext == PdfExport)
    {
        int count = 0;

        QGraphicsItem** itemsFiltered = new QGraphicsItem*[numItems];
        QStyleOptionGraphicsItem *optionsFiltered = new QStyleOptionGraphicsItem[numItems];

        for (int i = 0; i < numItems; i++)
        {
            if (!mTools.contains(rootItem(items[i])))
            {
                UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*> (items[i]);
                if(!pdfItem || mRenderingContext == NonScreen)
                {
                    itemsFiltered[count] = items[i];
                    optionsFiltered[count] = options[i];
                    count++;
                }
            }
        }

        QGraphicsScene::drawItems(painter, count, itemsFiltered, optionsFiltered, widget);

        delete[] optionsFiltered;
        delete[] itemsFiltered;

    }
    else if (mRenderingContext == Podcast)
    {
        int count = 0;

        QGraphicsItem** itemsFiltered = new QGraphicsItem*[numItems];
        QStyleOptionGraphicsItem *optionsFiltered = new QStyleOptionGraphicsItem[numItems];

        for (int i = 0; i < numItems; i++)
        {
            bool ok;
            int itemLayerType = items[i]->data(UBGraphicsItemData::ItemLayerType).toInt(&ok);
            if (ok && (itemLayerType >= UBItemLayerType::FixedBackground && itemLayerType <= UBItemLayerType::Tool))
            {
                itemsFiltered[count] = items[i];
                optionsFiltered[count] = options[i];
                count++;
            }
        }

        QGraphicsScene::drawItems(painter, count, itemsFiltered, optionsFiltered, widget);

        delete[] optionsFiltered;
        delete[] itemsFiltered;

    }
    else
    {
        QGraphicsScene::drawItems(painter, numItems, items, options, widget);
    }
}


void UBGraphicsScene::keyReleaseEvent(QKeyEvent * keyEvent)
{

    QList<QGraphicsItem*> si = selectedItems();

    if ((si.size() > 0) && (keyEvent->isAccepted()))
    {
#ifdef Q_OS_MAC
        if (keyEvent->key() == Qt::Key_Backspace)
#else
        if (keyEvent->matches(QKeySequence::Delete))
#endif
        {
            foreach(QGraphicsItem* item, si)
            {
                switch (item->type())
                {
                case UBGraphicsW3CWidgetItem::Type:
                    {
                        UBGraphicsW3CWidgetItem *wc3_widget = dynamic_cast<UBGraphicsW3CWidgetItem*>(item);
                        if (0 != wc3_widget)
                        if (!wc3_widget->hasFocus())
                            wc3_widget->remove();                                                             
                        break;
                    }
                case UBGraphicsAppleWidgetItem::Type:
                    {
                        UBGraphicsAppleWidgetItem *Apple_widget = dynamic_cast<UBGraphicsAppleWidgetItem*>(item);
                        if (0 !=Apple_widget)
                        if (!Apple_widget->hasFocus())
                            Apple_widget->remove();                          
                        break;
                    }
                case UBGraphicsTextItem::Type:
                    {
                        UBGraphicsTextItem *text_item = dynamic_cast<UBGraphicsTextItem*>(item);
                        if (0 != text_item)
                        if (!text_item->hasFocus())
                            text_item->remove();                              
                        break;
                    }

                default:
                    {
                        UBGraphicsItem *ubgi = dynamic_cast<UBGraphicsItem*>(item);
                        if (0 != ubgi)
                            ubgi->remove();
                        else
                            UBCoreGraphicsScene::removeItem(item);      
                    }
                }
            }
        }

        keyEvent->accept();
    }

    QGraphicsScene::keyReleaseEvent(keyEvent);
}


void UBGraphicsScene::setDocumentUpdated()
{
    if (document())
        document()->setMetaData(UBSettings::documentUpdatedAt
                , UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
}


void UBGraphicsScene::setToolCursor(int tool)
{
    if (tool != (int)UBStylusTool::Selector
            && tool != (int)UBStylusTool::Text)
    {
        deselectAllItems();
    }

    if (tool != (int)UBStylusTool::Eraser)
    {
        hideEraser();
    }
}
