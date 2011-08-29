
#ifndef UBMAGNIFER_H
#define UBMAGNIFER_H

#include <QWidget>
#include <QBitmap>
#include <QPen>

class QPixmap;
class QBitmap;
class QPen;
class QGraphicsView;

class UBMagnifer : public QWidget
{
    Q_OBJECT

public:
    UBMagnifer(QWidget *parent = 0);
    ~UBMagnifer();

    void setSize(int percentFromScene);
    void setZoom(int z);

    void setGrabView(QGraphicsView *view);
    void setMoveView(QGraphicsView *view) {mView = view;}

    void grabPoint(const QPoint &point);
    void grabPoint(const QPointF &point);
    void grabNMove(const QPoint &point, bool needGrab);
    void grabNMove(const QPointF &point, bool needGrab);

protected:
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);

    int sizePercentFromScene;


private:
    int zoom;
    int zWidth;
    int zHeight;
    int zWidthHalf;
    int zHeightHalf;
    bool inTimer;

    int timerUpdate;
    QPoint updPoint;
    QPointF updPointF;
    
    QPixmap pMap;
    QBitmap bmpMask;
    QPen borderPen;

    QGraphicsView *gView;
    QGraphicsView *mView;
};

#endif // UBMAGNIFER_H
