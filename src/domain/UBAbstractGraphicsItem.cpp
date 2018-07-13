#include "UBAbstractGraphicsItem.h"
#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"
#include "board/UBDrawingController.h"
#include "customWidgets/UBGraphicsItemAction.h"

UBAbstractGraphicsItem::UBAbstractGraphicsItem(QGraphicsItem *parent):
    QAbstractGraphicsShapeItem(parent)
{

    setDelegate(new UBGraphicsItemDelegate(this,0, GF_REVOLVABLE | GF_DUPLICATION_ENABLED | GF_RESPECT_RATIO));
    // Might be fit in int value under most OS

    /*Delegate()->init();
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);*/
    Delegate()->setCanTrigAnAction(true);
    Delegate()->setHorizontalMirror(true);
    Delegate()->setVerticalMirror(true);

    /*Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::NoResizing);*/

    setUuid(QUuid::createUuid());
    //used for the podcast
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
}


UBAbstractGraphicsItem::~UBAbstractGraphicsItem()
{

}

void UBAbstractGraphicsItem::setStyle(Qt::PenStyle penStyle)
{
    Qt::BrushStyle brushStyle = Qt::NoBrush;
    /*if (hasFillingProperty())*/
        brushStyle = brush().style();

    setStyle(brushStyle, penStyle);
}

void UBAbstractGraphicsItem::setStyle(Qt::BrushStyle brushStyle)
{
    Qt::PenStyle penStyle = Qt::NoPen;
    /*if(hasStrokeProperty())*/
        penStyle = pen().style();

    setStyle(brushStyle, penStyle);
}

void UBAbstractGraphicsItem::setStyle(Qt::BrushStyle brushStyle, Qt::PenStyle penStyle)
{
    /*if(hasFillingProperty()){*/
        QBrush b = brush();
        b.setStyle(brushStyle);
        setBrush(b);
    /*}

    if(hasStrokeProperty()){*/
        QPen p = pen();
        p.setStyle(penStyle);
        setPen(p);
    /*}*/
}

void UBAbstractGraphicsItem::setFillColor(const QColor& color)
{
    /*if(hasFillingProperty()){*/
        QBrush b = brush();
        b.setColor(color);
        setBrush(b);
    /*}*/
}

void UBAbstractGraphicsItem::setStrokeColor(const QColor& color)
{
    /*if(hasStrokeProperty()){*/
        QPen p = pen();
        p.setColor(color);
        setPen(p);
    /*}*/
}

void UBAbstractGraphicsItem::setStrokeSize(int size)
{
    /*if(hasStrokeProperty()){*/
        //qWarning()<<"setStrokeSize";
        QPen p = pen();
        p.setWidth(size);
        setPen(p);
    /*}*/
}

void UBAbstractGraphicsItem::setFillPattern(UBAbstractGraphicsItem::FillPattern pattern)
{
    mFillPatern = pattern;

    /*if (hasFillingProperty())
    {*/
        QBrush b = brush();
        switch (pattern) {
        case FillPattern_Diag1:
            b.setTexture(patternDiag());
            break;
        case FillPattern_Dot1:
            b.setTexture(patternPoint());
            break;
        default:
            break;
        }

        setBrush(b);
    /*}*/
}

void UBAbstractGraphicsItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid));
}

QVariant UBAbstractGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QAbstractGraphicsShapeItem::itemChange(change, newValue);
}

QBitmap UBAbstractGraphicsItem::patternPoint()
{
    int taillePattern = 10;
    QImage img(taillePattern, taillePattern, QImage::Format_Mono);

    img.setColor(0, 0);
    img.setColor(1, 1);

    // Initialize background
    for(int y=0; y<taillePattern; y++)
        for(int x=0; x<taillePattern; x++){
            img.setPixel(x, y, 0);
        }

    // The dot :
    img.setPixel(4, 3, 1);
    img.setPixel(5, 3, 1);
    img.setPixel(6, 3, 1);
    img.setPixel(3, 4, 1);
    img.setPixel(4, 4, 1);
    img.setPixel(5, 4, 1);
    img.setPixel(6, 4, 1);
    img.setPixel(7, 4, 1);
    img.setPixel(3, 5, 1);
    img.setPixel(4, 5, 1);
    img.setPixel(5, 5, 1);
    img.setPixel(6, 5, 1);
    img.setPixel(7, 5, 1);
    img.setPixel(3, 6, 1);
    img.setPixel(4, 6, 1);
    img.setPixel(5, 6, 1);
    img.setPixel(6, 6, 1);
    img.setPixel(7, 6, 1);
    img.setPixel(4, 7, 1);
    img.setPixel(5, 7, 1);
    img.setPixel(6, 7, 1);

    QBitmap bitmap = QBitmap::fromImage(img);

    return bitmap;
}

QBitmap UBAbstractGraphicsItem::patternDiag()
{
    int taillePattern = 20;
    QImage img(taillePattern, taillePattern, QImage::Format_Mono);

    img.setColor(0, 0);
    img.setColor(1, 1);

    // Initialize background :
    for(int y=0; y<taillePattern; y++)
        for(int x=0; x<taillePattern; x++){
            img.setPixel(x, y, 0);
        }

    // The diagonal :
    for(int y=0; y<taillePattern; y++){
        img.setPixel(taillePattern-1-y, y, 1);
        if (y<taillePattern-1){ // except for last line
            img.setPixel(taillePattern-2-y, y, 1);
        }
    }
    img.setPixel(taillePattern-1, taillePattern-1, 1);

    QBitmap bitmap = QBitmap::fromImage(img);

    return bitmap;
}

void UBAbstractGraphicsItem::setStyle(QPainter *painter)
{
    if(hasStrokeProperty()){
        painter->setPen(pen());
    }

    if(hasFillingProperty()){
        painter->setBrush(brush());
    }
}

void UBAbstractGraphicsItem::initializeStrokeProperty()
{
    QPen p(Qt::SolidLine);
    p.setWidth(1);
    p.setColor(Qt::black);
    setPen(p);
}

void UBAbstractGraphicsItem::initializeFillingProperty()
{
    QBrush b(Qt::SolidPattern);
    b.setColor(Qt::black);
    setBrush(b);
}

QRectF UBAbstractGraphicsItem::adjustBoundingRect(QRectF rect) const
{
    if(hasStrokeProperty()){
        int r = pen().width() / 2;

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

bool UBAbstractGraphicsItem::hasFillingProperty() const
{
    return brush() != QBrush();
}

bool UBAbstractGraphicsItem::hasStrokeProperty() const
{
    return pen() != QPen();
}

bool UBAbstractGraphicsItem::hasGradient() const
{
    return brush().gradient() != NULL;
}

void UBAbstractGraphicsItem::copyItemParameters(UBItem *copy) const
{
    UBAbstractGraphicsItem *cp = dynamic_cast<UBAbstractGraphicsItem*>(copy);

    if(!cp) return;

    cp->setPos(this->pos());
    cp->setTransform(this->transform());
    cp->setFlag(QGraphicsItem::ItemIsMovable, true);
    cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
    cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));

    if(Delegate()->action()){
        if(Delegate()->action()->linkType() == eLinkToAudio){
            UBGraphicsItemPlayAudioAction* audioAction = dynamic_cast<UBGraphicsItemPlayAudioAction*>(Delegate()->action());
            UBGraphicsItemPlayAudioAction* action = new UBGraphicsItemPlayAudioAction(audioAction->fullPath());
            cp->Delegate()->setAction(action);            
        }
        else
            cp->Delegate()->setAction(Delegate()->action());

    }

    cp->setBrush(brush());
    cp->setPen(pen());
}
