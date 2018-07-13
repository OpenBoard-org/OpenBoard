#include "UBEditableGraphicsPolygonItem.h"

#include <customWidgets/UBGraphicsItemAction.h>
#include "UBFreeHandle.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "UBGraphicsScene.h"

UBEditableGraphicsPolygonItem::UBEditableGraphicsPolygonItem(QGraphicsItem* parent)
    : UBAbstractEditableGraphicsPathItem(parent)
    , mClosed(false)
    , mOpened(false)
    , HANDLE_SIZE(20)
    , mIsInCreationMode(true)
{
    /*Delegate()->setCanReturnInCreationMode(true);*/
    initializeStrokeProperty();
    initializeFillingProperty();
}

UBEditableGraphicsPolygonItem::~UBEditableGraphicsPolygonItem()
{

}

void UBEditableGraphicsPolygonItem::addPoint(const QPointF & point)
{
    prepareGeometryChange();

    QPointF p(mapFromScene(point));

    QPainterPath painterPath = path();

    if (painterPath.elementCount() == 0)
    {
        painterPath.moveTo(p); // For the first point added, we must use moveTo().
        setPath(painterPath);

        mStartEndPoint[0] = p;
    }
    else
    {
        // If clic on first point, close the polygon
        // TODO à terme : utiliser la surface de la première poignée.
        QPointF pointDepart(painterPath.elementAt(0).x, painterPath.elementAt(0).y);
        QPointF pointFin(painterPath.elementAt(painterPath.elementCount()-1).x, painterPath.elementAt(painterPath.elementCount()-1).y);


        QGraphicsEllipseItem poigneeDepart(pointDepart.x()-10, pointDepart.y()-10, 20, 20);
        QGraphicsEllipseItem poigneeFin(pointFin.x()-10, pointFin.y()-10, 20, 20);

        if (poigneeDepart.contains(p))
        {
            setClosed(true);
        }
        else
        {
            if(poigneeFin.contains(p)){
                mIsInCreationMode = false;
                mOpened = true;
            }else{
                painterPath.lineTo(p);
                setPath(painterPath);
            }
        }

        mStartEndPoint[1] = p;
    }

    if(!mClosed && !mOpened){

        UBFreeHandle *handle = new UBFreeHandle();

        addHandle(handle);

        handle->setParentItem(this);
        handle->setEditableObject(this);
        handle->setPos(p);
        handle->setId(path().elementCount()-1);
        handle->hide();
    }
}

void UBEditableGraphicsPolygonItem::reopen()
{
    if (mClosed)
    {
        QPainterPath::Element firstElement = path().elementAt(0);

        QPointF firstPoint(firstElement.x, firstElement.y);

        QPainterPath newPainterPath(firstPoint);
        int nbElement = path().elementCount() -1;
        for(int iElement=1; iElement < nbElement; iElement++)
        {
            newPainterPath.lineTo(path().elementAt(iElement));
        }

        setPath(newPainterPath);
        mStartEndPoint[1] = QPointF(path().elementAt(path().elementCount()-1).x, path().elementAt(path().elementCount()-1).y);
        setClosed(false);
    }
}

void UBEditableGraphicsPolygonItem::setIsInCreationMode(bool mode)
{
    mIsInCreationMode = mode;
}

void UBEditableGraphicsPolygonItem::setOpened(bool opened)
{
    mOpened = opened;
}

void UBEditableGraphicsPolygonItem::setClosed(bool closed)
{
    mClosed = closed;

    QPainterPath painterPath = path();
    if (closed)
    {
        painterPath.closeSubpath(); // Automatically add a last point, identic to the first point.
    }
    else
    {
        // if last point and first point are the same, remove the last one, in order to open the path.
        int nbElements = painterPath.elementCount();
        if ( nbElements > 1)
        {
            QPainterPath::Element firstElement = painterPath.elementAt(0);
            QPainterPath::Element lastElement = painterPath.elementAt(nbElements - 1);

            QPointF firstPoint(firstElement.x, firstElement.y);
            QPointF lastPoint(lastElement.x, lastElement.y);

            if (firstPoint == lastPoint)
            {
                // Rebuild the path, excluding the last point.
                QPainterPath newPainterPath(firstPoint);
                for(int iElement=1; iElement<nbElements - 1; iElement++)
                {
                    newPainterPath.lineTo(painterPath.elementAt(iElement));
                }
                painterPath = newPainterPath;
            }
        }
    }

    setPath(painterPath);
    mIsInCreationMode = false;
}


