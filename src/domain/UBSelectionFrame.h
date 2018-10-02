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


#ifndef UBSELECTIONFRAME_H
#define UBSELECTIONFRAME_H

#include <QGraphicsRectItem>
#include <QtGui>
#include <core/UB.h>

#include "domain/UBGraphicsScene.h"

class DelegateButton;
class UBGraphicsItemDelegate;

class UBSelectionFrame : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    enum {om_idle, om_moving, om_rotating} mOperationMode;
    enum { Type = UBGraphicsItemType::SelectionFrameType };

    UBSelectionFrame();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;
    QPainterPath shape() const;

    void setLocalBrush(const QBrush &pBrush) {mLocalBrush = pBrush;}
    QBrush localBrush() const {return mLocalBrush;}
//    void setEnclosedItems(const QList<UBGraphicsItemDelegate*> pEncItems) {mEnclosedtems = pEncItems; updateRect();}
    void setEnclosedItems(const QList<QGraphicsItem*> pGraphicsItems);
    void updateRect();
    void updateScale();
    bool isEmpty() const {return this->rect().isEmpty();}
    virtual int type() const {return Type;}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private slots:
    void setAntiScaleRatio(qreal pAntiscaleRatio) {mAntiscaleRatio = pAntiscaleRatio;}
    void onZoomChanged(qreal pZoom);
    void remove();
    void duplicate();
    void increaseZlevelUp();
    void increaseZlevelTop();
    void increaseZlevelDown();
    void increaseZlevelBottom();
    void groupItems();

private:
    void addSelectionUndo(QList<QGraphicsItem*> items, UBZLayerController::moveDestination dest);
    void translateItem(QGraphicsItem *item, const QPointF &translatePoint);
    void placeButtons();
    void placeExceptionButton(DelegateButton *pButton, QTransform pTransform);
    void clearButtons();
    inline int adjThickness() const {return mThickness * mAntiscaleRatio;}
    inline UBGraphicsScene* ubscene();
    void setCursorFromAngle(QString angle);

    QList<QGraphicsItem*> sortedByZ(const QList<QGraphicsItem*> &pItems);
    QList<DelegateButton*> buttonsForFlags(UBGraphicsFlags fls);

    QList<QGraphicsItem*> enclosedGraphicsItems();


private:
    int mThickness;
    qreal mAntiscaleRatio;
    QList<UBGraphicsItemDelegate*> mEnclosedtems;
    QBrush mLocalBrush;

    QPointF mPressedPos;
    QPointF mLastMovedPos;
    QPointF mLastTranslateOffset;
    qreal mRotationAngle;

    bool mIsLocked;

    QList<DelegateButton*> mButtons;

    DelegateButton *mDeleteButton;
    DelegateButton *mDuplicateButton;
    DelegateButton *mZOrderUpButton;
    DelegateButton *mZOrderDownButton;
    DelegateButton *mGroupButton;

    DelegateButton *mRotateButton;

};

#endif // UBSELECTIONFRAME_H
