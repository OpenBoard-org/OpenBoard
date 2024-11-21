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


#include "UBGraphicsScene.h"

#include <QtGui>
#include <QtSvg>
#include <QGraphicsView>
#include <QGraphicsVideoItem>

#include "frameworks/UBGeometryUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"
#include "core/UBPersistenceManager.h"
#include "core/UBTextTools.h"

#include "gui/UBBackgroundRuling.h"
#include "gui/UBMagnifer.h"
#include "gui/UBMainWindow.h"
#include "gui/UBResources.h"

#include "tools/UBGraphicsRuler.h"
#include "tools/UBGraphicsAxes.h"
#include "tools/UBGraphicsProtractor.h"
#include "tools/UBGraphicsCompass.h"
#include "tools/UBGraphicsTriangle.h"
#include "tools/UBGraphicsCurtainItem.h"
#include "tools/UBGraphicsCache.h"

#include "document/UBDocumentProxy.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardView.h"

#include "UBGraphicsItemUndoCommand.h"
#include "UBGraphicsItemGroupUndoCommand.h"
#include "UBGraphicsTextItemUndoCommand.h"
#include "UBGraphicsPixmapItem.h"
#include "UBGraphicsSvgItem.h"
#include "UBGraphicsPolygonItem.h"
#include "UBGraphicsMediaItem.h"
#include "UBGraphicsWidgetItem.h"
#include "UBGraphicsPDFItem.h"
#include "UBGraphicsTextItem.h"
#include "UBGraphicsStrokesGroup.h"
#include "UBSelectionFrame.h"
#include "UBGraphicsItemZLevelUndoCommand.h"

#include "domain/UBGraphicsGroupContainerItem.h"

#include "UBGraphicsStroke.h"

#include "core/memcheck.h"


#define DEFAULT_Z_VALUE 0.0

qreal UBZLayerController::errorNumber = -20000001.0;

UBZLayerController::UBZLayerController(QGraphicsScene *scene) :
    mScene(scene)

{
    scopeMap.insert(itemLayerType::NoLayer,        ItemLayerTypeData( errorNumber, errorNumber));
    scopeMap.insert(itemLayerType::BackgroundItem, ItemLayerTypeData(-1000000.0, -1000000.0 ));
    // DEFAULT_Z_VALUE isn't used because it allows to easily identify new objects
    scopeMap.insert(itemLayerType::ObjectItem,     ItemLayerTypeData(-1000000.0,  DEFAULT_Z_VALUE - 1.0));
    scopeMap.insert(itemLayerType::DrawingItem,    ItemLayerTypeData( DEFAULT_Z_VALUE + 1.0, 1000000.0 ));
    scopeMap.insert(itemLayerType::ToolItem,       ItemLayerTypeData( 1000000.0,  1000100.0 ));
    scopeMap.insert(itemLayerType::CppTool,        ItemLayerTypeData( 1000100.0,  1000200.0 ));
    scopeMap.insert(itemLayerType::Curtain,        ItemLayerTypeData( 1000200.0,  1001000.0 ));
    scopeMap.insert(itemLayerType::Eraiser,        ItemLayerTypeData( 1001000.0,  1001100.0 ));
    scopeMap.insert(itemLayerType::Pointer,        ItemLayerTypeData( 1001100.0,  1001200.0 ));
    scopeMap.insert(itemLayerType::Cache,          ItemLayerTypeData( 1001300.0,  1001400.0 ));

    scopeMap.insert(itemLayerType::SelectedItem,   ItemLayerTypeData( 1001000.0,  1001000.0 ));
    scopeMap.insert(itemLayerType::SelectionFrame, ItemLayerTypeData( 1010000.0,  1010000.0 ));
}

qreal UBZLayerController::generateZLevel(itemLayerType::Enum key)
{

    if (!scopeMap.contains(key)) {
        qDebug() << "Number is out of layer scope";
        return errorNumber;
    }

    qreal result = scopeMap.value(key).curValue;
    qreal top = scopeMap.value(key).topLimit;
    qreal incrementalStep = scopeMap.value(key).incStep;

    result += incrementalStep;
    if (result >= top) {
        // If not only one variable presents in the scope, notify that values for scope are over
        if (scopeMap.value(key).topLimit != scopeMap.value(key).bottomLimit) {
            qDebug() << "new values are over for the scope" << key;
        }
        result = top - incrementalStep;
    }

    scopeMap[key].curValue = result;

    return result;
}
qreal UBZLayerController::generateZLevel(QGraphicsItem *item)
{
    qreal result = errorNumber;
    itemLayerType::Enum type = static_cast<itemLayerType::Enum>(item->data(UBGraphicsItemData::itemLayerType).toInt());

    if (validLayerType(type)) {
        result =  generateZLevel(type);
    }

    return result;
}

