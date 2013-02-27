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
