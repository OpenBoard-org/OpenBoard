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




#ifndef UBDOCUMENTTHUMBNAILWIDGET_H_
#define UBDOCUMENTTHUMBNAILWIDGET_H_

#include "UBThumbnailWidget.h"

class UBGraphicsScene;

class UBDocumentThumbnailWidget: public UBThumbnailWidget
{
    Q_OBJECT;

    public:
        UBDocumentThumbnailWidget(QWidget* parent);
        virtual ~UBDocumentThumbnailWidget();

        void setDragEnabled(bool enabled);
        bool dragEnabled() const;

        void hightlightItem(int index);

    public slots:
        virtual void setGraphicsItems(const QList<QGraphicsItem*>& pGraphicsItems,
            const QList<QUrl>& pItemPaths, const QStringList pLabels = QStringList(),
            const QString& pMimeType = QString(""));

    signals:
        void sceneDropped(UBDocumentProxy* proxy, int source, int target);

    private slots:
        void autoScroll();

    protected:

        virtual void mouseMoveEvent(QMouseEvent *event);

        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dragLeaveEvent(QDragLeaveEvent *event);
        virtual void dragMoveEvent(QDragMoveEvent *event);
        virtual void dropEvent(QDropEvent *event);

    private:
        void deleteDropCaret();

        QGraphicsRectItem *mDropCaretRectItem;
        UBThumbnailPixmap *mClosestDropItem;
        bool mDropIsRight;
        bool mDragEnabled;
        QTimer* mScrollTimer;
        int mScrollMagnitude;
};

#endif /* UBDOCUMENTTHUMBNAILWIDGET_H_ */
