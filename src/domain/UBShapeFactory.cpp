#include "UBShapeFactory.h"
#include "UBGraphicsEllipseItem.h"
#include "UBGraphicsRectItem.h"
#include "UBGraphicsLineItem.h"
#include "UBEditableGraphicsRegularShapeItem.h"
#include "UBEditableGraphicsPolygonItem.h"
#include "UBGraphicsFreehandItem.h"
#include "UB1HEditableGraphicsCircleItem.h"
#include "UB1HEditableGraphicsSquareItem.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "gui/UBMainWindow.h"
#include "UBGraphicsScene.h"

UBShapeFactory::UBShapeFactory():
    mCurrentShape(NULL),
    mBoardView(NULL),
    mIsCreating(false),
    mIsPress(false),
    mIsRegularShape(true),
    mFirstClickForFreeHand(true),
    mCurrentStrokeColor(Qt::black),
    mCurrentFillFirstColor(Qt::transparent),
    mCurrentFillSecondColor(Qt::transparent),
    mDrawingController(NULL),
    mCurrentBrushStyle(Qt::SolidPattern),
    mCurrentPenStyle(Qt::SolidLine),
    mThickness(3),
    mFillType(Transparent),
    mCursorMoved(false),
    mStartArrowType(UBAbstractGraphicsPathItem::ArrowType_None),
    mEndArrowType(UBAbstractGraphicsPathItem::ArrowType_None)
{

}

void UBShapeFactory::prepareChangeFill()
{
   mDrawingController->setStylusTool(UBStylusTool::ChangeFill);
   mIsRegularShape = false;
   mIsCreating = false;
   mShapeType = None;
}

void UBShapeFactory::changeFillColor(const QPointF& pos)
{
    UBGraphicsScene* scene = mBoardView->scene();
    QGraphicsItem* item = scene->itemAt(pos, QTransform());
    UBAbstractGraphicsItem* shape = dynamic_cast<UBAbstractGraphicsItem*>(item);

    applyCurrentStyle(shape);

    // Because filling with Bucket doesn't need selection of any item on the board,
    // we have to explicitly set Scene as Modified.
    // (because that's Selection of Item that usually set Scene changed)
    scene->setModified(true);
}

void UBShapeFactory::applyCurrentStyle(UBAbstractGraphicsItem* shape)
{
    if (shape)
    {
        if (mFillType == Diag){
            if (shape->hasFillingProperty()){
                shape->setStyle(Qt::TexturePattern);
                shape->setFillPattern(UBAbstractGraphicsItem::FillPattern_Diag1);
                shape->setFillColor(mCurrentFillFirstColor);
            }
        }
        else if (mFillType == Dense){
            if (shape->hasFillingProperty()){
                shape->setStyle(Qt::TexturePattern);
                shape->setFillPattern(UBAbstractGraphicsItem::FillPattern_Dot1);
                shape->setFillColor(mCurrentFillFirstColor);
            }
        }
        else if (mFillType == Gradient){
            if (shape->hasFillingProperty())
                setGradientFillingProperty(shape);
        }
        else{
            if (shape->hasFillingProperty()){
                shape->setStyle(mCurrentBrushStyle);
                shape->setFillColor(mCurrentFillFirstColor);
            }
        }
    }
}

void UBShapeFactory::returnToCreationMode(QGraphicsItem* item)
{
    mDrawingController->setStylusTool(UBStylusTool::Drawing);    
    UBApplication::mainWindow->actionPolygon->trigger();

    UBEditableGraphicsPolygonItem* polygon = dynamic_cast<UBEditableGraphicsPolygonItem*>(item);
    if (polygon)
    {
        mCurrentShape = polygon;
        mShapeType = Polygon;
        mIsRegularShape = false;
        mIsCreating = true;
        applyCurrentStyle(polygon);

        if (polygon->isClosed())
            polygon->reopen();            
        else
            polygon->setOpened(false);

        polygon->setIsInCreationMode(true);
    }

    item->setSelected(false);
    item->update();
}