qreal UBZLayerController::changeZLevelTo(QGraphicsItem *item, moveDestination dest)
{
    itemLayerType::Enum curItemLayerType = typeForData(item);
    if (curItemLayerType == itemLayerType::NoLayer) {
        qDebug() << "item's layer is out of the scope. Can't implement z-layer changing operation";
        return errorNum();
    }

    //select only items wiht the same z-level as item's one and push it to sortedItems QMultiMap
    QMultiMap<qreal, QGraphicsItem*> sortedItems;
    if (mScene->items().count()) {
        foreach (QGraphicsItem *tmpItem, mScene->items()) {
            if (typeForData(tmpItem) == curItemLayerType) {
                sortedItems.insert(tmpItem->data(UBGraphicsItemData::ItemOwnZValue).toReal(), tmpItem);
            }
        }
    }

    //If only one item itself - do nothing, return it's z-value
    if (sortedItems.count() == 1 && sortedItems.first() == item) {
        qDebug() << "only one item exists in layer. Have nothing to change";
        return item->data(UBGraphicsItemData::ItemOwnZValue).toReal();
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QMultiMapIterator<qreal, QGraphicsItem*>iCurElement(sortedItems);
#else
    QMapIterator<qreal, QGraphicsItem*>iCurElement(sortedItems);
#endif

    if (dest == up)
    {
        qreal newZValue = item->data(UBGraphicsItemData::ItemOwnZValue).toReal() + 1;
        UBGraphicsItem::assignZValue(item, newZValue);
        shiftStoredZValue(item, newZValue);

        //show a human-readable zValue so the user can adapt his objects zvalues easily
        UBApplication::showMessage(QString::number(1000000 + newZValue));
    }
    else if (dest == top)
    {
        if (iCurElement.findNext(item)) {
            if (iCurElement.hasNext()) {
                UBGraphicsItem::assignZValue(item, generateZLevel(item));
            }
        }
    }
    else if (dest == down)
    {
        qreal newZValue = item->data(UBGraphicsItemData::ItemOwnZValue).toReal()-1;
        if (newZValue >= -999999.0)
        {
            UBGraphicsItem::assignZValue(item, newZValue);
            shiftStoredZValue(item, newZValue);

            //show a human-readable zValue so the user can adapt his objects zvalues easily
            UBApplication::showMessage(QString::number(1000000 + newZValue));
        }
        else
        {
            UBApplication::showMessage(tr("Bottom layer limit reached"));
        }
    }
    else if (dest == bottom)
    {
        iCurElement.toBack();
        if (iCurElement.findPrevious(item)) {
            if (iCurElement.hasPrevious()) {
                qreal oldz = item->data(UBGraphicsItemData::ItemOwnZValue).toReal();
                iCurElement.toFront();
                qreal nextZ = iCurElement.next().value()->data(UBGraphicsItemData::ItemOwnZValue).toReal();

                ItemLayerTypeData curItemLayerTypeData = scopeMap.value(curItemLayerType);

                //if we have some free space between lowest graphics item and layer's bottom bound,
                //insert element close to first element in layer
                if (nextZ > curItemLayerTypeData.bottomLimit + curItemLayerTypeData.incStep) {
                    qreal result = nextZ - curItemLayerTypeData.incStep;
                    UBGraphicsItem::assignZValue(item, result);
                } else {
                    UBGraphicsItem::assignZValue(item, nextZ);

                    bool doubleGap = false; //to detect if we can finish rundown since we can insert item to the free space

                    while (iCurElement.peekNext().value() != item) {
                        qreal curZ = iCurElement.value()->data(UBGraphicsItemData::ItemOwnZValue).toReal();
                        qreal curNextZ = iCurElement.peekNext().value()->data(UBGraphicsItemData::ItemOwnZValue).toReal();
                        if (curNextZ - curZ >= 2 * curItemLayerTypeData.incStep) {
                            UBGraphicsItem::assignZValue(iCurElement.value(), curZ + curItemLayerTypeData.incStep);
                            doubleGap = true;
                            break;
                        } else {
                            UBGraphicsItem::assignZValue(iCurElement.value(), curNextZ);
                            iCurElement.next();
                        }
                    }
                    if (!doubleGap) {

                        UBGraphicsItem::assignZValue(iCurElement.value(), oldz);

                        while (iCurElement.hasNext() && (iCurElement.peekNext().value()->data(UBGraphicsItemData::ItemOwnZValue).toReal() == oldz)) {
                            UBGraphicsItem::assignZValue(iCurElement.next().value(), oldz);
                        }
                    }
                }
            }
        }
    }


    //clear selection of the item and then select it again to activate selectionChangeProcessing()
    item->scene()->clearSelection();
    item->setSelected(true);

    foreach (QGraphicsItem *iitem, sortedItems.values()) {
        if (iitem)
            iitem != item ? qDebug() <<  "current value" << iitem->zValue() : qDebug() << "marked value" << QString::number(iitem->zValue(), 'f');
    }

    //Return new z value assigned to item
    
    // experimental
    item->setZValue(item->data(UBGraphicsItemData::ItemOwnZValue).toReal());

    return item->data(UBGraphicsItemData::ItemOwnZValue).toReal();
}

itemLayerType::Enum UBZLayerController::typeForData(QGraphicsItem *item) const
{
    itemLayerType::Enum result = static_cast<itemLayerType::Enum>(item->data(UBGraphicsItemData::itemLayerType).toInt());

    if (!scopeMap.contains(result)) {
        result = itemLayerType::NoLayer;
    }

    return result;
}

void UBZLayerController::setLayerType(QGraphicsItem *pItem, itemLayerType::Enum pNewType)
{
   pItem->setData(UBGraphicsItemData::itemLayerType, QVariant(pNewType));
}

void UBZLayerController::shiftStoredZValue(QGraphicsItem *item, qreal zValue)
{
    itemLayerType::Enum type = typeForData(item);

    if (validLayerType(type)) {
        ItemLayerTypeData typeData = scopeMap.value(type);
        if (typeData.curValue < zValue) {
            scopeMap[type].curValue = zValue;
        }
    }
}

/**
 * @brief Returns true if the zLevel is not used by any item on the scene, or false if so.
 */
bool UBZLayerController::zLevelAvailable(QGraphicsItem* item)
{
    foreach(QGraphicsItem* it, mScene->items())
    {
        if (item != it)
        {
            if (it->zValue() == item->zValue())
                return false;
        }
    }

    return true;
}

UBGraphicsScene::UBGraphicsScene(std::shared_ptr<UBDocumentProxy> document, bool enableUndoRedoStack)
    : mEraser(0)
    , mPointer(0)
    , mMarkerCircle(0)
    , mPenCircle(0)
    , mDocument(document)
    , mDarkBackground(false)
    , mIsDesktopMode(false)
    , mZoomFactor(1)
    , mBackgroundObject(0)
    , mPreviousWidth(0)
    , mDistanceFromLastStrokePoint(0)
    , mInputDeviceIsPressed(false)
    , mArcPolygonItem(0)
    , mRenderingContext(Screen)
    , mCurrentStroke(0)
    , mItemCount(0)
    , mUndoRedoStackEnabled(enableUndoRedoStack)
    , magniferControlViewWidget(0)
    , magniferDisplayViewWidget(0)
    , mZLayerController(new UBZLayerController(this))
    , mpLastPolygon(NULL)
    , mTempPolygon(NULL)
    , mDrawWithCompass(false)
    , mCurrentPolygon(0)
    , mSelectionFrame(0)
    , mGraphicsCache(nullptr)
{
    UBCoreGraphicsScene::setObjectName("BoardScene");
    setItemIndexMethod(BspTreeIndex);

    setUuid(QUuid::createUuid());
    setDocument(document);
    createEraiser();
    createPointer();
    createMarkerCircle();
    createPenCircle();

    if (UBApplication::applicationController)
    {
        setViewState(SceneViewState(1,
            UBApplication::applicationController->initialHScroll(),
            UBApplication::applicationController->initialVScroll()));
    }

    mBackgroundGridSize = UBSettings::settings()->crossSize;

//    Just for debug. Do not delete please
//    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChangedProcessing()));
    connect(UBApplication::undoStack.data(), SIGNAL(indexChanged(int)), this, SLOT(updateSelectionFrameWrapper(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(stylusToolChanged(int,int)), this, SLOT(stylusToolChanged(int,int)));
    connect(UBApplication::boardController, &UBBoardController::zoomChanged, this, &UBGraphicsScene::zoomChanged);
}

UBGraphicsScene::~UBGraphicsScene()
{
    // disconnect all consumers of this signal at once to speed-up deletion of scene
    disconnect(this, &UBGraphicsScene::zoomChanged, nullptr, nullptr);

    if (mCurrentStroke && mCurrentStroke->polygons().empty()){
        delete mCurrentStroke;
        mCurrentStroke = NULL;
    }

    if (mZLayerController)
        delete mZLayerController;

    if (mGraphicsCache)
    {
        delete mGraphicsCache;
    }
}

void UBGraphicsScene::selectionChangedProcessing()
{
    if (selectedItems().count()){
        UBApplication::showMessage("ZValue is " + QString::number(selectedItems().constFirst()->zValue(), 'f') + "own z value is "
                                   + QString::number(selectedItems().constFirst()->data(UBGraphicsItemData::ItemOwnZValue).toReal(), 'f'));

    }
}

void UBGraphicsScene::setLastCenter(QPointF center)
{
    mViewState.setLastSceneCenter(center);
}

QPointF UBGraphicsScene::lastCenter()
{
    return mViewState.lastSceneCenter();
}

bool UBGraphicsScene::inputDevicePress(const QPointF& scenePos, const qreal& pressure, Qt::KeyboardModifiers modifiers)
{
    bool accepted = false;

    if (mInputDeviceIsPressed) {
        qWarning() << "scene received input device pressed, without input device release, muting event as input device move";
        accepted = inputDeviceMove(scenePos, pressure, modifiers);
    }
    else {
        mInputDeviceIsPressed = true;

        UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController()->stylusTool();

        if (UBDrawingController::drawingController()->isDrawingTool()) {
            // -----------------------------------------------------------------
            // We fall here if we are using the Pen, the Marker or the Line tool
            // -----------------------------------------------------------------
            qreal width = 0;

            // delete current stroke, if not assigned to any polygon
            if (mCurrentStroke && mCurrentStroke->polygons().empty()){
                delete mCurrentStroke;
                mCurrentStroke = NULL;
            }

            // hide the marker preview circle
            if (currentTool == UBStylusTool::Marker)
                hideMarkerCircle();

            // hide the pen preview circle
            if (currentTool == UBStylusTool::Pen)
                hidePenCircle();

            // ---------------------------------------------------------------
            // Create a new Stroke. A Stroke is a collection of QGraphicsLines
            // ---------------------------------------------------------------
            mCurrentStroke = new UBGraphicsStroke(shared_from_this());

            if (currentTool != UBStylusTool::Line){
                // Handle the pressure
                width = UBDrawingController::drawingController()->currentToolWidth() * pressure;
            }
            else{
                // Ignore pressure for the line tool
                width = UBDrawingController::drawingController()->currentToolWidth();
            }

            width /= UBApplication::boardController->systemScaleFactor();
            width /= UBApplication::boardController->currentZoom();

            mAddedItems.clear();
            mRemovedItems.clear();

            if (UBDrawingController::drawingController()->activeRuler())
                UBDrawingController::drawingController()->activeRuler()->StartLine(scenePos, width);
            else {
                bool isLine = UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Line;
                QPointF pos = scenePos;

                if (isLine && isSnapping())
                {
                    pos += snap(scenePos);
                }

                moveTo(pos);
                drawLineTo(pos, width, isLine);

                mCurrentStroke->addPoint(pos, width);
            }
            accepted = true;
        }
        else if (currentTool == UBStylusTool::Eraser) {
            mAddedItems.clear();
            mRemovedItems.clear();
            moveTo(scenePos);

            qreal eraserWidth = UBSettings::settings()->currentEraserWidth();
            eraserWidth /= UBApplication::boardController->systemScaleFactor();
            eraserWidth /= UBApplication::boardController->currentZoom();

            eraseLineTo(scenePos, eraserWidth);
            drawEraser(scenePos, mInputDeviceIsPressed);

            accepted = true;
        }
        else if (currentTool == UBStylusTool::Pointer) {
            drawPointer(scenePos, true);
            accepted = true;
        }
    }

    if (mCurrentStroke && mCurrentStroke->polygons().empty()){
        delete mCurrentStroke;
        mCurrentStroke = NULL;
    }

    return accepted;
}

bool UBGraphicsScene::inputDeviceMove(const QPointF& scenePos, const qreal& pressure, Qt::KeyboardModifiers modifiers)
{
    bool accepted = false;

    UBDrawingController *dc = UBDrawingController::drawingController();
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)dc->stylusTool();

    QPointF position = QPointF(scenePos);
    mCurrentPoint = position;

    if (currentTool == UBStylusTool::Eraser)
    {
        drawEraser(position, mInputDeviceIsPressed);
        accepted = true;
    }

    else if (currentTool == UBStylusTool::Marker) {
        if (mInputDeviceIsPressed)
            hideMarkerCircle();
        else {
            drawMarkerCircle(position);
            accepted = true;
        }
    }

    else if (currentTool == UBStylusTool::Pen) {
        if (mInputDeviceIsPressed)
            hidePenCircle();
        else {
            drawPenCircle(position);
            accepted = true;
        }
    }

    if (mInputDeviceIsPressed)
    {
        if (dc->isDrawingTool())
        {
            qreal width = 0;

            if (currentTool != UBStylusTool::Line){
                // Handle the pressure
                width = dc->currentToolWidth() * qMax(pressure, 0.2);
            }else{
                // Ignore pressure for line tool
                width = dc->currentToolWidth();
            }

            width /= UBApplication::boardController->systemScaleFactor();
            width /= UBApplication::boardController->currentZoom();

            std::optional<QPointF> altPosition;

            if (currentTool == UBStylusTool::Line || dc->activeRuler())
            {
                if (UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Marker)
                if(NULL != mpLastPolygon && NULL != mCurrentStroke && mAddedItems.size() > 0){
                    UBCoreGraphicsScene::removeItemFromDeletion(mpLastPolygon);
                    mAddedItems.remove(mpLastPolygon);
                    mCurrentStroke->remove(mpLastPolygon);
                    if (mCurrentStroke->polygons().empty()){
                        delete mCurrentStroke;
                        mCurrentStroke = NULL;
                    }
                    removeItem(mpLastPolygon);
                    mPreviousPolygonItems.removeAll(mpLastPolygon);
                }

                // ------------------------------------------------------------------------
                // Here we wanna make sure that the Line will 'grip' at multiples of
                // rotationAngleStep and propose a point as alternative snap point
                // ------------------------------------------------------------------------

                if (isSnapping())
                {
                    double step = UBSettings::settings()->rotationAngleStep->get().toDouble();
                    QLineF radius(mPreviousPoint, position);
                    qreal angle = radius.angle();
                    angle = qRound(angle / step) * step;
                    radius.setAngle(angle);
                    altPosition = radius.p2();
                }
            }

            if (!mCurrentStroke)
                mCurrentStroke = new UBGraphicsStroke(shared_from_this());

            if(dc->activeRuler()){
                dc->activeRuler()->DrawLine(position, width);
            }

            else if (currentTool == UBStylusTool::Line) {
                if (isSnapping())
                {
                    // compute two proposals and compare the angle
                    QPointF gridSnapPoint;
                    position += snap(position, nullptr, altPosition, &gridSnapPoint);

                    if (position != gridSnapPoint)
                    {
                        const auto angle1 = QLineF{mPreviousPoint, position}.angle();
                        const auto angle2 = QLineF{mPreviousPoint, gridSnapPoint}.angle();

                        if (std::fmod(std::fabs(angle1 - angle2), 360.) < 0.01)
                        {
                            position = gridSnapPoint;
                        }
                    }
                }

                QLineF radius(mPreviousPoint, position);
                auto angle = radius.angle();
                QLineF viewRadius{UBApplication::boardController->controlView()->mapFromScene(radius.p1()),
                        UBApplication::boardController->controlView()->mapFromScene(radius.p2())};
                QPoint offset = - viewRadius.p2().toPoint();
                viewRadius.setLength(viewRadius.length() + 30);
                offset += viewRadius.p2().toPoint();
                UBApplication::boardController->setCursorFromAngle(angle, offset);

                drawLineTo(position, width, true);
            }

            else {
                bool interpolate = false;

                if ((currentTool == UBStylusTool::Pen && UBSettings::settings()->boardInterpolatePenStrokes->get().toBool())
                    || (currentTool == UBStylusTool::Marker && UBSettings::settings()->boardInterpolateMarkerStrokes->get().toBool()))
                {
                    interpolate = true;
                }


                // Don't draw segments smaller than a certain length. This can help with performance
                // (less polygons to draw) but mostly with making the curve look smooth.

                qreal antiScaleRatio = 1./(UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
                qreal MIN_DISTANCE = 10*antiScaleRatio; // arbitrary. Move to settings if relevant.
                qreal distance = QLineF(mPreviousPoint, scenePos).length();

                mDistanceFromLastStrokePoint += distance;

                if (mDistanceFromLastStrokePoint > MIN_DISTANCE) {
                    QList<QPair<QPointF, qreal> > newPoints = mCurrentStroke->addPoint(scenePos, width, interpolate);
                    if (newPoints.length() > 1)
                        drawCurve(newPoints);

                    mDistanceFromLastStrokePoint = 0;
                }

                if (interpolate) {
                    // Bezier curves aren't drawn all the way to the scenePos (they stop halfway between the previous and
                    // current scenePos), so we add a line from the last drawn position in the stroke and the
                    // scenePos, to make the drawing feel more responsive. This line is then deleted if a new segment is
                    // added to the stroke. (Or it is added to the stroke when we stop drawing)

                    if (mTempPolygon) {
                        removeItem(mTempPolygon);
                        mTempPolygon = NULL;
                    }

                    if (!mCurrentStroke->points().empty())
                    {
                        QPointF lastDrawnPoint = mCurrentStroke->points().last().first;

                        mTempPolygon = lineToPolygonItem(QLineF(lastDrawnPoint, scenePos), mPreviousWidth, width);
                        addItem(mTempPolygon);
                    }
                }
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

bool UBGraphicsScene::inputDeviceRelease(int tool, Qt::KeyboardModifiers modifiers)
{
    bool accepted = false;

    if (mPointer)
    {
        mPointer->hide();
        accepted = true;
    }

    if (tool < 0)
    {
        tool = UBDrawingController::drawingController()->stylusTool();
    }

    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)tool;
    UBDrawingController *dc = UBDrawingController::drawingController();

    if (dc->isDrawingTool(tool) || mDrawWithCompass)
    {
        if(mArcPolygonItem){

            UBGraphicsStrokesGroup* pStrokes = new UBGraphicsStrokesGroup();

            // Add the arc
            mAddedItems.remove(mArcPolygonItem);
            removeItem(mArcPolygonItem);
            UBCoreGraphicsScene::removeItemFromDeletion(mArcPolygonItem);
            mArcPolygonItem->setStrokesGroup(pStrokes);
            pStrokes->addToGroup(mArcPolygonItem);

            // Add the center cross
            foreach(QGraphicsItem* item, mAddedItems){
                mAddedItems.remove(item);
                removeItem(item);
                UBCoreGraphicsScene::removeItemFromDeletion(item);
                UBGraphicsPolygonItem* pi = qgraphicsitem_cast<UBGraphicsPolygonItem*>(item);
                if (pi)
                    pi->setStrokesGroup(pStrokes);
                pStrokes->addToGroup(item);
            }

            mAddedItems.clear();
            mAddedItems << pStrokes;
            addItem(pStrokes);

            mDrawWithCompass = false;
        }
        else if (mCurrentStroke){
            if (mTempPolygon) {
                UBGraphicsPolygonItem * poly = dynamic_cast<UBGraphicsPolygonItem*>(mTempPolygon->deepCopy());
                removeItem(mTempPolygon);
                mTempPolygon = NULL;
                addPolygonItemToCurrentStroke(poly);
            }

            // replace the stroke by a simplified version of it
            if ((currentTool == UBStylusTool::Pen && UBSettings::settings()->boardSimplifyPenStrokes->get().toBool())
                || (currentTool == UBStylusTool::Marker && UBSettings::settings()->boardSimplifyMarkerStrokes->get().toBool()))
            {
                simplifyCurrentStroke();
            }


            UBGraphicsStrokesGroup* pStrokes = new UBGraphicsStrokesGroup();

            // Remove the strokes that were just drawn here and replace them by a stroke item
            foreach(UBGraphicsPolygonItem* poly, mCurrentStroke->polygons()){
                mPreviousPolygonItems.removeAll(poly);
                removeItem(poly);
                UBCoreGraphicsScene::removeItemFromDeletion(poly);
                poly->setStrokesGroup(pStrokes);
                pStrokes->addToGroup(poly);
            }

            // TODO LATER : Generate well pressure-interpolated polygons and create the line group with them

            mAddedItems.clear();
            mAddedItems << pStrokes;
            addItem(pStrokes);

            if (mCurrentStroke->polygons().empty()){
                delete mCurrentStroke;
                mCurrentStroke = 0;
            }
            mCurrentPolygon = 0;
        }
    }

    if (mRemovedItems.size() > 0 || mAddedItems.size() > 0)
    {
        if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
            if (UBApplication::undoStack)
            {
                UBGraphicsItemUndoCommand* udcmd = new UBGraphicsItemUndoCommand(shared_from_this(), mRemovedItems, mAddedItems); //deleted by the undoStack
                UBApplication::undoStack->push(udcmd);
            }
        }

        mRemovedItems.clear();
        mAddedItems.clear();
        accepted = true;
    }

    mInputDeviceIsPressed = false;

    setDocumentUpdated();

    if (mCurrentStroke && mCurrentStroke->polygons().empty()){
        delete mCurrentStroke;
    }

    mCurrentStroke = NULL;
    return accepted;
}

void UBGraphicsScene::drawEraser(const QPointF &pPoint, bool pressed)
{
    if (mEraser) {
        qreal eraserWidth = UBSettings::settings()->currentEraserWidth();
        eraserWidth /= UBApplication::boardController->systemScaleFactor();
        eraserWidth /= UBApplication::boardController->currentZoom();

        qreal eraserRadius = eraserWidth / 2;

    // TODO UB 4.x optimize - no need to do that every time we move it
        mEraser->setRect(QRectF(pPoint.x() - eraserRadius, pPoint.y() - eraserRadius, eraserWidth, eraserWidth));
        redrawEraser(pressed);
    }
}

void UBGraphicsScene::redrawEraser(bool pressed)
{
    if (mEraser) {
        QPen pen = mEraser->pen();

        if(pressed)
            pen.setStyle(Qt::SolidLine);
        else
            pen.setStyle(Qt::DotLine);

        mEraser->setPen(pen);
        mEraser->show();
    }
}

void UBGraphicsScene::hideEraser()
{
    if (mEraser)
        mEraser->hide();
}

void UBGraphicsScene::drawPointer(const QPointF &pPoint, bool isFirstDraw)
{
    qreal pointerDiameter = UBSettings::pointerDiameter / UBApplication::boardController->currentZoom();
    pointerDiameter /= UBApplication::boardController->systemScaleFactor();
    qreal pointerRadius = pointerDiameter / 2;

    // TODO UB 4.x optimize - no need to do that every time we move it
    if (mPointer) {
        mPointer->setRect(QRectF(pPoint.x() - pointerRadius,
                                 pPoint.y() - pointerRadius,
                                 pointerDiameter,
                                 pointerDiameter));
        if(isFirstDraw) {
            mPointer->show();
        }
    }
}

void UBGraphicsScene::drawMarkerCircle(const QPointF &pPoint)
{
    if (mMarkerCircle) {
        qreal markerDiameter = UBSettings::settings()->currentMarkerWidth();
        markerDiameter /= UBApplication::boardController->systemScaleFactor();
        markerDiameter /= UBApplication::boardController->currentZoom();
        qreal markerRadius = markerDiameter/2;

        mMarkerCircle->setRect(QRectF(pPoint.x() - markerRadius, pPoint.y() - markerRadius,
                                      markerDiameter, markerDiameter));
        mMarkerCircle->show();
    }

}

void UBGraphicsScene::drawPenCircle(const QPointF &pPoint)
{
    QCursor cursor;

    if (mPenCircle && UBSettings::settings()->showPenPreviewCircle->get().toBool() &&
        UBSettings::settings()->currentPenWidth() >= UBSettings::settings()->penPreviewFromSize->get().toInt()) {
        qreal penDiameter = UBSettings::settings()->currentPenWidth();
        penDiameter /= UBApplication::boardController->systemScaleFactor();
        penDiameter /= UBApplication::boardController->currentZoom();
        qreal penRadius = penDiameter/2;

        mPenCircle->setRect(QRectF(pPoint.x() - penRadius, pPoint.y() - penRadius,
                                      penDiameter, penDiameter));
        mPenCircle->show();
        cursor = Qt::BlankCursor;
    }
    else
    {
        cursor = UBResources::resources()->penCursor;
    }

    if (!UBDrawingController::drawingController()->activeRuler())
    {
        // set cursor only if no active ruler
        if (controlView() && controlView()->viewport())
            controlView()->viewport()->setCursor(cursor);
    }
}

void UBGraphicsScene::hideMarkerCircle()
{
    if (mMarkerCircle) {
        mMarkerCircle->hide();
    }
}

void UBGraphicsScene::hidePenCircle()
{
    if (mPenCircle)
        mPenCircle->hide();
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
    // some time have crash here on access to app (when call from destructor when close OpenBoard app)
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
    mDrawWithCompass = false;
}
void UBGraphicsScene::drawLineTo(const QPointF &pEndPoint, const qreal &pWidth, bool bLineStyle)
{
    drawLineTo(pEndPoint, pWidth, pWidth, bLineStyle);

}

void UBGraphicsScene::drawLineTo(const QPointF &pEndPoint, const qreal &startWidth, const qreal &endWidth, bool bLineStyle)
{
    if (mPreviousWidth == -1.0)
        mPreviousWidth = startWidth;

    qreal initialWidth = startWidth;
    if (initialWidth == endWidth)
        initialWidth = mPreviousWidth;

    if (bLineStyle) {
        QSetIterator<QGraphicsItem*> itItems(mAddedItems);

        while (itItems.hasNext()) {
            QGraphicsItem* item = itItems.next();
            removeItem(item);
        }
        mAddedItems.clear();
    }

    UBGraphicsPolygonItem *polygonItem = lineToPolygonItem(QLineF(mPreviousPoint, pEndPoint), initialWidth, endWidth);
    addPolygonItemToCurrentStroke(polygonItem);

    if (!bLineStyle) {
        mPreviousPoint = pEndPoint;
        mPreviousWidth = endWidth;
    }
}

void UBGraphicsScene::drawCurve(const QList<QPair<QPointF, qreal> >& points)
{
    UBGraphicsPolygonItem* polygonItem = curveToPolygonItem(points);
    addPolygonItemToCurrentStroke(polygonItem);

    mPreviousPoint = points.last().first;
    mPreviousWidth = points.last().second;
}

void UBGraphicsScene::drawCurve(const QList<QPointF>& points, qreal startWidth, qreal endWidth)
{
    UBGraphicsPolygonItem* polygonItem = curveToPolygonItem(points, startWidth, endWidth);
    addPolygonItemToCurrentStroke(polygonItem);

    mPreviousWidth = endWidth;
    mPreviousPoint = points.last();
}

void UBGraphicsScene::addPolygonItemToCurrentStroke(UBGraphicsPolygonItem* polygonItem)
{
    if (!polygonItem->brush().isOpaque())
    {
        // -------------------------------------------------------------------------------------
        // Here we substract the polygons that are overlapping in order to keep the transparency
        // -------------------------------------------------------------------------------------
        for (int i = 0; i < mPreviousPolygonItems.size(); i++)
        {
            UBGraphicsPolygonItem* previous = mPreviousPolygonItems.value(i);
            polygonItem->subtract(previous);
        }
    }

    mpLastPolygon = polygonItem;
    mAddedItems.insert(polygonItem);

    // Here we add the item to the scene
    addItem(polygonItem);
    if (!mCurrentStroke)
        mCurrentStroke = new UBGraphicsStroke(shared_from_this());

    polygonItem->setStroke(mCurrentStroke);

    mPreviousPolygonItems.append(polygonItem);

}

void UBGraphicsScene::eraseLineTo(const QPointF &pEndPoint, const qreal &pWidth)
{
    const QLineF line(mPreviousPoint, pEndPoint);
    mPreviousPoint = pEndPoint;

    const QPolygonF eraserPolygon = UBGeometryUtils::lineToPolygon(line, pWidth);
    const QRectF eraserBoundingRect = eraserPolygon.boundingRect();

    QPainterPath eraserPath;
    eraserPath.addPolygon(eraserPolygon);

    // Get all the items that are intersecting with the eraser path, except the eraser itself
    QList<QGraphicsItem*> collidItems = items(eraserBoundingRect, Qt::IntersectsItemBoundingRect);
    collidItems.removeOne(mEraser);

    QList<UBGraphicsPolygonItem*> intersectedItems;

    typedef QList<QPolygonF> POLYGONSLIST;
    QList<POLYGONSLIST> intersectedPolygons;

    for(int i=0; i<collidItems.size(); i++)
    {
        UBGraphicsPolygonItem *pi = qgraphicsitem_cast<UBGraphicsPolygonItem*>(collidItems[i]);
        if(pi == NULL)
            continue;

        QPainterPath itemPainterPath;
        itemPainterPath.addPolygon(pi->sceneTransform().map(pi->polygon()));

        if (eraserPath.contains(itemPainterPath))
        {
            // Completely remove item
            intersectedItems << pi;
            intersectedPolygons << QList<QPolygonF>();
        }
        else if (eraserPath.intersects(itemPainterPath))
        {
            itemPainterPath.setFillRule(Qt::WindingFill);
            // reverse eraserPath so that it has the opposite orientation of the stroke
            // necessary for punching a hole with WindingFill rule
            QPainterPath newPath = itemPainterPath.subtracted(eraserPath.toReversed());
            intersectedItems << pi;
            intersectedPolygons << newPath.simplified().toFillPolygons(pi->sceneTransform().inverted());
        }
    }

    for(int i=0; i<intersectedItems.size(); i++)
    {
        // item which intersects with eraser
        UBGraphicsPolygonItem *intersectedPolygonItem = intersectedItems[i];

        if (!intersectedPolygons[i].empty())
        {
            // intersected polygons generated as QList<QPolygon> QPainterPath::toFillPolygons(),
            // so each intersectedPolygonItem has one or couple of QPolygons who should be removed from it.
            for(int j = 0; j < intersectedPolygons[i].size(); j++)
            {
                // create small polygon from couple of polygons to replace particular erased polygon
                UBGraphicsPolygonItem* polygonItem = new UBGraphicsPolygonItem(intersectedPolygons[i][j], intersectedPolygonItem->parentItem());

                intersectedPolygonItem->copyItemParameters(polygonItem);
                polygonItem->setNominalLine(false);
                polygonItem->setStroke(intersectedPolygonItem->stroke());
                if (intersectedPolygonItem->strokesGroup())
                {
                    polygonItem->setStrokesGroup(intersectedPolygonItem->strokesGroup());
                    intersectedPolygonItem->strokesGroup()->addToGroup(polygonItem);
                }
                mAddedItems << polygonItem;
            }
        }

        //remove full polygon item and replace it by a couple of polygons which create the same stroke without the part that intersects with the eraser
         mRemovedItems << intersectedPolygonItem;

        QTransform t;
        bool bApplyTransform = false;
        if (intersectedPolygonItem->strokesGroup())
        {
            if (intersectedPolygonItem->strokesGroup()->parentItem())
            {
                bApplyTransform = true;
                t = intersectedPolygonItem->sceneTransform();
            }
            intersectedPolygonItem->strokesGroup()->removeFromGroup(intersectedPolygonItem);
        }
        removeItem(intersectedPolygonItem);
        if (bApplyTransform)
            intersectedPolygonItem->setTransform(t);
    }

    if (!intersectedItems.empty())
        setModified(true);
}

void UBGraphicsScene::drawArcTo(const QPointF& pCenterPoint, qreal pSpanAngle)
{
    mDrawWithCompass = true;
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
    mArcPolygonItem->setFillRule(Qt::WindingFill);
    mArcPolygonItem->setStroke(mCurrentStroke);
    mAddedItems.insert(mArcPolygonItem);
    addItem(mArcPolygonItem);

    setDocumentUpdated();
}

void UBGraphicsScene::setSceneBackground(bool pIsDark, const UBBackgroundRuling *background)
{
    bool needRepaint = false;

    if (mDarkBackground != pIsDark)
    {
        mDarkBackground = pIsDark;

        updateEraserColor();
        updateMarkerCircleColor();
        updatePenCircleColor();
        recolorAllItems();

        needRepaint = true;
    }

    if (mBackground != background)
    {
        mBackground = background;
        needRepaint = true;
    }

    if (needRepaint)
    {
        updateBackground();
    }
}

void UBGraphicsScene::setBackgroundZoomFactor(qreal zoom)
{
    mZoomFactor = zoom;
}


void UBGraphicsScene::setBackgroundGridSize(int pSize)
{
    if (pSize > 0) {
        mBackgroundGridSize = pSize;
        updateBackground();
    }
}

void UBGraphicsScene::setDrawingMode(bool bModeDesktop)
{
    mIsDesktopMode = bModeDesktop;
}

void UBGraphicsScene::recolorAllItems()
{
    QHash<QGraphicsView*, QGraphicsView::ViewportUpdateMode> previousUpdateModes;
    foreach(QGraphicsView* view, views())
    {
        previousUpdateModes.insert(view, view->viewportUpdateMode());
        view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    }

    bool currentIslight = isLightBackground();
    foreach (QGraphicsItem *item, items()) {
        if (item->type() == UBGraphicsStrokesGroup::Type) {
            UBGraphicsStrokesGroup *curGroup = static_cast<UBGraphicsStrokesGroup*>(item);
            QColor compareColor =  curGroup->color(currentIslight ? UBGraphicsStrokesGroup::colorOnDarkBackground
                                                                  : UBGraphicsStrokesGroup::colorOnLightBackground);

            if (curGroup->color() == compareColor) {
                QColor newColor = curGroup->color(!currentIslight ? UBGraphicsStrokesGroup::colorOnDarkBackground
                                                                  : UBGraphicsStrokesGroup::colorOnLightBackground);
                curGroup->setColor(newColor);
            }
        }

        if (item->type() == UBGraphicsTextItem::Type)
        {
            UBGraphicsTextItem *textItem = static_cast<UBGraphicsTextItem*>(item);
            textItem->recolor();
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


UBGraphicsPolygonItem* UBGraphicsScene::lineToPolygonItem(const QLineF &pLine, const qreal &pStartWidth, const qreal &pEndWidth)
{
    UBGraphicsPolygonItem *polygonItem = new UBGraphicsPolygonItem(pLine, pStartWidth, pEndWidth);

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

    //polygonItem->setColor(QColor(rand()%256, rand()%256, rand()%256, polygonItem->brush().color().alpha()));

    polygonItem->setColorOnDarkBackground(colorOnDarkBG);
    polygonItem->setColorOnLightBackground(colorOnLightBG);

    polygonItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Graphic));
}

UBGraphicsPolygonItem* UBGraphicsScene::arcToPolygonItem(const QLineF& pStartRadius, qreal pSpanAngle, qreal pWidth)
{
    QPolygonF polygon = UBGeometryUtils::arcToPolygon(pStartRadius, pSpanAngle, pWidth);

    return polygonToPolygonItem(polygon);
}

UBGraphicsPolygonItem* UBGraphicsScene::curveToPolygonItem(const QList<QPair<QPointF, qreal> >& points)
{
    QPolygonF polygon = UBGeometryUtils::curveToPolygon(points, false, true);

    return polygonToPolygonItem(polygon);

}

UBGraphicsPolygonItem* UBGraphicsScene::curveToPolygonItem(const QList<QPointF>& points, qreal startWidth, qreal endWidth)
{
    QPolygonF polygon = UBGeometryUtils::curveToPolygon(points, startWidth, endWidth);

    return polygonToPolygonItem(polygon);
}

void UBGraphicsScene::clearSelectionFrame()
{
    if (mSelectionFrame) {
        mSelectionFrame->setEnclosedItems(QList<QGraphicsItem*>());
    }
}

UBBoardView *UBGraphicsScene::controlView()
{
    UBBoardView *result = 0;
    foreach (QGraphicsView *view, views()) {
        if (view->objectName() == CONTROLVIEW_OBJ_NAME) {
            result = static_cast<UBBoardView*>(view);
        }
    }

    return result;
}

void UBGraphicsScene::notifyZChanged(QGraphicsItem *item, qreal zValue)
{
    mZLayerController->shiftStoredZValue(item, zValue);
}

void UBGraphicsScene::updateSelectionFrame()
{
    if (!mSelectionFrame) {
        mSelectionFrame = new UBSelectionFrame();
        bool sceneWasModified = isModified();
        addItem(mSelectionFrame);
        setModified(sceneWasModified);
    }

    QList<QGraphicsItem*> selItems = selectedItems();
    switch (selItems.count()) {
    case 0 : {
        mSelectionFrame->setVisible(false);
        mSelectionFrame->setEnclosedItems(selItems);
    } break;
    case 1: {
        mSelectionFrame->setVisible(false);
        mSelectionFrame->setEnclosedItems(QList<QGraphicsItem*>());

        UBGraphicsItemDelegate *itemDelegate = UBGraphicsItem::Delegate(selItems.first());
        if (itemDelegate)
        {
            itemDelegate->createControls();
            selItems.first()->setVisible(true);
            itemDelegate->showControls();
        }

    } break;
    default: {
        mSelectionFrame->setVisible(true);
        mSelectionFrame->setEnclosedItems(selItems);
    } break;
    }
}

void UBGraphicsScene::updateSelectionFrameWrapper(int)
{
    updateSelectionFrame();
}

UBGraphicsPolygonItem* UBGraphicsScene::polygonToPolygonItem(const QPolygonF pPolygon)
{
    UBGraphicsPolygonItem *polygonItem = new UBGraphicsPolygonItem(pPolygon);

    initPolygonItem(polygonItem);

    return polygonItem;
}

void UBGraphicsScene::hideTool()
{
    hideEraser();
    hideMarkerCircle();
    hidePenCircle();
}

void UBGraphicsScene::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    hideTool();
}

std::shared_ptr<UBGraphicsScene> UBGraphicsScene::sceneDeepCopy() const
{
    std::shared_ptr<UBGraphicsScene> copy = std::make_shared<UBGraphicsScene>(this->document(), this->mUndoRedoStackEnabled);

    copy->setBackgroundGridSize(mBackgroundGridSize);
    copy->mDarkBackground = mDarkBackground;
    copy->mBackground = mBackground;
    copy->setSceneRect(this->sceneRect());

    if (this->mNominalSize.isValid())
        copy->setNominalSize(this->mNominalSize);

    foreach (auto item, items())
    {
        UBItem* ubItem = dynamic_cast<UBItem*>(item);

        // copy visible top-level items
        if (ubItem && item->isVisible() && !item->parentItem())
        {
            QGraphicsItem* cloneItem = nullptr;
            UBGraphicsGroupContainerItem* group = dynamic_cast<UBGraphicsGroupContainerItem*>(item);

            if (group)
            {
                UBGraphicsGroupContainerItem* groupCloned = group->deepCopyNoChildDuplication();
                groupCloned->resetTransform();
                groupCloned->setPos(0, 0);

                foreach (QGraphicsItem* childItem, group->childItems())
                {
                    UBItem* childUBItem = dynamic_cast<UBItem*>(childItem);
                    if (childUBItem)
                    {
                        UBItem* childUBItemCopy = childUBItem->deepCopy();
                        QGraphicsItem* copiedChild = dynamic_cast<QGraphicsItem*>(childUBItemCopy);
                        groupCloned->addToGroup(copiedChild);
                    }
                }

                bool locked = group->Delegate()->isLocked();

                if (locked)
                    groupCloned->setData(UBGraphicsItemData::ItemLocked, QVariant(true));

                groupCloned->setData(UBGraphicsItemData::ItemIsHiddenOnDisplay, QVariant(group->data(UBGraphicsItemData::ItemIsHiddenOnDisplay)));

                groupCloned->setTransform(QTransform::fromTranslate(group->pos().x(), group->pos().y()));
                groupCloned->setTransform(group->transform(), true);
                cloneItem = groupCloned;
            }
            else
            {
                cloneItem = dynamic_cast<QGraphicsItem*>(ubItem->deepCopy());
            }

            if (cloneItem)
            {
                copy->addItem(cloneItem);

                if (isBackgroundObject(item))
                    copy->setAsBackgroundObject(cloneItem);

                if (this->mTools.contains(item))
                    copy->mTools << cloneItem;
            }
        }
    }

    // TODO UB 4.7 ... complete all members ?

    return copy;
}

UBItem* UBGraphicsScene::deepCopy() const
{
    qWarning() << "UBGraphicsScene::deepCopy must not be called";
    return nullptr;
}

void UBGraphicsScene::clearContent(clearCase pCase)
{
    QSet<QGraphicsItem*> removedItems;
    UBGraphicsItemUndoCommand::GroupDataTable groupsMap;

    switch (pCase) {
    case clearBackground :
        if(mBackgroundObject){
            removeItem(mBackgroundObject);
            removedItems << mBackgroundObject;
            mBackgroundObject = nullptr;
        }
        break;

    case clearItemsAndAnnotations :
    case clearItems :
    case clearAnnotations :
        foreach(QGraphicsItem* item, items()) {

            UBGraphicsGroupContainerItem *itemGroup = item->parentItem()
                                                      ? qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(item->parentItem())
                                                      : 0;
            UBGraphicsItemDelegate *curDelegate = UBGraphicsItem::Delegate(item);
            if (!curDelegate) {
                continue;
            }

            bool isGroup = item->type() == UBGraphicsGroupContainerItem::Type;
            bool isStrokesGroup = item->type() == UBGraphicsStrokesGroup::Type;

            bool shouldDelete = false;
            switch (static_cast<int>(pCase)) {
            case clearAnnotations :
                shouldDelete = isStrokesGroup;
                break;
            case clearItems :
                shouldDelete = !isGroup && !isBackgroundObject(item) && !isStrokesGroup;
                break;
            case clearItemsAndAnnotations:
                shouldDelete = !isGroup && !isBackgroundObject(item);
                break;
            }

            if(shouldDelete) {
                if (itemGroup) {
                    itemGroup->removeFromGroup(item);

                    groupsMap.insert(itemGroup, UBGraphicsItem::getOwnUuid(item));
                    if (itemGroup->childItems().count() == 1) {
                        groupsMap.insert(itemGroup, UBGraphicsItem::getOwnUuid(itemGroup->childItems().constFirst()));
                        QGraphicsItem *lastItem = itemGroup->childItems().constFirst();
                        bool isSelected = itemGroup->isSelected();
                        itemGroup->destroy(false);
                        lastItem->setSelected(isSelected);
                    }
                    itemGroup->Delegate()->update();
                }

                curDelegate->remove(false);
                removedItems << item;
            }
        }
        break;
    }

    // force refresh, QT is a bit lazy and take a lot of time (nb item ^2 ?) to trigger repaint
    update(sceneRect());

    if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented

        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(shared_from_this(), removedItems, QSet<QGraphicsItem*>(), groupsMap);
        UBApplication::undoStack->push(uc);
    }

    if (pCase == clearBackground) {
        mBackgroundObject = 0;
    }

    setDocumentUpdated();
}

void UBGraphicsScene::saveWidgetSnapshots()
{
    foreach (auto item, items())
    {
        if (item->type() == UBGraphicsItemType::GraphicsWidgetItemType)
        {
            UBGraphicsWidgetItem* widget = qgraphicsitem_cast<UBGraphicsWidgetItem *>(item);
            widget->takeSnapshot();
            widget->saveSnapshot();
        }
    }

}

UBGraphicsPixmapItem* UBGraphicsScene::addPixmap(const QPixmap& pPixmap, QGraphicsItem* replaceFor, const QPointF& pPos, qreal pScaleFactor, bool pUseAnimation, bool useProxyForDocumentPath)
{
    // create PNG data from pixmap
    QBuffer buffer;
    pPixmap.save(&buffer, "png");

    return addImage(buffer.data(), replaceFor, pPos, pScaleFactor, pUseAnimation, useProxyForDocumentPath);
}

UBGraphicsPixmapItem* UBGraphicsScene::addImage(QByteArray pData, QGraphicsItem* replaceFor, const QPointF& pPos, qreal pScaleFactor, bool pUseAnimation, bool useProxyForDocumentPath)
{
    QBuffer buffer(&pData);
    QImageReader imageReader(&buffer);
    imageReader.setAutoTransform(true);
    QString format = imageReader.format();
    QImage img = imageReader.read();
    QPixmap pixmap = QPixmap::fromImage(img);

    UBGraphicsPixmapItem* pixmapItem = new UBGraphicsPixmapItem();

    pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

    pixmapItem->setPixmap(pixmap);

    QPointF half(pixmap.width() * pScaleFactor / 2, pixmap.height()  * pScaleFactor / 2);
    pixmapItem->setPos(pPos - half);

    addItem(pixmapItem);

    if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(shared_from_this(), replaceFor, pixmapItem);
        UBApplication::undoStack->push(uc);
    }

    pixmapItem->setTransform(QTransform::fromScale(pScaleFactor, pScaleFactor), true);

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

    QString documentPath;
    if(useProxyForDocumentPath)
        documentPath = this->document()->persistencePath();
    else
        documentPath = UBApplication::boardController->selectedDocument()->persistencePath();

    if (format != "png")
    {
        // provide compatibility with OpenBoard < 1.7.0 which uses 'contains("png")' as image indicator
        format = "png." + format;
    }

    QString fileName = UBPersistenceManager::imageDirectory + "/" + pixmapItem->uuid().toString() + "." + format;

    QString path = documentPath + "/" + fileName;

    if (!QFile::exists(path))
    {
        QDir dir;
        dir.mkdir(documentPath + "/" + UBPersistenceManager::imageDirectory);
        QFile file(path);

        if (file.open(QFile::WriteOnly))
        {
            file.write(pData);
            file.close();
        }
    }

    return pixmapItem;
}

void UBGraphicsScene::textUndoCommandAdded(UBGraphicsTextItem *textItem)
{
    if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
        UBGraphicsTextItemUndoCommand* uc = new UBGraphicsTextItemUndoCommand(textItem);
        UBApplication::undoStack->push(uc);
    }
}
UBGraphicsMediaItem* UBGraphicsScene::addMedia(const QUrl& pMediaFileUrl, bool shouldPlayAsap, const QPointF& pPos)
{
    qDebug() << pMediaFileUrl.toLocalFile();
    if (!QFile::exists(pMediaFileUrl.toLocalFile()))
    if (!QFile::exists(pMediaFileUrl.toString()))
        return NULL;

    UBGraphicsMediaItem * mediaItem = UBGraphicsMediaItem::createMediaItem(pMediaFileUrl);

    if (mediaItem)
    {
        connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), mediaItem, SLOT(activeSceneChanged()));

        mediaItem->setPos(pPos);

        mediaItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        mediaItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

        addItem(mediaItem);

        mediaItem->show();

        if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
            UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(shared_from_this(), 0, mediaItem);
            UBApplication::undoStack->push(uc);
        }

        if (shouldPlayAsap)
            mediaItem->play();
    }

    setDocumentUpdated();

    return mediaItem;
}

