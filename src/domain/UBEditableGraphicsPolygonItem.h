#ifndef UBGRAPHICSPATHITEM_H
#define UBGRAPHICSPATHITEM_H

#include "UBAbstractEditableGraphicsPathItem.h"

class UBEditableGraphicsPolygonItem : public UBAbstractEditableGraphicsPathItem
{
public:
    UBEditableGraphicsPolygonItem(QGraphicsItem* parent = 0);
    ~UBEditableGraphicsPolygonItem();

    virtual void addPoint(const QPointF &point);
    inline bool isClosed() const {return mClosed;}
    inline void setClosed(bool closed);

    inline bool isOpened() const{ return mOpened; }
    void setOpened(bool opened);

    void reopen();

    void setIsInCreationMode(bool mode);

    // UBItem interface
    UBItem *deepCopy() const;
    void copyItemParameters(UBItem *copy) const;

    // QGraphicsItem interface
    enum { Type = UBGraphicsItemType::GraphicsPathItemType };
    virtual int type() const { return Type; }
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    virtual void updateHandle(UBAbstractHandle *handle);

private:
    bool mClosed;
    bool mOpened;
    bool mIsInCreationMode;

    QPointF mStartEndPoint[2];

    int HANDLE_SIZE; //in pixel
};

#endif // UBGRAPHICSPATHITEM_H
