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


#include "UBShapeFactory.h"

#include "UB1HEditableGraphicsCircleItem.h"
#include "UB1HEditableGraphicsSquareItem.h"
#include "UB3HEditableGraphicsEllipseItem.h"
#include "UB3HEditableGraphicsRectItem.h"
#include "UBEditableGraphicsLineItem.h"
#include "UBEditableGraphicsPolygonItem.h"
#include "UBEditableGraphicsRegularShapeItem.h"

#include "adaptors/UBSvgShapeAdaptor.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"

#include "core/UBApplication.h"
#include "core/UBShortcutManager.h"

#include "domain/UBGraphicsItemUndoCommand.h"
#include "domain/UBGraphicsScene.h"

#include "gui/UBMainWindow.h"

namespace {
static QMap<Qt::PenStyle, QList<qreal>> sPatterns{};
}

UBShapeFactory::UBShapeFactory()
{
    mShapeActions = new Ui::ShapeActions;
    QWidget* actionWidget = new QWidget();
    mShapeActions->setupUi(actionWidget);

    UBSvgShapeAdaptor::registerExtension();

    // shortcuts
    UBShortcutManager::shortcutManager()->addActions(
                UBShortcutManager::tr("Stylus Palette"), {
                    mShapeActions->actionSmartLine,
                    mShapeActions->actionPolygon,
                    mShapeActions->actionCircle,
                    mShapeActions->actionEllipse,
                    mShapeActions->actionSquare,
                    mShapeActions->actionRectangle,
                    mShapeActions->actionRegularPolygon
                }, UBApplication::mainWindow);
}

void UBShapeFactory::init()
{
    mBoardView = UBApplication::boardController->controlView();
    mDrawingController = UBDrawingController::drawingController();

    sPatterns[Qt::SolidLine] = {};
    sPatterns[Qt::DashLine] = {4, 3};
    sPatterns[Qt::DotLine] = {0.1, 2.5};

    connect(mBoardView, &UBBoardView::mouseMove, this, &UBShapeFactory::onMouseMove);
    connect(mBoardView, &UBBoardView::mouseRelease, this, &UBShapeFactory::onMouseRelease);
    connect(mBoardView, &UBBoardView::mousePress, this, &UBShapeFactory::onMousePress);
}

Ui::ShapeActions* UBShapeFactory::shapeActions() const
{
    return mShapeActions;
}

UBAbstractGraphicsItem* UBShapeFactory::instanciateCurrentShape()
{
    switch (mShapeType)
    {
    case ShapeType::Ellipse:
        mCurrentShape = new UB3HEditableGraphicsEllipseItem();
        break;
    case ShapeType::Circle:
        mCurrentShape = new UB1HEditableGraphicsCircleItem();
        break;
    case ShapeType::Rectangle:
        mCurrentShape = new UB3HEditableGraphicsRectItem();
        break;
    case ShapeType::Square:
        mCurrentShape = new UB1HEditableGraphicsSquareItem();
        break;
    case ShapeType::Line:
        mCurrentShape = new UBEditableGraphicsLineItem();
        break;
    case ShapeType::Polygon:
        mCurrentShape = new UBEditableGraphicsPolygonItem();
        break;
    case ShapeType::RegularPolygon:
        mCurrentShape = new UBEditableGraphicsRegularShapeItem(mNVertices);
        break;
    default:
        break;
    }

    mCurrentShape->applyItemStyle(mShapeStyle, UBApplication::boardController->activeScene()->isDarkBackground());

    return mCurrentShape;
}

void UBShapeFactory::createEllipse(bool create)
{
    if (create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = ShapeType::Ellipse;
    }
}

void UBShapeFactory::createCircle(bool create)
{
    if (create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = ShapeType::Circle;
    }
}

void UBShapeFactory::createRectangle(bool create)
{
    if (create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = ShapeType::Rectangle;
    }
}

void UBShapeFactory::createSquare(bool create)
{
    if (create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = ShapeType::Square;
    }
}

void UBShapeFactory::createLine(bool create)
{
    if (create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = ShapeType::Line;
    }
}

void UBShapeFactory::createRegularPolygon(int nVertices)
{
    mDrawingController->setStylusTool(UBStylusTool::Drawing);
    mIsRegularShape = false;
    mIsCreating = true;
    mShapeType = ShapeType::RegularPolygon;
    mNVertices = nVertices;
}

void UBShapeFactory::createPolygon(bool create)
{
    if (create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = false;
        mIsCreating = true;
        mShapeType = ShapeType::Polygon;
    }
}