UBGraphicsMediaItem* UBGraphicsScene::addVideo(const QUrl& pVideoFileUrl, bool shouldPlayAsap, const QPointF& pPos)
{
   return addMedia(pVideoFileUrl, shouldPlayAsap, pPos);
}

UBGraphicsMediaItem* UBGraphicsScene::addAudio(const QUrl& pAudioFileUrl, bool shouldPlayAsap, const QPointF& pPos)
{
   return addMedia(pAudioFileUrl, shouldPlayAsap, pPos);
}

UBGraphicsWidgetItem* UBGraphicsScene::addWidget(const QUrl& pWidgetUrl, const QPointF& pPos)
{
    int widgetType = UBGraphicsWidgetItem::widgetType(pWidgetUrl);

    if(widgetType == UBWidgetType::Apple) // NOTE @letsfindaway obsolete
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

// NOTE @letsfindaway obsolete
UBGraphicsAppleWidgetItem* UBGraphicsScene::addAppleWidget(const QUrl& pWidgetUrl, const QPointF& pPos)
{
    UBGraphicsAppleWidgetItem *appleWidget = new UBGraphicsAppleWidgetItem(pWidgetUrl);

    addGraphicsWidget(appleWidget, pPos);

    return appleWidget;
}

UBGraphicsW3CWidgetItem* UBGraphicsScene::addW3CWidget(const QUrl& pWidgetUrl, const QPointF& pPos)
{
    UBGraphicsW3CWidgetItem *w3CWidget = new UBGraphicsW3CWidgetItem(pWidgetUrl, 0);

    addGraphicsWidget(w3CWidget, pPos);

    return w3CWidget;
}

void UBGraphicsScene::addGraphicsWidget(UBGraphicsWidgetItem* graphicsWidget, const QPointF& pPos)
{
    graphicsWidget->setFlag(QGraphicsItem::ItemIsSelectable, true);

    addItem(graphicsWidget);

    qreal ssf = 1 / UBApplication::boardController->systemScaleFactor();

    graphicsWidget->setTransform(QTransform::fromScale(ssf, ssf), true);

    graphicsWidget->setPos(QPointF(pPos.x() - graphicsWidget->boundingRect().width() / 2,
        pPos.y() - graphicsWidget->boundingRect().height() / 2));

    if (graphicsWidget->canBeContent())
    {
        graphicsWidget->loadMainHtml();

        graphicsWidget->setSelected(true);
        if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
            UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(shared_from_this(), 0, graphicsWidget);
            UBApplication::undoStack->push(uc);
        }

        setDocumentUpdated();
    }
    else
    {
        UBApplication::boardController->moveGraphicsWidgetToControlView(graphicsWidget);
    }

    UBApplication::boardController->controlView()->setFocus();
}


