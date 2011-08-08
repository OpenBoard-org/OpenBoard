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

    protected:

        virtual void mouseMoveEvent(QMouseEvent *event);

        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dragLeaveEvent(QDragLeaveEvent *event);
        virtual void dragMoveEvent(QDragMoveEvent *event);
        virtual void dropEvent(QDropEvent *event);

    private:
        void deleteDropCaret();

        QGraphicsRectItem *mDropCaretRectItem;
        UBSceneThumbnailPixmap *mClosestDropItem;
        bool mDropIsRight;
        bool mDragEnabled;
};

#endif /* UBDOCUMENTTHUMBNAILWIDGET_H_ */
