/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef GRAPHICSPDFITEM_H
#define GRAPHICSPDFITEM_H

#include <QtGui/QGraphicsItem>
#include <QtCore/QObject>

#include "PDFRenderer.h"

class GraphicsPDFItem : public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

    public:
        GraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem *parentItem = 0);
        virtual ~GraphicsPDFItem();

        virtual QRectF boundingRect() const;

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        int pageNumber() const { return mPageNumber; }
        QUuid fileUuid() const { return mRenderer->fileUuid(); }
        QByteArray fileData() const { return mRenderer->fileData(); }

    protected:
        PDFRenderer *mRenderer;
        int mPageNumber;
};

#endif // GRAPHICSPDFITEM_H