void UBShapeFactory::setGradientFillingProperty(UBAbstractGraphicsItem* shape)
{
    QRectF rect = shape->boundingRect();

    QLinearGradient gradient(rect.topLeft(), rect.topRight());
    gradient.setColorAt(0, mCurrentFillFirstColor);
    gradient.setColorAt(1, mCurrentFillSecondColor);
    shape->setBrush(gradient);
}

UBShapeFactory::FillType UBShapeFactory::fillType()
{
    return mFillType;
}

void UBShapeFactory::setFillType(FillType fillType)
{
    mFillType = fillType;
}

void UBShapeFactory::setStartArrowType(UBAbstractGraphicsPathItem::ArrowType arrowType)
{
    mStartArrowType = arrowType;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBAbstractGraphicsPathItem * abstractGraphicsPathItem = dynamic_cast<UBAbstractGraphicsPathItem*>(items.at(i));

        if(abstractGraphicsPathItem)
        {
            abstractGraphicsPathItem->setStartArrowType(arrowType);
        }

        items.at(i)->update();
    }
}

void UBShapeFactory::setEndArrowType(UBAbstractGraphicsPathItem::ArrowType arrowType)
{
    mEndArrowType = arrowType;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBAbstractGraphicsPathItem * abstractGraphicsPathItem = dynamic_cast<UBAbstractGraphicsPathItem*>(items.at(i));

        if(abstractGraphicsPathItem)
        {
            abstractGraphicsPathItem->setEndArrowType(arrowType);
        }

        items.at(i)->update();
    }
}

void UBShapeFactory::init()
{
    mBoardView = UBApplication::boardController->controlView();
    mDrawingController = UBDrawingController::drawingController();

    //Our custom dash is a point follow by a space
    mDotDashes << 0.1 << 3;

    connect(mBoardView, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMove(QMouseEvent*)));
    connect(mBoardView, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMouseRelease(QMouseEvent*)));
    connect(mBoardView, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePress(QMouseEvent*)));

}


UBAbstractGraphicsItem* UBShapeFactory::instanciateCurrentShape()
{
    switch (mShapeType) {
    case Ellipse:
        mCurrentShape = new UB3HEditableGraphicsEllipseItem();
        break;
    case Circle:
        mCurrentShape = new UB1HEditableGraphicsCircleItem();
        break;
    case Rectangle:
        mCurrentShape = new UB3HEditableGraphicsRectItem();
        break;
    case Square:
        mCurrentShape = new UB1HEditableGraphicsSquareItem();
        break;
    case Line:
        mCurrentShape = new UBEditableGraphicsLineItem();
        break;
    case Pen:
        mCurrentShape = new UBGraphicsFreehandItem();
        break;
    case Polygon:
        mCurrentShape = new UBEditableGraphicsPolygonItem();
        break;
    case RegularPolygon:
        mCurrentShape = new UBEditableGraphicsRegularShapeItem(mNVertices);
        break;
    default:
        break;
    }

    mCurrentShape->setStyle(mCurrentBrushStyle, mCurrentPenStyle);

    if(mCurrentPenStyle == Qt::CustomDashLine){
        QPen p = mCurrentShape->pen();
        p.setCapStyle(Qt::RoundCap);
        p.setDashPattern(mDotDashes);
        mCurrentShape->setPen(p);
    }

    if (mFillType == Diag)    {
        if (mCurrentShape->hasFillingProperty()){
            mCurrentShape->setStyle(Qt::TexturePattern);
            mCurrentShape->setFillPattern(UBAbstractGraphicsItem::FillPattern_Diag1);
            mCurrentShape->setFillColor(mCurrentFillFirstColor);
        }
    }
    else if (mFillType == Dense){
        if (mCurrentShape->hasFillingProperty()){
            mCurrentShape->setStyle(Qt::TexturePattern);
            mCurrentShape->setFillPattern(UBAbstractGraphicsItem::FillPattern_Dot1);
            mCurrentShape->setFillColor(mCurrentFillFirstColor);
        }
    }
    else if (mFillType == Gradient){
        if (mCurrentShape->hasFillingProperty()){
            QRectF rect = mCurrentShape->boundingRect();

            QLinearGradient gradient(rect.topLeft(), rect.topRight());

            gradient.setColorAt(0, mCurrentFillFirstColor);

            gradient.setColorAt(1, mCurrentFillSecondColor);

            mCurrentShape->setBrush(gradient);
        }
    }
    else{
        if (mCurrentShape->hasFillingProperty()){
            mCurrentShape->setStyle(mCurrentBrushStyle);
            mCurrentShape->setFillColor(mCurrentFillFirstColor);
        }
    }

    mCurrentShape->setStrokeColor(mCurrentStrokeColor);

    //qWarning()<<"mThickness: ";
    //qWarning()<<mThickness;
    mCurrentShape->setStrokeSize(mThickness);

    UBAbstractGraphicsPathItem * abstractGraphicsPathItem  = dynamic_cast<UBAbstractGraphicsPathItem*>(mCurrentShape);
    if (abstractGraphicsPathItem)
    {
        abstractGraphicsPathItem->setStartArrowType(mStartArrowType);
        abstractGraphicsPathItem->setEndArrowType(mEndArrowType);
    }

    return mCurrentShape;
}