void UBShapeFactory::onMouseMove(QMouseEvent* event)
{
    if (mIsCreating && mIsPress)
    {
        mCursorMoved = true;
        QPointF cursorPosition = mBoardView->mapToScene(event->pos());

        if (mBoardView->scene()->isSnapping())
        {
            std::optional<QPointF> altPosition;
            QPointF lineStartPoint;

            if (mShapeType == ShapeType::Line)
            {
                const auto step = UBSettings::settings()->rotationAngleStep->get().toDouble();
                UBEditableGraphicsLineItem* line = dynamic_cast<UBEditableGraphicsLineItem*>(mCurrentShape);
                lineStartPoint = line->startPoint();
                QLineF radius(lineStartPoint, cursorPosition);
                auto angle = radius.angle();
                angle = qRound(angle / step) * step;
                radius.setAngle(angle);
                altPosition = radius.p2();
            }

            QPointF gridSnapPoint;
            cursorPosition += mBoardView->scene()->snap(cursorPosition, nullptr, altPosition, &gridSnapPoint);

            if (mShapeType == ShapeType::Line)
            {
                if (cursorPosition != gridSnapPoint)
                {
                    const auto angle1 = QLineF{lineStartPoint, cursorPosition}.angle();
                    const auto angle2 = QLineF{lineStartPoint, gridSnapPoint}.angle();

                    if (std::fmod(std::fabs(angle1 - angle2), 360.) < 0.01)
                    {
                        cursorPosition = gridSnapPoint;
                    }
                }
            }
        }

        if (mIsRegularShape)
        {
            if (mShapeType == ShapeType::Ellipse)
            {
                UB3HEditableGraphicsEllipseItem* shape = dynamic_cast<UB3HEditableGraphicsEllipseItem*>(mCurrentShape);
                QRectF rect = QRectF(shape->pos(), cursorPosition);

                shape->setRadiusX(rect.width() / 2);
                shape->setRadiusY(rect.height() / 2);
            }
            else if (mShapeType == ShapeType::Circle)
            {
                UB1HEditableGraphicsCircleItem* shape = dynamic_cast<UB1HEditableGraphicsCircleItem*>(mCurrentShape);

                shape->setRect(QRectF(shape->pos(), cursorPosition));
            }
            else if (mShapeType == ShapeType::Rectangle)
            {
                UB3HEditableGraphicsRectItem* shape = dynamic_cast<UB3HEditableGraphicsRectItem*>(mCurrentShape);

                shape->setRect(QRectF(shape->pos(), cursorPosition));
            }
            else if (mShapeType == ShapeType::Square)
            {
                UB1HEditableGraphicsSquareItem* shape = dynamic_cast<UB1HEditableGraphicsSquareItem*>(mCurrentShape);

                shape->setRect(QRectF(shape->pos(), cursorPosition));
            }
            else if (mShapeType == ShapeType::Line)
            {
                UBEditableGraphicsLineItem* line = dynamic_cast<UBEditableGraphicsLineItem*>(mCurrentShape);

                line->setEndPoint(cursorPosition);

                QLineF radius(line->startPoint(), cursorPosition);
                auto angle = radius.angle();
                QLineF viewRadius{UBApplication::boardController->controlView()->mapFromScene(radius.p1()),
                                  UBApplication::boardController->controlView()->mapFromScene(radius.p2())};
                QPoint offset = -viewRadius.p2().toPoint();
                viewRadius.setLength(viewRadius.length() + 30);
                offset += viewRadius.p2().toPoint();
                UBApplication::boardController->setCursorFromAngle(angle, offset);
            }
        }
        else
        {
            if (mShapeType == ShapeType::RegularPolygon)
            {
                UBEditableGraphicsRegularShapeItem* regularPathItem =
                    dynamic_cast<UBEditableGraphicsRegularShapeItem*>(mCurrentShape);
                regularPathItem->updatePath(cursorPosition);
                mBoundingRect = regularPathItem->boundingRect();
            }
        }
    }
}

