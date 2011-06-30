#ifndef UB_ABSTRACTDRAWRULER_H_
#define UB_ABSTRACTDRAWRULER_H_

#include <QtGui>
class UBGraphicsScene;
class QGraphicsSvgItem;

class UBAbstractDrawRuler : public QObject
{
    Q_OBJECT

public:
    UBAbstractDrawRuler();
    ~UBAbstractDrawRuler();

    void create(QGraphicsItem& item);

    virtual void StartLine(const QPointF& position, qreal width);
    virtual void DrawLine(const QPointF& position, qreal width);
    virtual void EndLine();

protected:

    void paint();

    virtual UBGraphicsScene* scene() const = 0;

    virtual void rotateAroundTopLeftOrigin(qreal angle) = 0;

    virtual QPointF topLeftOrigin() const = 0;
    virtual QRectF  resizeButtonRect() const = 0;
    virtual QRectF  closeButtonRect() const = 0;
    virtual QRectF  rotateButtonRect() const = 0;

    void updateResizeCursor(QGraphicsItem &item);

    bool mResizing;
    bool mRotating;
    bool mShowButtons;
    QGraphicsSvgItem* mCloseSvgItem;
    QGraphicsSvgItem* mRotateSvgItem;
    QCursor mResizeCursor;
    qreal mAntiScaleRatio;

    QPointF startDrawPosition;

    QCursor moveCursor() const;
    QCursor resizeCursor() const;
    QCursor rotateCursor() const;
    QCursor closeCursor() const;
    QCursor drawRulerLineCursor() const;

    QColor  drawColor() const;
    QColor  middleFillColor() const;
    QColor  edgeFillColor() const;
    QFont   font() const;

    static const QColor sLightBackgroundEdgeFillColor;
    static const QColor sLightBackgroundMiddleFillColor;
    static const QColor sLightBackgroundDrawColor;
    static const QColor sDarkBackgroundEdgeFillColor;
    static const QColor sDarkBackgroundMiddleFillColor;
    static const QColor sDarkBackgroundDrawColor;

    static const int    sLeftEdgeMargin = 10;
    static const int    sMinLength = 150;
    static const int    sDegreeToQtAngleUnit = 16;
    static const int    sRotationRadius = 15;
    static const int    sPixelsPerMillimeter = 5;
    static const int    sFillTransparency = 127;
    static const int    sDrawTransparency = 192;
    static const int    sRoundingRadius = sLeftEdgeMargin / 2;

};

#endif