void UBShapeFactory::createEllipse(bool create)
{
    if(create){
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Ellipse;
    }
}

void UBShapeFactory::createCircle(bool create)
{
    if(create){
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Circle;
    }
}

void UBShapeFactory::createRectangle(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Rectangle;
    }
}

void UBShapeFactory::createSquare(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Square;
    }
}

void UBShapeFactory::createLine(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Line;
    }
}

void UBShapeFactory::createPen(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = false;
        mIsCreating = true;
        mShapeType = Pen;
    }
}

void UBShapeFactory::createRegularPolygon(int nVertices)
{
    mDrawingController->setStylusTool(UBStylusTool::Drawing);
    mIsRegularShape = false;
    mIsCreating = true;
    mShapeType = RegularPolygon;
    mNVertices = nVertices;
}

void UBShapeFactory::createPolygon(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = false;
        mIsCreating = true;
        mShapeType = Polygon;
    }
}

void UBShapeFactory::onMouseMove(QMouseEvent *event)
{
    if(mIsCreating && mIsPress){
        mCursorMoved = true;
        QPointF cursorPosition = mBoardView->mapToScene(event->pos());

        if(mIsRegularShape)
        {
            if (mShapeType == Ellipse)
            {
                UB3HEditableGraphicsEllipseItem* shape = dynamic_cast<UB3HEditableGraphicsEllipseItem*>(mCurrentShape);
                QRectF rect = QRectF(shape->pos(), cursorPosition);

                //qWarning() << "before setting radius";
                shape->setRadiusX(rect.width()/2);
                shape->setRadiusY(rect.height()/2);
                /*QThread::msleep(50);*/
                //qWarning() << "after setting radius";
            }
            else if(mShapeType == Circle)
            {
                UB1HEditableGraphicsCircleItem* shape = dynamic_cast<UB1HEditableGraphicsCircleItem*>(mCurrentShape);

                shape->setRect(QRectF(shape->pos(), cursorPosition));
            }
            else if (mShapeType == Rectangle)
            {
                UB3HEditableGraphicsRectItem* shape = dynamic_cast<UB3HEditableGraphicsRectItem*>(mCurrentShape);

                shape->setRect(QRectF(shape->pos(), cursorPosition));
            }
            else if(mShapeType == Square)
            {
                UB1HEditableGraphicsSquareItem* shape = dynamic_cast<UB1HEditableGraphicsSquareItem*>(mCurrentShape);

                shape->setRect(QRectF(shape->pos(), cursorPosition));
            }
            else if (mShapeType == Line)
            {
                UBEditableGraphicsLineItem* line = dynamic_cast<UBEditableGraphicsLineItem*>(mCurrentShape);

                line->setEndPoint(cursorPosition);
            }
        }else{
            if(mShapeType == Pen){
                UBGraphicsFreehandItem *freeHand = dynamic_cast<UBGraphicsFreehandItem*>(mCurrentShape);
                if (freeHand)
                {
                    QPointF point = event->pos() - freeHand->path().currentPosition();

                    if(point.manhattanLength() > 3){
                        freeHand->addPoint(cursorPosition);
                    }

                    mBoundingRect = freeHand->boundingRect();
                }
            }else if (mShapeType == RegularPolygon){
                UBEditableGraphicsRegularShapeItem* regularPathItem = dynamic_cast<UBEditableGraphicsRegularShapeItem*>(mCurrentShape);
                regularPathItem->updatePath(cursorPosition);
                mBoundingRect = regularPathItem->boundingRect();
            }
        }
        if (mCurrentShape)
        {
            if (mFillType == Gradient)
            {
                if (mCurrentShape->hasFillingProperty())
                    setGradientFillingProperty(mCurrentShape);
            }
        }
    }
}