UBGraphicsGroupContainerItem *UBGraphicsScene::createGroup(QList<QGraphicsItem *> items)
{
    UBGraphicsGroupContainerItem *groupItem = new UBGraphicsGroupContainerItem();

    addItem(groupItem);
    foreach (QGraphicsItem *item, items) {
        if (item->type() == UBGraphicsGroupContainerItem::Type) {
            QList<QGraphicsItem*> childItems = item->childItems();
            UBGraphicsGroupContainerItem *currentGroup = dynamic_cast<UBGraphicsGroupContainerItem*>(item);
            if (currentGroup) {
                currentGroup->destroy();
            }
            foreach (QGraphicsItem *chItem, childItems) {
                groupItem->addToGroup(chItem);
            }
        } else {
            groupItem->addToGroup(item);
        }
    }

    groupItem->setVisible(true);
    groupItem->setFocus();

    if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemGroupUndoCommand* uc = new UBGraphicsItemGroupUndoCommand(shared_from_this(), groupItem);
        UBApplication::undoStack->push(uc);
    }

    setDocumentUpdated();

    return groupItem;
}

void UBGraphicsScene::addGroup(UBGraphicsGroupContainerItem *groupItem)
{
    addItem(groupItem);

    groupItem->setVisible(true);
    groupItem->setFocus();

    if (groupItem->uuid().isNull()) {
        groupItem->setUuid(QUuid::createUuid());
    }

    if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(shared_from_this(), 0, groupItem);
        UBApplication::undoStack->push(uc);
    }

    setDocumentUpdated();
}

