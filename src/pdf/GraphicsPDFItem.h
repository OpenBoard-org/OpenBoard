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




#ifndef GRAPHICSPDFITEM_H
#define GRAPHICSPDFITEM_H

#include <QtWidgets>
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
        void setCacheAllowed(bool const value) { mIsCacheAllowed = value; }
        virtual void updateChild() = 0;
    protected:
        PDFRenderer *mRenderer;
        int mPageNumber;
        bool mIsCacheAllowed;

    private slots:
        void OnRequireUpdate();
};

#endif // GRAPHICSPDFITEM_H