void UBShapeFactory::onMousePress(QMouseEvent *event)
{
    if(mIsCreating){
        mCursorMoved = false;
        mIsPress = true;

        QPointF cursorPosition = mBoardView->mapToScene(event->pos());

        if(mIsRegularShape){
            if (mShapeType == Ellipse)
            {
                UB3HEditableGraphicsEllipseItem* ellipse = dynamic_cast<UB3HEditableGraphicsEllipseItem*>(instanciateCurrentShape());
                ellipse->setPos(cursorPosition);
                mBoardView->scene()->addItem(ellipse);
            }
            else if(mShapeType == Circle)
            {
                UB1HEditableGraphicsCircleItem* ellipse = dynamic_cast<UB1HEditableGraphicsCircleItem*>(instanciateCurrentShape());
                ellipse->setPos(cursorPosition);

                mBoardView->scene()->addItem(ellipse);
            }
            else if (mShapeType == Rectangle)
            {
                UB3HEditableGraphicsRectItem* rect = dynamic_cast<UB3HEditableGraphicsRectItem*>(instanciateCurrentShape());

                rect->setRect(QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0));

                mBoardView->scene()->addItem(rect);
            }
            else if(mShapeType == Square)
            {
                UB1HEditableGraphicsSquareItem* rect = dynamic_cast<UB1HEditableGraphicsSquareItem*>(instanciateCurrentShape());

                rect->setRect(QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0));

                mBoardView->scene()->addItem(rect);
            }
            else if (mShapeType == Line)
            {
                UBEditableGraphicsLineItem* line = dynamic_cast<UBEditableGraphicsLineItem*>(instanciateCurrentShape());

                line->setLine(cursorPosition, cursorPosition);
                line->setMagnetic(true);

                mBoardView->scene()->addItem(line);
            }
        }else{
            if (mShapeType == RegularPolygon)
            {
                UBEditableGraphicsRegularShapeItem* regularPathItem = dynamic_cast<UBEditableGraphicsRegularShapeItem*>(instanciateCurrentShape());

                regularPathItem->setStartPoint(cursorPosition);

                mBoardView->scene()->addItem(regularPathItem);
            }
            else //Polygon
            {
                if(mShapeType == Pen){
                    if(mFirstClickForFreeHand){
                        UBGraphicsFreehandItem* pathItem = dynamic_cast<UBGraphicsFreehandItem*>(instanciateCurrentShape());


                        pathItem->addPoint(cursorPosition);

                        mFirstClickForFreeHand = false;

                        mBoardView->scene()->addItem(pathItem);
                    }
                }else{
                    UBEditableGraphicsPolygonItem* pathItem = dynamic_cast<UBEditableGraphicsPolygonItem*>(mCurrentShape);
                    if (mCurrentShape == NULL || pathItem == NULL)
                    {
                        pathItem = dynamic_cast<UBEditableGraphicsPolygonItem*>(instanciateCurrentShape());
                        mBoardView->scene()->addItem(pathItem);
                    }

                    pathItem->addPoint(cursorPosition);


                    if (pathItem->isClosed() || pathItem->isOpened())
                    {
                        if (pathItem->path().elementCount() < 2)
                            mBoardView->scene()->removeItem(pathItem);
                        mCurrentShape = NULL;
                    }
                }
            }
        }
    }

}