UBGraphicsSvgItem* UBGraphicsScene::addSvg(const QUrl& pSvgFileUrl, const QPointF& pPos, const QByteArray pData)
{
    QString path = pSvgFileUrl.toLocalFile();

    UBGraphicsSvgItem *svgItem;
    if (pData.isNull())
        svgItem = new UBGraphicsSvgItem(path);
    else
        svgItem = new UBGraphicsSvgItem(pData);

    svgItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    svgItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

    qreal sscale = 1 / UBApplication::boardController->systemScaleFactor();
    svgItem->setTransform(QTransform::fromScale(sscale, sscale), true);

    QPointF half(svgItem->boundingRect().width() / 2, svgItem->boundingRect().height() / 2);
    svgItem->setPos(pPos - half);
    svgItem->show();

    addItem(svgItem);

    if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(shared_from_this(), 0, svgItem);
        UBApplication::undoStack->push(uc);
    }

    setDocumentUpdated();

    QString documentPath = UBApplication::boardController->selectedDocument()->persistencePath();

    QString fileName = UBPersistenceManager::imageDirectory + "/" + svgItem->uuid().toString() + ".svg";

    QString completePath = documentPath + "/" + fileName;

    if (!QFile::exists(completePath))
    {
        QDir dir;
        dir.mkdir(documentPath + "/" + UBPersistenceManager::imageDirectory);

        QFile file(completePath);
        if (!file.open(QIODevice::WriteOnly))
        {
            qWarning() << "cannot open file for writing embeded svg content " << completePath;
            return NULL;
        }

        file.write(svgItem->fileData());
        file.close();
    }

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

    if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(shared_from_this(), 0, textItem);
        UBApplication::undoStack->push(uc);
    }

    connect(textItem, SIGNAL(textUndoCommandAdded(UBGraphicsTextItem*)), this, SLOT(textUndoCommandAdded(UBGraphicsTextItem*)));

    textItem->setSelected(true);
    textItem->setFocus();

    setDocumentUpdated();

    return textItem;
}

