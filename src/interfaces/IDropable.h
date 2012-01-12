#ifndef IDROPABLE_H
#define IDROPABLE_H

#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>

class IDropable
{
public:
    virtual ~IDropable(){}

protected:
    virtual void dropEvent(QDropEvent* pEvent) = 0;
    virtual void dragEnterEvent(QDragEnterEvent* pEvent) = 0;
    virtual void dragMoveEvent(QDragMoveEvent* pEvent) = 0;
    virtual void dragLeaveEvent(QDragLeaveEvent* pEvent) = 0;
};

#endif // IDROPABLE_H