void UBShapeFactory::onMouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event);
    mIsPress = false;

    UBEditableGraphicsLineItem* line= dynamic_cast<UBEditableGraphicsLineItem*>(mCurrentShape);
    if (line)
    {
        if (line->startPoint() == line->endPoint())
             mBoardView->scene()->removeItem(line);
    }
    else if(mShapeType == Rectangle)
    {
        UB3HEditableGraphicsRectItem* shape = dynamic_cast<UB3HEditableGraphicsRectItem*>(mCurrentShape);

        QRectF rect = shape->rect();

        shape->setRect(reverseRect(rect));
    }
    else if(mShapeType == Square)
    {
        UB1HEditableGraphicsSquareItem* shape = dynamic_cast<UB1HEditableGraphicsSquareItem*>(mCurrentShape);

        QRectF rect = shape->rect();

        shape->setRect(reverseRect(rect));
    }
    else if(mShapeType == Ellipse)
    {
        UB3HEditableGraphicsEllipseItem* shape = dynamic_cast<UB3HEditableGraphicsEllipseItem*>(mCurrentShape);

        QRectF rect = shape->rect();

        shape->setRect(reverseRect(rect));
    }
    else if(mShapeType == Circle)
    {
        UB1HEditableGraphicsCircleItem* shape = dynamic_cast<UB1HEditableGraphicsCircleItem*>(mCurrentShape);

        QRectF rect = shape->rect();

        shape->setRect(reverseRect(rect));
    }
    else if(mShapeType == RegularPolygon)
    {
        UBEditableGraphicsRegularShapeItem* shape = dynamic_cast<UBEditableGraphicsRegularShapeItem*>(mCurrentShape);

        QPointF startPoint = shape->correctStartPoint();
        shape->setStartPoint(startPoint);
    }
    else if(mShapeType == Pen)
    {
        UBGraphicsFreehandItem* freehand = dynamic_cast<UBGraphicsFreehandItem*>(mCurrentShape);

        if (freehand)
        {
            if (freehand->path().elementCount() > 2)
            {
                QGraphicsEllipseItem poigneeDepart(freehand->mStartEndPoint[0].x()-10, freehand->mStartEndPoint[0].y()-10, 20, 20);
                if (poigneeDepart.contains(freehand->mStartEndPoint[1]))
                {
                    freehand->addPoint(freehand->mStartEndPoint[0]);
                    freehand->setClosed(true);
                    applyCurrentStyle(mCurrentShape);
                }
                freehand->setIsInCreationMode(false);
            }
        }

        mFirstClickForFreeHand = true;
    }

    if (!mCursorMoved && mCurrentShape && mShapeType != Polygon)
        mBoardView->scene()->removeItem(mCurrentShape);

    if (mShapeType != Polygon)
        mCurrentShape = NULL;

}

QRectF UBShapeFactory::reverseRect(const QRectF& rect)
{
    qreal w = rect.width();
    qreal h = rect.height();

    QRectF reversedRect;
    QPointF p1, p2;

    if(w < 0 && h < 0){
        p1 = rect.bottomRight();
        p2 = rect.topLeft();
    }else if(w > 0 && h < 0){
        p1 = rect.bottomLeft();
        p2 = rect.topRight();
    }else if(w < 0 && h > 0){
        p1 = rect.topRight();
        p2 = rect.bottomLeft();
    }else{
        p1 = rect.topLeft();
        p2 = rect.bottomRight();
    }


    reversedRect.setTopLeft(p1);
    reversedRect.setBottomRight(p2);

    return reversedRect;
}

void UBShapeFactory::desactivate()
{
    mIsPress = false;
    mIsCreating = false;
    mCurrentShape = NULL;
    mShapeType = None;
}

void UBShapeFactory::terminateShape()
{
    //when clicking on stroke and fill subpalettes, creation mode could stay even though the current shape had changed
    if (mShapeType == Polygon)
    {
        UBEditableGraphicsPolygonItem* p = dynamic_cast<UBEditableGraphicsPolygonItem*>(mCurrentShape);
        if (p)
            p->setIsInCreationMode(false);
    }

    // Ends the current shape :
    mCurrentShape = NULL;
}