UBGraphicsTextItem *UBGraphicsScene::addTextHtml(const QString &pString, const QPointF& pTopLeft)
{
    UBGraphicsTextItem *textItem = new UBGraphicsTextItem();

    textItem->setPlainText("");
    textItem->setHtml(UBTextTools::cleanHtml(pString));

    textItem->initFontProperties();

    addItem(textItem);
    textItem->show();

    if (mUndoRedoStackEnabled) { //should be deleted after scene own undo stack implemented
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(shared_from_this(), 0, textItem);
        UBApplication::undoStack->push(uc);
    }

    connect(textItem, SIGNAL(textUndoCommandAdded(UBGraphicsTextItem*)), this, SLOT(textUndoCommandAdded(UBGraphicsTextItem*)));

    textItem->setFocus();

    setDocumentUpdated();
    textItem->setPos(pTopLeft);

    return textItem;
}

void UBGraphicsScene::addItem(QGraphicsItem* item)
{
    UBCoreGraphicsScene::addItem(item);

    // the default z value is already set. This is the case when a svg file is read
    if(item->zValue() == DEFAULT_Z_VALUE
            || item->zValue() == UBZLayerController::errorNum())
    {
        qreal zvalue = mZLayerController->generateZLevel(item);
        UBGraphicsItem::assignZValue(item, zvalue);
    }

    else
        notifyZChanged(item, item->zValue());

    if (!mTools.contains(item))
    {
        ++mItemCount;
        setModified(true);
    }

    auto widget = dynamic_cast<UBGraphicsWidgetItem*>(item);

    if (widget)
    {
        widget->initAPI();
    }

    UBGraphicsItem* ubitem = dynamic_cast<UBGraphicsItem*>(item);

    if (ubitem)
    {
        ubitem->Delegate()->sceneChanged(this);
    }
}

void UBGraphicsScene::addItems(const QSet<QGraphicsItem*>& items)
{
    foreach(QGraphicsItem* item, items) {
        UBCoreGraphicsScene::addItem(item);
        UBGraphicsItem::assignZValue(item, mZLayerController->generateZLevel(item));
    }

    mItemCount += items.size();
    setModified(true);
}

void UBGraphicsScene::removeItem(QGraphicsItem* item)
{
    item->setSelected(false);
    UBCoreGraphicsScene::removeItem(item);
    UBApplication::boardController->freezeW3CWidget(item, true);

    if (!mTools.contains(item))
    {
        --mItemCount;
        setModified(true);
    }
}

void UBGraphicsScene::removeItems(const QSet<QGraphicsItem*>& items)
{
    foreach(QGraphicsItem* item, items)
        UBCoreGraphicsScene::removeItem(item);

    mItemCount -= items.size();
    setModified(true);
}

void UBGraphicsScene::deselectAllItems()
{
    foreach(QGraphicsItem *gi, selectedItems())
    {
        gi->clearFocus();
        gi->setSelected(false);
        // Hide selection frame
        if (mSelectionFrame) {
            mSelectionFrame->setEnclosedItems(QList<QGraphicsItem*>());
        }
        UBGraphicsTextItem* textItem = dynamic_cast<UBGraphicsTextItem*>(gi);
        if(textItem)
            textItem->activateTextEditor(false);
    }
}

void UBGraphicsScene::deselectAllItemsExcept(QGraphicsItem* item)
{
    foreach(QGraphicsItem* eachItem,selectedItems()){
        if(eachItem != item){
            eachItem->setSelected(false);

            UBGraphicsTextItem* textItem = dynamic_cast<UBGraphicsTextItem*>(eachItem);
            if(textItem)
                textItem->activateTextEditor(false);
        }
    }
}

/**
 * Return the bounding rectangle of all items on the page except for tools (ruler, compass,...)
 */
QRectF UBGraphicsScene::annotationsBoundingRect() const
{
    QRectF boundingRect;

    foreach (QGraphicsItem *item, items()) {
        if (!mTools.contains(rootItem(item)))
            boundingRect |= item->sceneBoundingRect();
    }

    return boundingRect;
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

        if (pAdaptTransformation)
        {
            item = scaleToFitDocumentSize(item, true, 0, pExpand);
        }

        if (item->scene() != this)
            addItem(item);

        mZLayerController->setLayerType(item, itemLayerType::BackgroundItem);
        UBGraphicsItem::assignZValue(item, mZLayerController->generateZLevel(item));

        mBackgroundObject = item;

    }

    return item;
}

void UBGraphicsScene::unsetBackgroundObject()
{
    if (!mBackgroundObject)
        return;

    mBackgroundObject->setFlag(QGraphicsItem::ItemIsSelectable, true);
    mBackgroundObject->setFlag(QGraphicsItem::ItemIsMovable, true);
    mBackgroundObject->setAcceptedMouseButtons(Qt::LeftButton);

    // Item zLayer and Layer Type should be set by the caller of this function, as
    // it may depend on the object type, where it was before, etc.

    mBackgroundObject = 0;
}

QRectF UBGraphicsScene::normalizedSceneRect(qreal ratio)
{

    QRectF normalizedRect(nominalSize().width() / -2., nominalSize().height() / -2.,
        nominalSize().width(), nominalSize().height());

    foreach(QGraphicsItem* gi, items())
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

QGraphicsItem *UBGraphicsScene::itemForUuid(QUuid uuid)
{
    QGraphicsItem *result = 0;

    //simple search before implementing container for fast access
    foreach (QGraphicsItem *item, items())
    {
        if (UBGraphicsScene::getPersonalUuid(item) == uuid && !uuid.isNull()) {
            result = item;
        }
    }

    return result;
}

void UBGraphicsScene::setDocument(std::shared_ptr<UBDocumentProxy> pDocument)
{
    if (pDocument != mDocument)
    {
        if (mDocument)
        {
            setModified(true);
        }

        mDocument = pDocument;
    }
}

const UBBackgroundRuling* UBGraphicsScene::background() const
{
    return mBackground;
}

QGraphicsItem* UBGraphicsScene::scaleToFitDocumentSize(QGraphicsItem* item, bool center, int margin, bool expand)
{
    int maxWidth = mNominalSize.width() - (margin * 2);
    int maxHeight = mNominalSize.height() - (margin * 2);

    QRectF size = item->sceneBoundingRect();

    if (expand || size.width() > maxWidth || size.height() > maxHeight)
    {
        qreal ratio = qMin(maxWidth / size.width(), maxHeight / size.height());

        item->setTransform(QTransform::fromScale(ratio, ratio), true);

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

    ruler->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    addItem(ruler);

    ruler->setVisible(true);
}

void UBGraphicsScene::addAxes(QPointF center)
{
    UBGraphicsAxes* axes = new UBGraphicsAxes(); // mem : owned and destroyed by the scene

    axes->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    addItem(axes);

    QPointF itemSceneCenter = axes->sceneBoundingRect().center();
    axes->moveBy(center.x() - itemSceneCenter.x(), center.y() - itemSceneCenter.y());

    axes->setVisible(true);
}

void UBGraphicsScene::addProtractor(QPointF center)
{
    // Protractor

    UBGraphicsProtractor* protractor = new UBGraphicsProtractor(); // mem : owned and destroyed by the scene
    mTools << protractor;

    protractor->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    addItem(protractor);

    QPointF itemSceneCenter = protractor->sceneBoundingRect().center();
    protractor->moveBy(center.x() - itemSceneCenter.x(), center.y() - itemSceneCenter.y());

    protractor->setVisible(true);
}

void UBGraphicsScene::addTriangle(QPointF center)
{
    // Triangle

    UBGraphicsTriangle* triangle = new UBGraphicsTriangle(); // mem : owned and destroyed by the scene
    mTools << triangle;

    triangle->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    addItem(triangle);

    QPointF itemSceneCenter = triangle->sceneBoundingRect().center();
    triangle->moveBy(center.x() - itemSceneCenter.x(), center.y() - itemSceneCenter.y());

    triangle->setVisible(true);
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
    connect(magniferControlViewWidget, SIGNAL(magnifierDrawingModeChange_Signal(int)), this, SLOT(changeMagnifierMode(int)));
    connect(magniferControlViewWidget, SIGNAL(magnifierResized_Signal(qreal)), this, SLOT(resizedMagnifier(qreal)));

    setModified(true);
}

void UBGraphicsScene::moveMagnifier()
{
   if (magniferControlViewWidget)
   {
       QPoint magnifierPos = QPoint(magniferControlViewWidget->pos().x() + magniferControlViewWidget->size().width() / 2, magniferControlViewWidget->pos().y() + magniferControlViewWidget->size().height() / 2 );
       moveMagnifier(magnifierPos, true);
       setModified(true);
   }
}

void UBGraphicsScene::moveMagnifier(QPoint newPos, bool forceGrab)
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

    magniferControlViewWidget->grabNMove(globalPoint, globalPoint, forceGrab, false);
    magniferDisplayViewWidget->grabNMove(globalPoint, dvPoint, forceGrab, true);

    setModified(true);
}

void UBGraphicsScene::closeMagnifier()
{
    DisposeMagnifierQWidgets();
    setModified(true);
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
        setModified(true);
    }
}

