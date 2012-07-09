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

#include "GraphicsPDFItem.h"

#include <qglobal.h>

#include <QtGui/QStyleOptionGraphicsItem>

#include "core/memcheck.h"

GraphicsPDFItem::GraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem *parentItem)
    : QObject(0), QGraphicsItem(parentItem)
    , mRenderer(renderer)
    , mPageNumber(pageNumber)
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    mRenderer->attach();
}

GraphicsPDFItem::~GraphicsPDFItem()
{
    mRenderer->detach();
}

QRectF GraphicsPDFItem::boundingRect() const
{
    if (!mRenderer->isValid())
    {
        return QRectF();
    }

    return QRectF(QPointF(0, 0), mRenderer->pageSizeF(mPageNumber));
}

void GraphicsPDFItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (!mRenderer || !mRenderer->isValid())
    {
        qWarning("GraphicsPDFItem::paint: Invalid renderer");
        return;
    }

    if (option)
        mRenderer->render(painter, mPageNumber, option->exposedRect);
    else
        qWarning("GraphicsPDFItem::paint: option is null, ignoring painting");
}
