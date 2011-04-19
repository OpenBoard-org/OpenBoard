/*
 * UBDocumentThumbnailWidget.h
 *
 *  Created on: Dec 8, 2008
 *      Author: Luc
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
