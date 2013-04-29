/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
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



#ifndef UB_ABSTRACTDRAWRULER_H_
#define UB_ABSTRACTDRAWRULER_H_

#include <QtGui>
#include "frameworks/UBGeometryUtils.h"

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

    signals:
        void hidden();

protected:

    void paint();

    virtual UBGraphicsScene* scene() const = 0;

    virtual void rotateAroundCenter(qreal angle) = 0;

    virtual QPointF rotationCenter() const = 0;
    virtual QRectF closeButtonRect() const = 0;
    virtual void paintGraduations(QPainter *painter) = 0;

    bool mShowButtons;
    QGraphicsSvgItem* mCloseSvgItem;
    qreal mAntiScaleRatio;

    QPointF startDrawPosition;

    QCursor moveCursor() const;
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

    static const int sLeftEdgeMargin;
    static const int sDegreeToQtAngleUnit;
    static const int sRotationRadius;
    static const int sFillTransparency;
    static const int sDrawTransparency;
    static const int sRoundingRadius;
    int sPixelsPerMillimeter;
};

#endif
