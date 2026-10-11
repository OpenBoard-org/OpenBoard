/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#pragma once

#include <QColor>
#include <QGraphicsItem>
#include <QObject>

#include "core/UB.h"
#include "domain/UBItemStyle.h"

#include "ui_shapeActions.h"

class UBAbstractGraphicsItem;
class UBBoardView;
class QMouseEvent;
class UBDrawingController;


class UBShapeFactory : public QObject
{
    Q_OBJECT

public:
    UBShapeFactory();
    void init();

    Ui::ShapeActions* shapeActions() const;

    static bool isShape(QGraphicsItem* item);
    static void desactivateEditionMode(QGraphicsItem* item);
    static bool isInEditMode(QGraphicsItem* item);

    QRectF reverseRect(const QRectF& rect);

    static QList<qreal> dashPattern(Qt::PenStyle style);
    static Qt::PenStyle styleForPattern(QList<qreal> pattern);

    static QString shapeTypeToString(ShapeType shapeType);
    static ShapeType shapeTypeFromString(const QString& shapeTypeString);

public slots:
    void createRegularPolygon(int nVertices);
    void createEllipse(bool create);
    void createPolygon(bool create);
    void createCircle(bool create);
    void createRectangle(bool create);
    void createSquare(bool create);
    void createLine(bool create);

    void onMouseMove(QMouseEvent* event);
    void onMousePress(QMouseEvent* event);
    void onMouseRelease(QMouseEvent* event);

    void desactivate();
    void terminateShape();
    void discardCurrentShape();

    void setCurrentStyle(const UBItemStyle& style);

private:
    UBAbstractGraphicsItem* mCurrentShape{nullptr};
    UBBoardView* mBoardView{nullptr};
    UBItemStyle mShapeStyle{};

    bool mIsCreating{false};
    bool mIsPress{false};
    bool mIsRegularShape{true};

    ShapeType mShapeType{ShapeType::None};

    UBDrawingController* mDrawingController{nullptr};

    int mNVertices{0};

    QRectF mBoundingRect{};

    bool mCursorMoved{false};

    Ui::ShapeActions* mShapeActions{nullptr};

    QSet<UBAbstractGraphicsItem*> mSelectedShapes{};

protected:
    UBAbstractGraphicsItem* instanciateCurrentShape();
};
