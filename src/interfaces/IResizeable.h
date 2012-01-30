#ifndef IRESIZEABLE_H
#define IRESIZEABLE_H

#include <QResizeEvent>

class IResizeable
{
public:
    ~IResizeable();

protected:
    virtual void resizeEvent(QResizeEvent* pEvent) = 0;
};

#endif // IRESIZEABLE_H