void UBGraphicsScene::changeMagnifierMode(int mode)
{
    if(magniferControlViewWidget)
        magniferControlViewWidget->setDrawingMode(mode);

    if(magniferDisplayViewWidget)
        magniferDisplayViewWidget->setDrawingMode(mode);
}

void UBGraphicsScene::resizedMagnifier(qreal newPercent)
{
    if(newPercent > 18 && newPercent < 50)
    {
        magniferControlViewWidget->setSize(newPercent);
        magniferControlViewWidget->grabPoint();
        magniferDisplayViewWidget->setSize(newPercent);
        magniferDisplayViewWidget->grabPoint();
        setModified(true);
    }
}

void UBGraphicsScene::stylusToolChanged(int tool, int previousTool)
{
    if (tool != previousTool)
    {
        hideTool();

        if (mInputDeviceIsPressed)
        {
            // tool was changed while input device is pressed
            // simulate release and press to terminate previous strokes
            inputDeviceRelease(previousTool);
            inputDevicePress(mCurrentPoint);
        }
        else if (previousTool >= 0)
        {
            inputDeviceMove(mCurrentPoint);
        }
    }
}

void UBGraphicsScene::controlViewportChanged()
{
    // inform all UBGraphicsWidgetItems about viewport change
    // partial workaround for QTBUG-109068
    foreach (QGraphicsItem *item, items())
    {
        if (item->type() == UBGraphicsWidgetItem::Type)
        {
            UBGraphicsWidgetItem* widgetItem = qgraphicsitem_cast<UBGraphicsWidgetItem *>(item);
            widgetItem->updatePosition();
        }
    }
}

void UBGraphicsScene::addCompass(QPointF center)
{
    UBGraphicsCompass* compass = new UBGraphicsCompass(); // mem : owned and destroyed by the scene
    mTools << compass;
    addItem(compass);

    QRectF rect = compass->rect();
    compass->setRect(center.x() - rect.width() / 2, center.y() - rect.height() / 2, rect.width(), rect.height());

    compass->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    compass->setVisible(true);
}

void UBGraphicsScene::addCache()
{
    UBGraphicsCache* cache = graphicsCache();

    if (!items().contains(cache)) {
        addItem(cache);

        cache->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

        cache->setVisible(true);
        cache->setSelected(true);
        UBApplication::boardController->notifyCache(true);
        UBApplication::boardController->notifyPageChanged();
    }
}

UBGraphicsCache* UBGraphicsScene::graphicsCache()
{
    if (!mGraphicsCache)
    {
        mGraphicsCache = new UBGraphicsCache;
    }

    return mGraphicsCache;
}

bool UBGraphicsScene::isSnapping() const
{
    return UBApplication::mainWindow->actionSnap->isChecked();
}

QPointF UBGraphicsScene::snap(const QPointF& point, double* force, std::optional<QPointF> proposedPoint, QPointF* gridSnapPoint) const
{
    if (!mBackground)
    {
        return {};
    }

    QRectF nominalScene{{0, 0}, mNominalSize};
    nominalScene.moveCenter({0, 0});
    return mBackground->snap(point, backgroundGridSize(), nominalScene, force, proposedPoint, gridSnapPoint);
}

QPointF UBGraphicsScene::snap(const std::vector<QPointF>& corners, int* snapIndex) const
{
    if (corners.empty())
    {
        if (snapIndex)
        {
            *snapIndex = -1;
        }

        return QPointF{};
    }

    std::vector<double> forces;
    std::vector<QPointF> snapVectors;

    for (const auto& corner : corners)
    {
        double force = 0.;
        snapVectors.emplace_back(snap(corner, &force));
        forces.emplace_back(force);
    }

    const auto maxElement = std::max_element(forces.cbegin(), forces.cend());
    const auto index = std::distance(forces.cbegin(), maxElement);

    if (snapIndex)
    {
        *snapIndex = index;
    }

    return snapVectors[index];
}

QPointF UBGraphicsScene::snap(const QRectF& rect, Qt::Corner* corner) const
{
    int snapIndex;
    std::vector<QPointF> rectPoints{rect.topLeft(), rect.topRight(), rect.bottomLeft(), rect.bottomRight()};
    const auto offset = snap(rectPoints, &snapIndex);
    const auto snapCorner = Qt::Corner(snapIndex);

    if (corner)
    {
        *corner = snapCorner;
    }

    if (!offset.isNull())
    {
        auto* view = UBApplication::boardController->controlView();
        view->updateSnapIndicator(snapCorner, rectPoints.at(snapIndex) + offset);
    }

    return offset;
}

QRectF UBGraphicsScene::itemRect(const QGraphicsItem* item)
{
    // compute an item's rectangle in scene coordinates
    // taking into account the shape of the item and
    // the nature of nominal lines
    QRectF bounds = item->boundingRect();

    const QAbstractGraphicsShapeItem* shapeItem = dynamic_cast<const QAbstractGraphicsShapeItem*>(item);

    if (shapeItem && shapeItem->pen().style() != Qt::NoPen)
    {
        qreal margin = shapeItem->pen().widthF() / 2.f;
        bounds -= QMarginsF(margin, margin, margin, margin);
    }

    // Try to find out whether the item is a single line
    // Note: this only works for lines drawn within the current session as the isNominalLine
    // and originalLine attributes are lost when serializing the document.
    const UBGraphicsStrokesGroup* strokesGroup = dynamic_cast<const UBGraphicsStrokesGroup*>(item);

    if (strokesGroup && strokesGroup->childItems().count() == 1)
    {
        UBGraphicsPolygonItem* polygonItem = dynamic_cast<UBGraphicsPolygonItem*>(strokesGroup->childItems().at(0));

        if (polygonItem && polygonItem->isNominalLine())
        {
            const auto line = polygonItem->originalLine();
            bounds = QRectF{line.p1(), line.p2()};
        }
    }

    QRectF rect = item->mapRectToScene(bounds);

    return rect;
}

void UBGraphicsScene::addMask(const QPointF &center)
{
    UBGraphicsCurtainItem* curtain = new UBGraphicsCurtainItem(); // mem : owned and destroyed by the scene
    mTools << curtain;

    addItem(curtain);

    QRectF rect = UBApplication::boardController->activeScene()->normalizedSceneRect();
    rect.setRect(center.x() - rect.width()/4, center.y() - rect.height()/4, rect.width()/2 , rect.height()/2);
    curtain->setRect(rect);
    curtain->setVisible(true);
    curtain->setSelected(true);
}

void UBGraphicsScene::setRenderingQuality(UBItem::RenderingQuality pRenderingQuality, UBItem::CacheBehavior cacheBehavior)
{
    foreach (auto item, items())
    {
        UBItem *ubItem = dynamic_cast<UBItem*>(item);

        if (ubItem)
        {
            ubItem->setRenderingQuality(pRenderingQuality);
            ubItem->setCacheBehavior(cacheBehavior);
        }
    }
}

QList<QUrl> UBGraphicsScene::relativeDependenciesOfItem(QGraphicsItem* item) const
{
    QList<QUrl> relativePaths;

    UBGraphicsVideoItem *videoItem = dynamic_cast<UBGraphicsVideoItem*> (item);
    if (videoItem){
        QString completeFileName = QFileInfo(videoItem->mediaFileUrl().toLocalFile()).fileName();
        QString path = UBPersistenceManager::videoDirectory + "/";
        relativePaths << QUrl(path + completeFileName);
        return relativePaths;
    }

    UBGraphicsAudioItem *audioItem =  dynamic_cast<UBGraphicsAudioItem*> (item);
    if (audioItem){
        QString completeFileName = QFileInfo(audioItem->mediaFileUrl().toLocalFile()).fileName();
        QString path = UBPersistenceManager::audioDirectory + "/";
        relativePaths << QUrl(path + completeFileName);
        return relativePaths;
    }

    UBGraphicsWidgetItem* widget = dynamic_cast<UBGraphicsWidgetItem*>(item);
    if(widget){
        QString widgetPath = UBPersistenceManager::widgetDirectory + "/" + widget->uuid().toString() + ".wgt";
        QString screenshotPath = UBPersistenceManager::widgetDirectory + "/" + widget->uuid().toString().remove("{").remove("}") + ".png";
        relativePaths << QUrl(widgetPath);
        relativePaths << QUrl(screenshotPath);
        return relativePaths;
    }

    UBGraphicsPixmapItem* pixmapItem = dynamic_cast<UBGraphicsPixmapItem*>(item);
    if(pixmapItem){
        QDir imageDir = mDocument->persistencePath() + "/" + UBPersistenceManager::imageDirectory;
        QStringList imageFiles = imageDir.entryList({pixmapItem->uuid().toString() + ".*"});

        if (!imageFiles.isEmpty())
        {
            relativePaths << QUrl(UBPersistenceManager::imageDirectory + "/" + imageFiles.last());
        }

        return relativePaths;
    }

    UBGraphicsSvgItem* svgItem = dynamic_cast<UBGraphicsSvgItem*>(item);
    if(svgItem){
        relativePaths << QUrl(UBPersistenceManager::imageDirectory + "/" + svgItem->uuid().toString() + ".svg");
        return relativePaths;
    }

    return relativePaths;
}

QList<QUrl> UBGraphicsScene::relativeDependencies() const
{
    QList<QUrl> relativePaths;

    foreach(auto item, items())
    {
        UBGraphicsGroupContainerItem* groupItem = dynamic_cast<UBGraphicsGroupContainerItem*>(item);
        if(groupItem)
        {
            foreach (QGraphicsItem* child, groupItem->childItems())
            {
                relativePaths << relativeDependenciesOfItem(child);
            }
        }
        else
            relativePaths << relativeDependenciesOfItem(item);
    }

    return relativePaths;
}