void UBShapeFactory::onMousePress(QMouseEvent* event)
{
    if (mIsCreating)
    {
        mCursorMoved = false;
        mIsPress = true;

        QPointF cursorPosition = mBoardView->mapToScene(event->pos());

        if (mBoardView->scene()->isSnapping())
        {
            cursorPosition += mBoardView->scene()->snap(cursorPosition);
        }

        if (mIsRegularShape)
        {
            if (mShapeType == ShapeType::Ellipse)
            {
                UB3HEditableGraphicsEllipseItem* ellipse =
                    dynamic_cast<UB3HEditableGraphicsEllipseItem*>(instanciateCurrentShape());
                ellipse->setPos(cursorPosition);

                mBoardView->scene()->addItem(ellipse);
            }
            else if (mShapeType == ShapeType::Circle)
            {
                UB1HEditableGraphicsCircleItem* ellipse =
                    dynamic_cast<UB1HEditableGraphicsCircleItem*>(instanciateCurrentShape());
                ellipse->setPos(cursorPosition);

                mBoardView->scene()->addItem(ellipse);
            }
            else if (mShapeType == ShapeType::Rectangle)
            {
                UB3HEditableGraphicsRectItem* rect =
                    dynamic_cast<UB3HEditableGraphicsRectItem*>(instanciateCurrentShape());

                rect->setRect(QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0));

                mBoardView->scene()->addItem(rect);
            }
            else if (mShapeType == ShapeType::Square)
            {
                UB1HEditableGraphicsSquareItem* rect =
                    dynamic_cast<UB1HEditableGraphicsSquareItem*>(instanciateCurrentShape());

                rect->setRect(QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0));

                mBoardView->scene()->addItem(rect);
            }
            else if (mShapeType == ShapeType::Line)
            {
                UBEditableGraphicsLineItem* line = dynamic_cast<UBEditableGraphicsLineItem*>(instanciateCurrentShape());

                line->setLine(cursorPosition, cursorPosition);

                mBoardView->scene()->addItem(line);
            }
        }
        else
        {
            if (mShapeType == ShapeType::RegularPolygon)
            {
                UBEditableGraphicsRegularShapeItem* regularPathItem =
                    dynamic_cast<UBEditableGraphicsRegularShapeItem*>(instanciateCurrentShape());

                regularPathItem->setStartPoint(cursorPosition);

                mBoardView->scene()->addItem(regularPathItem);
            }
            else // Polygon
            {
                UBEditableGraphicsPolygonItem* pathItem = dynamic_cast<UBEditableGraphicsPolygonItem*>(mCurrentShape);
                if (mCurrentShape == NULL || pathItem == NULL)
                {
                    pathItem = dynamic_cast<UBEditableGraphicsPolygonItem*>(instanciateCurrentShape());
                    mBoardView->scene()->addItem(pathItem);
                }

                pathItem->addPoint(cursorPosition);


                if (pathItem->isClosed() || pathItem->isOpened())
                {
                    if (pathItem->path().elementCount() < 2)
                    {
                        discardCurrentShape();
                    }
                    else
                    {
                        terminateShape();
                    }
                }
            }
        }
    }
}

void UBShapeFactory::onMouseRelease(QMouseEvent* event)
{
    Q_UNUSED(event);
    mIsPress = false;

    UBEditableGraphicsLineItem* line = dynamic_cast<UBEditableGraphicsLineItem*>(mCurrentShape);

    if (line)
    {
        if (line->startPoint() == line->endPoint())
        {
            discardCurrentShape();
        }
    }
    else if (mShapeType == ShapeType::Rectangle)
    {
        UB3HEditableGraphicsRectItem* shape = dynamic_cast<UB3HEditableGraphicsRectItem*>(mCurrentShape);

        QRectF rect = shape->rect();

        shape->setRect(reverseRect(rect));
    }
    else if (mShapeType == ShapeType::Square)
    {
        UB1HEditableGraphicsSquareItem* shape = dynamic_cast<UB1HEditableGraphicsSquareItem*>(mCurrentShape);

        QRectF rect = shape->rect();

        shape->setRect(reverseRect(rect));
    }
    else if (mShapeType == ShapeType::Ellipse)
    {
        UB3HEditableGraphicsEllipseItem* shape = dynamic_cast<UB3HEditableGraphicsEllipseItem*>(mCurrentShape);

        QRectF rect = shape->rect();

        shape->setRect(reverseRect(rect));
    }
    else if (mShapeType == ShapeType::Circle)
    {
        UB1HEditableGraphicsCircleItem* shape = dynamic_cast<UB1HEditableGraphicsCircleItem*>(mCurrentShape);

        QRectF rect = shape->rect();

        shape->setRect(reverseRect(rect));
    }
    else if (mShapeType == ShapeType::RegularPolygon)
    {
        UBEditableGraphicsRegularShapeItem* shape = dynamic_cast<UBEditableGraphicsRegularShapeItem*>(mCurrentShape);

        QPointF startPoint = shape->correctStartPoint();
        shape->setStartPoint(startPoint);
    }

    if (mCurrentShape)
    {
        mCurrentShape->applyItemStyle(mShapeStyle, mBoardView->scene()->isDarkBackground());
    }

    if (!mCursorMoved && mCurrentShape && mShapeType != ShapeType::Polygon)
    {
        discardCurrentShape();
    }

    if (mShapeType != ShapeType::Polygon)
        terminateShape();
}

QRectF UBShapeFactory::reverseRect(const QRectF& rect)
{
    qreal w = rect.width();
    qreal h = rect.height();

    QRectF reversedRect;
    QPointF p1, p2;

    if (w < 0 && h < 0)
    {
        p1 = rect.bottomRight();
        p2 = rect.topLeft();
    }
    else if (w > 0 && h < 0)
    {
        p1 = rect.bottomLeft();
        p2 = rect.topRight();
    }
    else if (w < 0 && h > 0)
    {
        p1 = rect.topRight();
        p2 = rect.bottomLeft();
    }
    else
    {
        p1 = rect.topLeft();
        p2 = rect.bottomRight();
    }


    reversedRect.setTopLeft(p1);
    reversedRect.setBottomRight(p2);

    return reversedRect;
}

