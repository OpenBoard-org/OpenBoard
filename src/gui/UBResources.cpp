
#include "UBResources.h"

#include <QtGui>

#include "core/UBApplication.h"

UBResources* UBResources::sSingleton = 0;

UBResources::UBResources(QObject* pParent)
 : QObject(pParent)
{
    // NOOP
}

UBResources::~UBResources()
{
    // NOOP
}

UBResources* UBResources::resources()
{
    if (!sSingleton)
    {
        sSingleton = new UBResources(UBApplication::staticMemoryCleaner);
        sSingleton->init();
    }

    return sSingleton;

}

void UBResources::init()
{
    // Cursors
    penCursor     = QCursor(Qt::CrossCursor);
    eraserCursor  = QCursor(QPixmap(":/images/cursors/eraser.png"), 21, 21);
    markerCursor  = QCursor(QPixmap(":/images/cursors/marker.png"), 3, 30);
    pointerCursor = QCursor(QPixmap(":/images/cursors/laser.png"), 2, 1);
    handCursor    = QCursor(Qt::OpenHandCursor);
    zoomInCursor  = QCursor(QPixmap(":/images/cursors/zoomIn.png"), 9, 9);
    zoomOutCursor = QCursor(QPixmap(":/images/cursors/zoomOut.png"), 9, 9);
    arrowCursor   = QCursor(Qt::ArrowCursor);
    textCursor    = QCursor(Qt::ArrowCursor);
    rotateCursor  = QCursor(QPixmap(":/images/cursors/rotate.png"), 16, 16);
}
