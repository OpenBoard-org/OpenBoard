#ifndef UBSHAPE_H
#define UBSHAPE_H

#include "UBItem.h"
#include "core/UB.h"

#include <QAbstractGraphicsShapeItem>

class UBAbstractGraphicsItem : public UBItem, public UBGraphicsItem, public QAbstractGraphicsShapeItem
{
public:
    UBAbstractGraphicsItem(QGraphicsItem *parent = 0);

    virtual ~UBAbstractGraphicsItem();

    bool hasFillingProperty() const;

    bool hasStrokeProperty() const;

    bool hasGradient() const;

    void setStyle(Qt::PenStyle penStyle);

    void setStyle(Qt::BrushStyle brushStyle);

    void setStyle(Qt::BrushStyle brushStyle, Qt::PenStyle penStyle);

    void setFillColor(const QColor& color);

    void setStrokeColor(const QColor& color);

    void setStrokeSize(int size);

    enum FillPattern{   // Warning : those values are persisted. Do NOT change this order. Only add new values at the end of enum.
        FillPattern_None,
        FillPattern_Diag1,
        FillPattern_Dot1
    };

    FillPattern fillPattern() const {return mFillPatern;}
    void setFillPattern(FillPattern pattern);

    // UBItem interface
    void setUuid(const QUuid &pUuid);

    void initializeFillingProperty();

    void initializeStrokeProperty();

    //disambiguation from UBGraphicsItem and QabstractGraphicsShapeItem
    virtual int type() const = 0;

    //must be define, because the delegate use it
    virtual QRectF boundingRect() const{ return QRect(); }

    virtual void copyItemParameters(UBItem *copy) const;

protected:
    void setStyle(QPainter *painter);

    QRectF adjustBoundingRect(QRectF rect) const;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    FillPattern mFillPatern;
    QBitmap patternPoint();
    QBitmap patternDiag();
};

#endif // UBSHAPE_H
