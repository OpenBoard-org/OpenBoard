#ifndef UBABSTRACTHANDLE_H
#define UBABSTRACTHANDLE_H

#include <QGraphicsItem>
#include <QObject>

#include <QPainter>

#include "UBItem.h"

class UBAbstractEditable;

class UBAbstractHandle : public QGraphicsItem, public UBItem
{
public:

    virtual ~UBAbstractHandle(){ }

    UBAbstractHandle(UBAbstractHandle* const src);

    void setId(int id)
    {
        mId = id;
    }

    int getId()
    {
        return mId;
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QRectF boundingRect() const;

    void setEditableObject(UBAbstractEditable *eo)
    {
        mEditableObject = eo;
    }

    UBAbstractEditable *editableObject()
    {
        return mEditableObject;
    }

    enum { Type = UBGraphicsItemType::GraphicsHandle };
    virtual int type() const { return Type; }

    int radius() const
    {
        return this->mRadius;
    }

protected:
    UBAbstractHandle();

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    int mId;
    bool mClick;
    int mRadius;

    UBAbstractEditable *mEditableObject;
};

#endif // UBABSTRACTHANDLE_H
