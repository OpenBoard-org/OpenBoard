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

#include "UBThumbnailView.h"
#include "domain/UBGraphicsScene.h"

#include "core/UBMimeData.h"

#include "core/memcheck.h"

UBThumbnailView::UBThumbnailView()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
}

UBThumbnailView::~UBThumbnailView()
{
    // NOOP
}

void UBThumbnailView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Do not draw crossed background in thumbnails
    if (qobject_cast<UBGraphicsScene*>(scene())->isDarkBackground())
    {
        painter->fillRect(rect, QBrush(QColor(Qt::black)));
    }
    else
    {
        painter->fillRect(rect, QBrush(QColor(Qt::white)));
    }
}

void UBThumbnailView::mouseDoubleClickEvent ( QMouseEvent * event )
{
    Q_UNUSED(event);
    emit doubleClicked();
}