UBItem *UBEditableGraphicsPolygonItem::deepCopy() const
{
    UBEditableGraphicsPolygonItem * copy = new UBEditableGraphicsPolygonItem();

    copyItemParameters(copy);

    return copy;
}

void UBEditableGraphicsPolygonItem::copyItemParameters(UBItem *copy) const
{
    UBAbstractEditableGraphicsPathItem::copyItemParameters(copy);

    UBEditableGraphicsPolygonItem *cp = dynamic_cast<UBEditableGraphicsPolygonItem*>(copy);

    if(cp){
        for(int i = 0; i < mHandles.size(); i++){
            UBFreeHandle *handle = new UBFreeHandle();

            handle->setParentItem(cp);
            handle->setEditableObject(cp);
            handle->setPos(mHandles.at(i)->pos());
            handle->setId(mHandles.at(i)->getId());
            handle->hide();

            cp->mHandles.push_back(handle);
        }

        cp->mIsInCreationMode = mIsInCreationMode;
        cp->mClosed = mClosed;
        cp->mStartEndPoint[0] = mStartEndPoint[0];
        cp->mStartEndPoint[1] = mStartEndPoint[1];
    }
}

QRectF UBEditableGraphicsPolygonItem::boundingRect() const
{
    QRectF rect = UBAbstractEditableGraphicsPathItem::boundingRect();

    int enlarge = 0;

    if (mIsInCreationMode)//gérer les poignées aux extrémités
        enlarge += HANDLE_SIZE/2;

    rect.adjust(-enlarge, -enlarge, enlarge, enlarge);

    return rect;
}

void UBEditableGraphicsPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if(this->isClosed())
        painter->setBrush(brush());

    painter->setPen(pen());

    if (isClosed())
    {
        painter->fillPath(path(), painter->brush());
    }

    painter->drawPath(path());

    if (!isClosed())
    {
        QPen penHandles;
        penHandles.setWidth(1);
        penHandles.setColor(Qt::black);
        penHandles.setStyle(Qt::SolidLine);
        painter->setPen(penHandles);

        int hsize = HANDLE_SIZE/2;

        if (mIsInCreationMode)
        {
            painter->drawEllipse(mStartEndPoint[0].x() - hsize, mStartEndPoint[0].y() - hsize, HANDLE_SIZE, HANDLE_SIZE);

            if(path().elementCount() >= 2)
                painter->drawEllipse(mStartEndPoint[1].x() - hsize, mStartEndPoint[1].y() - hsize, HANDLE_SIZE, HANDLE_SIZE);
        }
    }

    drawArrows();
}

void UBEditableGraphicsPolygonItem::updateHandle(UBAbstractHandle *handle)
{
    prepareGeometryChange();

    int id = handle->getId();

    QPainterPath oldPath = path();

    QPainterPath newPath;

    if(mClosed && id == 0){
        newPath.moveTo(handle->pos());
        for(int i = 1; i < oldPath.elementCount()-1; i++){
            newPath.lineTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
        }
        newPath.lineTo(handle->pos());
    }else{
        for(int i = 0; i < oldPath.elementCount(); i++){
            if(i == 0){
                if(i == id){
                    newPath.moveTo(handle->pos());
                }else{
                    newPath.moveTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
                }
            }else{
                if(i == id){
                    newPath.lineTo(handle->pos());
                }else{
                    newPath.lineTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
                }
            }
        }
    }

    setPath(newPath);

    if(hasGradient()){
        QLinearGradient g(path().boundingRect().topLeft(), path().boundingRect().topRight());

        g.setColorAt(0, brush().gradient()->stops().at(0).second);
        g.setColorAt(1, brush().gradient()->stops().at(1).second);

        setBrush(g);
    }

    mStartEndPoint[0] = QPointF(path().elementAt(0).x, path().elementAt(0).y);
    mStartEndPoint[1] = QPointF(path().elementAt(path().elementCount()-1).x, path().elementAt(path().elementCount()-1).y);
    update();
}

QPainterPath UBEditableGraphicsPolygonItem::shape() const
{
    QPainterPath path;
    if(isInEditMode()){
        path.addRect(boundingRect());
    }else{
        path = this->path();
    }

    return path;
}
