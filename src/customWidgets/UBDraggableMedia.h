#ifndef UBDRAGGABLEMEDIA_H
#define UBDRAGGABLEMEDIA_H

#include "UBMediaWidget.h"

class UBDraggableMedia : public UBMediaWidget
{
public:
    UBDraggableMedia(eMediaType type = eMediaType_Video, QWidget* parent=0, const char* name="UBDraggableMedia");
    ~UBDraggableMedia();
protected:
    void mousePressEvent(QMouseEvent* ev);
    void mouseMoveEvent(QMouseEvent* ev);

private:
    QPoint mDragStartPos;
};

#endif // UBDRAGGABLEMEDIA_H
