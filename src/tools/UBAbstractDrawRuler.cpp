/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBAbstractDrawRuler.h"
#include <QtSvg>
#include "core/UB.h"
#include "gui/UBResources.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#include "core/memcheck.h"


const QColor UBAbstractDrawRuler::sLightBackgroundMiddleFillColor = QColor(0x72, 0x72, 0x72, sFillTransparency);
const QColor UBAbstractDrawRuler::sLightBackgroundEdgeFillColor = QColor(0xc3, 0xc3, 0xc3, sFillTransparency);
const QColor UBAbstractDrawRuler::sLightBackgroundDrawColor = QColor(0x33, 0x33, 0x33, sDrawTransparency);
const QColor UBAbstractDrawRuler::sDarkBackgroundMiddleFillColor = QColor(0xb5, 0xb5, 0xb5, sFillTransparency);
const QColor UBAbstractDrawRuler::sDarkBackgroundEdgeFillColor = QColor(0xdd, 0xdd, 0xdd, sFillTransparency);
const QColor UBAbstractDrawRuler::sDarkBackgroundDrawColor = QColor(0xff, 0xff, 0xff, sDrawTransparency);

const int UBAbstractDrawRuler::sLeftEdgeMargin = 10;
const int UBAbstractDrawRuler::sDegreeToQtAngleUnit = 16;
const int UBAbstractDrawRuler::sRotationRadius = 15;
const int UBAbstractDrawRuler::sFillTransparency = 127;
const int UBAbstractDrawRuler::sDrawTransparency = 192;
const int UBAbstractDrawRuler::sRoundingRadius = sLeftEdgeMargin / 2;


UBAbstractDrawRuler::UBAbstractDrawRuler()
	: mShowButtons(false)
    , mAntiScaleRatio(1.0)
{
	//we actually need to evaluate pixels per millimeter
	QDesktopWidget* desktop = UBApplication::desktop();
	int dpiCommon = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
    sPixelsPerMillimeter = qRound(dpiCommon / UBGeometryUtils::inchSize);

}

void UBAbstractDrawRuler::create(QGraphicsItem& item)
{
	item.setFlag(QGraphicsItem::ItemIsMovable, true);
    item.setFlag(QGraphicsItem::ItemIsSelectable, true);
    item.setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    item.setAcceptsHoverEvents(true);

    mCloseSvgItem = new QGraphicsSvgItem(":/images/closeTool.svg", &item);
    mCloseSvgItem->setVisible(false);
    mCloseSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
}


UBAbstractDrawRuler::~UBAbstractDrawRuler()
{
}

QCursor UBAbstractDrawRuler::moveCursor() const
{
    return Qt::SizeAllCursor;
}

QCursor UBAbstractDrawRuler::rotateCursor() const
{
    return UBResources::resources()->rotateCursor;
}

QCursor UBAbstractDrawRuler::closeCursor() const
{
    return Qt::ArrowCursor;
}

QCursor UBAbstractDrawRuler::drawRulerLineCursor() const
{
	return UBResources::resources()->drawLineRulerCursor;
}

QColor UBAbstractDrawRuler::drawColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundDrawColor : sLightBackgroundDrawColor;
}

QColor UBAbstractDrawRuler::middleFillColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundMiddleFillColor : sLightBackgroundMiddleFillColor;
}

QColor UBAbstractDrawRuler::edgeFillColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundEdgeFillColor : sLightBackgroundEdgeFillColor;
}

QFont UBAbstractDrawRuler::font() const
{
    QFont font("Arial");
    font.setPixelSize(16);
    return font;
}

void UBAbstractDrawRuler::StartLine(const QPointF& position, qreal width)
{
    Q_UNUSED(position);
    Q_UNUSED(width);
}
void UBAbstractDrawRuler::DrawLine(const QPointF& position, qreal width)
{
    Q_UNUSED(position);
    Q_UNUSED(width);
}
void UBAbstractDrawRuler::EndLine()
{}


void UBAbstractDrawRuler::paint()
{
    mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
    QTransform antiScaleTransform;
    antiScaleTransform.scale(mAntiScaleRatio, mAntiScaleRatio);

    mCloseSvgItem->setTransform(antiScaleTransform);
    mCloseSvgItem->setPos(closeButtonRect().topLeft());

}