QSize UBGraphicsScene::nominalSize()
{
    if (mDocument && !mNominalSize.isValid())
    {
        mNominalSize = mDocument->defaultDocumentSize();
    }

    return mNominalSize;
}

/**
 * @brief Return the scene's boundary size, including any background item
 *
 * If no background item is present, this returns nominalSize()
 */
QSize UBGraphicsScene::sceneSize()
{
    UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(backgroundObject());

    if (pdfItem) {
        QRectF targetRect = pdfItem->sceneBoundingRect();
        return targetRect.size().toSize();
    }

    else
        return nominalSize();
}

QSizeF UBGraphicsScene::sceneSizeF() const
{
    UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(backgroundObject());

    if (pdfItem)
    {
        QRectF targetRect = pdfItem->sceneBoundingRect();
        return targetRect.size();
    }
    else if (mDocument && !mNominalSize.isValid())
    {
        return mDocument->defaultDocumentSize();
    }
    else
    {
        return mNominalSize;
    }
}

void UBGraphicsScene::setNominalSize(const QSize& pSize)
{
    if (nominalSize() != pSize)
    {
        mNominalSize = pSize;
        updateBackground();

        if(mDocument)
            mDocument->setDefaultDocumentSize(pSize);
    }
}

void UBGraphicsScene::setNominalSize(int pWidth, int pHeight)
{
     setNominalSize(QSize(pWidth, pHeight));
}

void UBGraphicsScene::setSelectedZLevel(QGraphicsItem * item)
{
    item->setZValue(mZLayerController->generateZLevel(itemLayerType::SelectedItem));
}

void UBGraphicsScene::setOwnZlevel(QGraphicsItem *item)
{
    item->setZValue(item->data(UBGraphicsItemData::ItemOwnZValue).toReal());
}

QUuid UBGraphicsScene::getPersonalUuid(QGraphicsItem *item)
{
    QString idCandidate = item->data(UBGraphicsItemData::ItemUuid).toString();
    return idCandidate == QUuid().toString() ? QUuid() : QUuid(idCandidate);
}

qreal UBGraphicsScene::changeZLevelTo(QGraphicsItem *item, UBZLayerController::moveDestination dest, bool addUndo)
{
    qreal previousZVal = item->data(UBGraphicsItemData::ItemOwnZValue).toReal();
    qreal res = mZLayerController->changeZLevelTo(item, dest);

    if(addUndo){
        UBGraphicsItemZLevelUndoCommand* uc = new UBGraphicsItemZLevelUndoCommand(shared_from_this(), item, previousZVal, dest);
        UBApplication::undoStack->push(uc);
    }

    return res;
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
                bool isPdfItem =  qgraphicsitem_cast<UBGraphicsPDFItem*> (items[i]) != NULL;
                if(!isPdfItem || mRenderingContext == NonScreen)
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

void UBGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (mIsDesktopMode)
    {
        QGraphicsScene::drawBackground (painter, rect);
        return;
    }

    bool darkBackground = isDarkBackground ();

    if (darkBackground)
    {
        painter->fillRect (rect, QBrush (QColor (Qt::black)));
    }
    else
    {
        painter->fillRect (rect, QBrush (QColor (Qt::white)));
    }

    if (mZoomFactor > 0.5)
    {
        if (mZoomFactor < 0.7)
        {
            painter->setOpacity(mZoomFactor / 2);
        }

        if (mBackground)
        {
            qreal gridSize = backgroundGridSize();
            QRectF nominalScene{{0, 0}, mNominalSize};
            nominalScene.moveCenter({0, 0});
            mBackground->draw(painter, rect, gridSize, nominalScene, mDarkBackground);
        }
    }
}

void UBGraphicsScene::keyReleaseEvent(QKeyEvent * keyEvent)
{
    // let's propagate the event through the scene's children to
    // see if it must be handled by a child before trying to handle it as
    // a scene key event. Note that the child must accept() the event
    // to stop the propagation.
    keyEvent->ignore();
    QGraphicsScene::keyReleaseEvent(keyEvent);

    if (!keyEvent->isAccepted())
    {
        // Select All scene event
        if(keyEvent->matches(QKeySequence::SelectAll))
        {
            foreach(auto item, items())
            {
                UBGraphicsItem* ubGraphicsItem = dynamic_cast<UBGraphicsItem*>(item);

                if (ubGraphicsItem) //only select items that inherit from UBGraphicsItem
                    item->setSelected(true);
            }

            keyEvent->accept();
        }

        // Delete selection scene event
#ifdef Q_OS_MAC
        if (keyEvent->key() == Qt::Key_Backspace)
#else
        if (keyEvent->matches(QKeySequence::Delete))
#endif
        {
            foreach(QGraphicsItem* item, selectedItems())
            {
                UBGraphicsItem* ubGraphicsItem = dynamic_cast<UBGraphicsItem*>(item);
                if (ubGraphicsItem)
                {
                    ubGraphicsItem->remove();
                }
                else //should never happen ?
                {
                    UBCoreGraphicsScene::removeItem(item);
                }
            }

            keyEvent->accept();
        }

        updateSelectionFrame();
    }
}

bool UBGraphicsScene::hasTextItemWithFocus(UBGraphicsGroupContainerItem *item){
    bool bHasFocus = false;

    foreach(QGraphicsItem* pItem, item->childItems()){
        UBGraphicsTextItem *text_item = dynamic_cast<UBGraphicsTextItem*>(pItem);
        if (NULL != text_item){
            if(text_item->hasFocus()){
                bHasFocus = true;
                break;
            }
        }
    }

    return bHasFocus;
}


void UBGraphicsScene::simplifyCurrentStroke()
{
    if (!mCurrentStroke)
        return;

    UBGraphicsStroke* simplerStroke = mCurrentStroke->simplify();
    if (!simplerStroke)
        return;

    foreach(UBGraphicsPolygonItem* poly, mCurrentStroke->polygons()){
        mPreviousPolygonItems.removeAll(poly);
        removeItem(poly);
    }

    mCurrentStroke = simplerStroke;

    foreach(UBGraphicsPolygonItem* poly, mCurrentStroke->polygons()) {
        addItem(poly);
        mPreviousPolygonItems.append(poly);
    }

}

void UBGraphicsScene::setDocumentUpdated()
{
    if (document())
    {
        QDateTime now = QDateTime::currentDateTime();
        document()->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(now));
    }
}

void UBGraphicsScene::updateBackground()
{
    foreach(QGraphicsView* view, views())
    {
        view->resetCachedContent();
    }

}

void UBGraphicsScene::createEraiser()
{
    if (UBSettings::settings()->showEraserPreviewCircle->get().toBool()) {
        mEraser = new QGraphicsEllipseItem(); // mem : owned and destroyed by the scene
        mEraser->setRect(QRect(0, 0, 0, 0));
        mEraser->setVisible(false);

        updateEraserColor();

        mEraser->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
        mEraser->setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Eraiser)); //Necessary to set if we want z value to be assigned correctly

        mTools << mEraser;
        UBGraphicsScene::addItem(mEraser);
    }
}

void UBGraphicsScene::createPointer()
{
    mPointer = new QGraphicsEllipseItem();  // mem : owned and destroyed by the scene
    mPointer->setRect(QRect(0, 0, 20, 20));
    mPointer->setVisible(false);

    mPointer->setPen(Qt::NoPen);
    mPointer->setBrush(QBrush(QColor(255, 0, 0, 186)));

    mPointer->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));
    mPointer->setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Pointer)); //Necessary to set if we want z value to be assigned correctly

    mTools << mPointer;
    UBGraphicsScene::addItem(mPointer);
}

void UBGraphicsScene::createMarkerCircle()
{
    if (UBSettings::settings()->showMarkerPreviewCircle->get().toBool()) {
        mMarkerCircle = new QGraphicsEllipseItem();

        mMarkerCircle->setRect(QRect(0, 0, 0, 0));
        mMarkerCircle->setVisible(false);

        mMarkerCircle->setPen(Qt::DotLine);
        updateMarkerCircleColor();

        mMarkerCircle->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
        mMarkerCircle->setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Eraiser));

        mTools << mMarkerCircle;
        UBGraphicsScene::addItem(mMarkerCircle);
    }
}

void UBGraphicsScene::createPenCircle()
{
    if (UBSettings::settings()->showPenPreviewCircle->get().toBool()) {
        mPenCircle = new QGraphicsEllipseItem();

        mPenCircle->setRect(QRect(0, 0, 0, 0));
        mPenCircle->setVisible(false);

        mPenCircle->setPen(Qt::DotLine);
        updatePenCircleColor();

        mPenCircle->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
        mPenCircle->setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Eraiser));

        mTools << mPenCircle;
        UBGraphicsScene::addItem(mPenCircle);
    }
}

void UBGraphicsScene::updateEraserColor()
{
    if (!mEraser)
        return;

    if (mDarkBackground) {
        mEraser->setBrush(UBSettings::eraserBrushDarkBackground);
        mEraser->setPen(UBSettings::eraserPenDarkBackground);
    }

    else {
        mEraser->setBrush(UBSettings::eraserBrushLightBackground);
        mEraser->setPen(UBSettings::eraserPenLightBackground);
    }
}

void UBGraphicsScene::updateMarkerCircleColor()
{
    if (!mMarkerCircle)
        return;

    QPen mcPen = mMarkerCircle->pen();

    if (mDarkBackground) {
        mcPen.setColor(UBSettings::markerCirclePenColorDarkBackground);
        mMarkerCircle->setBrush(UBSettings::markerCircleBrushColorDarkBackground);
    }

    else {
        mcPen.setColor(UBSettings::markerCirclePenColorLightBackground);
        mMarkerCircle->setBrush(UBSettings::markerCircleBrushColorLightBackground);
    }

    mcPen.setStyle(Qt::DotLine);
    mMarkerCircle->setPen(mcPen);
}

void UBGraphicsScene::updatePenCircleColor()
{
    if (!mPenCircle)
        return;

    QPen mcPen = mPenCircle->pen();

    if (mDarkBackground) {
        mcPen.setColor(UBSettings::penCirclePenColorDarkBackground);
        mPenCircle->setBrush(UBSettings::penCircleBrushColorDarkBackground);
    }

    else {
        mcPen.setColor(UBSettings::penCirclePenColorLightBackground);
        mPenCircle->setBrush(UBSettings::penCircleBrushColorLightBackground);
    }

    mcPen.setStyle(Qt::DotLine);
    mPenCircle->setPen(mcPen);
}

void UBGraphicsScene::setToolCursor(int tool)
{
    if (tool == (int)UBStylusTool::Selector ||
            tool == (int)UBStylusTool::Text ||
            tool == (int)UBStylusTool::Play) {
        deselectAllItems();
        hideMarkerCircle();
        hidePenCircle();
    }

    if (mCurrentStroke && mCurrentStroke->polygons().empty()){
        delete mCurrentStroke;
        mCurrentStroke = NULL;
    }

}

void UBGraphicsScene::initStroke()
{
    mCurrentStroke = new UBGraphicsStroke(shared_from_this());
}
