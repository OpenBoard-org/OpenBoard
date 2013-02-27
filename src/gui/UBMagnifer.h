/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



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
    
public slots:
    void slot_refresh();

protected:
    void paintEvent(QPaintEvent *);

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
