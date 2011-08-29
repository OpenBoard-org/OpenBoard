
#include "UBMagnifer.h"
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QWidget>
#include <QGraphicsView>
#include <QTimerEvent>
#include <QBitmap>
#include <QPen>

UBMagnifer::UBMagnifer(QWidget *parent) 
    : QWidget(parent)
    , gView(0)
    , mView(0)
    , sizePercentFromScene(20)
    , inTimer(false)
    , borderPen(Qt::darkGray)
{
    borderPen.setWidth(2);
}

UBMagnifer::~UBMagnifer()
{
    if (timerUpdate != 0)
    {
        killTimer(timerUpdate);
        timerUpdate = 0;
    }
}

void UBMagnifer::setSize(int percentFromScene) 
{
    if(gView == NULL) return;

    // calculate object size
    sizePercentFromScene = percentFromScene;
    QSize sceneSize = gView->size();
    int size = sizePercentFromScene * sceneSize.width() / 100;

    setGeometry(0, 0, size, size);

    // prepare transparent bit mask
    QImage mask_img(width(), height(), QImage::Format_Mono);
    mask_img.fill(0xff);
    QPainter mask_ptr(&mask_img);
    mask_ptr.setBrush( QBrush( QColor(0, 0, 0) ) );
    mask_ptr.drawEllipse(QPoint(size/2, size/2), size/2, size/2);
    bmpMask = QBitmap::fromImage(mask_img);

    // prepare general image
    pMap = QPixmap(width(), height());
    pMap.fill(Qt::transparent);
    pMap.setMask(bmpMask);
}

void UBMagnifer::setZoom(int z) 
{
    zoom = z;

    zWidth = size().width() / zoom;
    zWidthHalf = zWidth / 2;
    zHeight = size().height() / zoom;
    zHeightHalf = zHeight / 2;
}

void UBMagnifer::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, pMap);

    // in future replace to image border
    QPen pen(Qt::darkGray);
    pen.setWidth(2);
    p.setPen(borderPen);
    p.drawEllipse(QPoint( size().width() / 2, size().height() / 2), size().width() / 2, size().height() / 2);
}

void UBMagnifer::timerEvent(QTimerEvent *e)
{
    if(inTimer) return;
    if (e->timerId() == timerUpdate)
    {
        inTimer = true;
        if(!(updPoint.isNull())) grabPoint(updPoint);
        if(!(updPointF.isNull()))grabPoint(updPointF);
        inTimer = false;
    }
}

void UBMagnifer::grabPoint(const QPoint &point)
{
    QPointF itemPos = gView->mapFromGlobal(point);

    int x = itemPos.x() - zWidthHalf;
    int y = itemPos.y() - zHeightHalf;

    QPixmap newPixMap(QSize(zWidth,zHeight));
    ((QWidget*)gView)->render(&newPixMap, QPoint(0, 0), QRegion(x, y, zWidth, zHeight));
    pMap.fill(Qt::transparent);
    pMap = newPixMap.scaled(QSize(width(), height()));
    pMap.setMask(bmpMask);

    update();
}

void UBMagnifer::grabPoint(const QPointF &point)
{
    QPoint itemPos = gView->mapFromScene(point);

    int x = itemPos.x() - zWidthHalf;
    int y = itemPos.y() - zHeightHalf;

    QPixmap newPixMap(QSize(zWidth,zHeight));
    ((QWidget*)gView)->render(&newPixMap, QPoint(0, 0), QRegion(x, y, zWidth, zHeight));
    pMap.fill(Qt::transparent);
    pMap = newPixMap.scaled(QSize(width(), height()));
    pMap.setMask(bmpMask);

    update();
}


// from global
void UBMagnifer::grabNMove(const QPoint &point, bool needGrab = true)
{
    updPoint = point;

    if(needGrab)
        grabPoint(point);
    QPoint itemPos = mView->mapFromGlobal(point);
    move(itemPos.x() - width()/2, itemPos.y() - height()/2);
//    move(itemPos.x(), itemPos.y());
}

// from scene
void UBMagnifer::grabNMove(const QPointF &point, bool needGrab = true)
{
    updPointF = point;

    if(needGrab)
        grabPoint(point);
    QPoint itemPos = mView->mapFromScene(point);
    move(itemPos.x() - width()/2, itemPos.y() - height()/2);
//    move(itemPos.x(), itemPos.y());
}

void UBMagnifer::setGrabView(QGraphicsView *view)
{
    if (timerUpdate != 0)
        killTimer(timerUpdate);
    gView = view;
    timerUpdate = startTimer(200);
}

