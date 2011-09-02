
#ifndef UBMAGNIFIER_H
#define UBMAGNIFIER_H

#include <QtGui>

class UBMagnifierParams
{
public :
    int x;
    int y;
    qreal zoom;
    qreal sizePercentFromScene;
};

class UBMagnifier : public QWidget
{
    Q_OBJECT

public:
    UBMagnifier(QWidget *parent = 0, bool isInteractive = false);
    ~UBMagnifier();

    void setSize(qreal percentFromScene);
    void setZoom(qreal zoom);

    void setGrabView(QWidget *view);
    void setMoveView(QWidget *view) {mView = view;}

    void grabPoint();
    void grabPoint(const QPoint &point);
    void grabNMove(const QPoint &pGrab, const QPoint &pMove, bool needGrab = true, bool needMove = true);

    UBMagnifierParams params;

signals:
    void magnifierMoved_Signal(QPoint newPos);
    void magnifierClose_Signal();
    void magnifierZoomIn_Signal();
    void magnifierZoomOut_Signal();
    void magnifierResized_Signal(qreal newPercentSize);

protected:
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);

    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );

    QPoint mMousePressPos;
    qreal mMousePressDelta;
    bool mShouldMoveWidget;
    bool mShouldResizeWidget;


    QPixmap *sClosePixmap;
    QPixmap *sIncreasePixmap;
    QPixmap *sDecreasePixmap;
    QPixmap *mResizeItem;

    bool isCusrsorAlreadyStored;
    QCursor mOldCursor;
    QCursor mResizeCursor;

private:
    bool inTimer;
    bool m_isInteractive;

    int timerUpdate;
    QPoint updPointGrab;
    QPoint updPointMove;
    
    QPixmap pMap;
    QBitmap bmpMask;
    QPen borderPen;

    QWidget *gView;
    QWidget *mView;
};

#endif // UBMAGNIFIER_H
