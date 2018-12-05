/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBMAGNIFIER_H
#define UBMAGNIFIER_H

#include <QtGui>
#include <QWidget>

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
    enum DrawingMode
    {
        circular = 0,
        rectangular,
        modesCount // should me last.
    };

public:
    UBMagnifier(QWidget *parent = 0, bool isInteractive = false);
    ~UBMagnifier();

    void setSize(qreal percentFromScene);
    void createMask();
    void setZoom(qreal zoom);

    void setGrabView(QWidget *view);
    void setMoveView(QWidget *view) {mView = view;}
    void setDrawingMode(int mode);

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
    void magnifierDrawingModeChange_Signal(int mode);
    
public slots:
    void slot_refresh();

private:
    void calculateButtonsPositions();
protected:
    void paintEvent(QPaintEvent *);

    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );

    QPoint mMousePressPos;
    qreal mMousePressDelta;
    bool mShouldMoveWidget;
    bool mShouldResizeWidget;

    int m_iButtonInterval;
    QPixmap *sClosePixmap;
    QRect sClosePixmapButtonRect;
    QPixmap *sIncreasePixmap;
    QRect sIncreasePixmapButtonRect;
    QPixmap *sDecreasePixmap;
    QRect sDecreasePixmapButtonRect;
    QPixmap *sChangeModePixmap;
    QRect sChangeModePixmapButtonRect;
    QPixmap *mResizeItem;
    QRect mResizeItemButtonRect;

    bool isCusrsorAlreadyStored;
    QCursor mOldCursor;
    QCursor mResizeCursor;

private:
    DrawingMode mDrawingMode;

    QTimer mRefreshTimer;
    bool m_isInteractive;

    QPoint updPointGrab;
    QPoint updPointMove;
    
    QPixmap pMap;
    QBitmap bmpMask;
    QPen borderPen;

    QWidget *gView;
    QWidget *mView;
};

#endif // UBMAGNIFIER_H