void UBShapeFactory::desactivate()
{
    mIsPress = false;
    mIsCreating = false;
    mCurrentShape = NULL;
    mShapeType = ShapeType::None;
}

void UBShapeFactory::discardCurrentShape()
{
    // the scene keeps removed items for a later deletion: unregister the shape before deleting it here
    mBoardView->scene()->removeItem(mCurrentShape);
    mBoardView->scene()->removeItemFromDeletion(mCurrentShape);
    delete mCurrentShape;
    mCurrentShape = nullptr;
}

void UBShapeFactory::terminateShape()
{
    if (!mCurrentShape)
    {
        return;
    }

    if (mShapeType == ShapeType::Polygon)
    {
        UBEditableGraphicsPolygonItem* p = dynamic_cast<UBEditableGraphicsPolygonItem*>(mCurrentShape);
        if (p)
        {
            p->setIsInCreationMode(false);
            p->setOpened(true);
        }
    }

    // If shape is not part of the scene, then delete it
    if (!mCurrentShape->QGraphicsItem::scene())
    {
        delete mCurrentShape;
    }
    else
    {
        // commit an undo step for the shape
        UBGraphicsItemUndoCommand* uc = new UBGraphicsItemUndoCommand(mCurrentShape->scene(), nullptr, mCurrentShape);
        UBApplication::undoStack->push(uc);
    }

    // Ends the current shape :
    mCurrentShape = nullptr;
}

bool UBShapeFactory::isShape(QGraphicsItem* item)
{
    return item->type() == UBGraphicsItemType::GraphicsShapeItemType ||
           item->type() == UBGraphicsItemType::GraphicsPathItemType ||
           item->type() == UBGraphicsItemType::GraphicsRegularPathItemType;
}

void UBShapeFactory::setCurrentStyle(const UBItemStyle& style)
{
    mShapeStyle = style;
}

void UBShapeFactory::desactivateEditionMode(QGraphicsItem* item)
{
    UBAbstractEditable* edit = dynamic_cast<UBAbstractEditable*>(item);

    if (edit)
    {
        edit->deactivateEditionMode();
        item->setSelected(false);
    }
}

bool UBShapeFactory::isInEditMode(QGraphicsItem* item)
{
    UBAbstractEditable* edit = dynamic_cast<UBAbstractEditable*>(item);

    if (edit == 0)
        return false;

    return edit->isInEditMode();
}

QList<qreal> UBShapeFactory::dashPattern(Qt::PenStyle style)
{
    return sPatterns.value(style, {});
}

Qt::PenStyle UBShapeFactory::styleForPattern(QList<qreal> pattern)
{
    for (auto entry = sPatterns.begin(); entry != sPatterns.end(); ++entry)
    {
        if (pattern.size() == entry.value().size())
        {
            bool match{true};

            for (int i = 0; i < pattern.size(); ++i)
            {
                if (!qFuzzyCompare(pattern.at(i), entry.value().at(i)))
                {
                    match = false;
                    break;
                }
            }

            if (match)
            {
                return entry.key();
            }
        }
    }

    // default to solid line
    return Qt::SolidLine;
}

QString UBShapeFactory::shapeTypeToString(ShapeType shapeType)
{
    switch (shapeType)
    {
    case ShapeType::None:
        return "none";

    case ShapeType::Ellipse:
        return "ellipse";

    case ShapeType::Circle:
        return "circle";

    case ShapeType::Rectangle:
        return "rectangle";

    case ShapeType::Square:
        return "square";

    case ShapeType::Line:
        return "line";

    case ShapeType::RegularPolygon:
        return "regularPolygon";

    case ShapeType::Polygon:
        return "polygon";
    }

    return "";
}

ShapeType UBShapeFactory::shapeTypeFromString(const QString& shapeTypeString)
{
    if (shapeTypeString == "ellipse")
    {
        return ShapeType::Ellipse;
    }
    else if (shapeTypeString == "circle")
    {
        return ShapeType::Circle;
    }
    else if (shapeTypeString == "rectangle")
    {
        return ShapeType::Rectangle;
    }
    else if (shapeTypeString == "square")
    {
        return ShapeType::Square;
    }
    else if (shapeTypeString == "line")
    {
        return ShapeType::Line;
    }
    else if (shapeTypeString == "regularPolygon")
    {
        return ShapeType::RegularPolygon;
    }
    else if (shapeTypeString == "polygon")
    {
        return ShapeType::Polygon;
    }

    return ShapeType::None;
}