bool UBShapeFactory::isShape(QGraphicsItem *item)
{
    return item->type() == UBGraphicsItemType::GraphicsShapeItemType
            || item->type() == UBGraphicsItemType::GraphicsPathItemType
            || item->type() == UBGraphicsItemType::GraphicsRegularPathItemType
            || item->type() == UBGraphicsItemType::GraphicsFreehandItemType;
}

void UBShapeFactory::setStrokeStyle(Qt::PenStyle penStyle)
{
    mCurrentPenStyle = penStyle;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBAbstractGraphicsItem * shape = dynamic_cast<UBAbstractGraphicsItem*>(items.at(i));

        if(shape)
        {
            if(penStyle == Qt::CustomDashLine){
                QPen p = shape->pen();
                p.setCapStyle(Qt::RoundCap);
                p.setDashPattern(mDotDashes);
                shape->setPen(p);
            }else{
                shape->setStyle(mCurrentPenStyle);
            }
        }

        items.at(i)->update();
    }
}

void UBShapeFactory::setThickness(int thickness)
{
    mThickness = thickness;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBAbstractGraphicsItem * shape = dynamic_cast<UBAbstractGraphicsItem*>(items.at(i));

        if(shape){
            shape->setStrokeSize(mThickness);
        }

        items.at(i)->update();
    }
}

void UBShapeFactory::setStrokeColor(QColor color)
{
    mCurrentStrokeColor = color;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBAbstractGraphicsItem * shape = dynamic_cast<UBAbstractGraphicsItem*>(items.at(i));

        if(shape)
        {
            shape->setStrokeColor(mCurrentStrokeColor);
        }

        items.at(i)->update();
    }
}

QColor UBShapeFactory::strokeColor()
{
    return mCurrentStrokeColor;
}


void UBShapeFactory::setFillingFirstColor(QColor color)
{
    mCurrentFillFirstColor = color;

    updateFillingPropertyOnSelectedItems();
}

void UBShapeFactory::setFillingSecondColor(QColor color)
{
    mCurrentFillSecondColor = color;

    updateFillingPropertyOnSelectedItems();
}

void UBShapeFactory::updateFillingPropertyOnSelectedItems()
{
    UBGraphicsScene* scene = mBoardView->scene();

     QList<QGraphicsItem*> items = scene->selectedItems();

     for(int i = 0; i < items.size(); i++)
     {
         UBAbstractGraphicsItem * shape = dynamic_cast<UBAbstractGraphicsItem*>(items.at(i));

         if(shape)
         {
             if(shape->hasFillingProperty()){
                 if (mFillType == Gradient)
                 {
                     setGradientFillingProperty(shape);
                 }
                 else if (mFillType == Diag)
                 {
                     shape->setStyle(Qt::TexturePattern);
                     shape->setFillPattern(UBAbstractGraphicsItem::FillPattern_Diag1);
                     shape->setFillColor(mCurrentFillFirstColor);
                 }
                 else if (mFillType == Dense)
                 {
                     shape->setStyle(Qt::TexturePattern);
                     shape->setFillPattern(UBAbstractGraphicsItem::FillPattern_Dot1);
                     shape->setFillColor(mCurrentFillFirstColor);
                 }
                 else
                 {
                     shape->setStyle(mCurrentBrushStyle);
                     shape->setFillColor(mCurrentFillFirstColor);
                 }
             }
         }
         items.at(i)->update();
     }
}

QColor UBShapeFactory::fillFirstColor()
{
    return mCurrentFillFirstColor;
}

QColor UBShapeFactory::fillSecondColor()
{
    return mCurrentFillSecondColor;
}

void UBShapeFactory::desactivateEditionMode(QGraphicsItem *item)
{
    UBAbstractEditable *edit = dynamic_cast<UBAbstractEditable*>(item);

    if(edit)
    {
        edit->deactivateEditionMode();
        item->setSelected(false);
    }
}

bool UBShapeFactory::isInEditMode(QGraphicsItem *item)
{
    UBAbstractEditable *edit = dynamic_cast<UBAbstractEditable*>(item);

    if(edit == 0) return false;

    return edit->isInEditMode();
}

QVector<qreal> UBShapeFactory::dashPattern() const
{
    return mDotDashes;
}
