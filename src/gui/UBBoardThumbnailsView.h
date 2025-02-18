/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBBOARDTHUMBNAILSVIEW_H
#define UBBOARDTHUMBNAILSVIEW_H

#include <QResizeEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QTimer>

#include "gui/UBThumbnailArranger.h"
#include "gui/UBThumbnailsView.h"

class UBDocument;
class UBThumbnail;

class UBBoardThumbnailsView : public UBThumbnailsView
{
    Q_OBJECT
public:
    UBBoardThumbnailsView(QWidget* parent=0, const char* name="UBBoardThumbnailsView");

    void setDocument(std::shared_ptr<UBDocument> document);

public slots:
    void updateActiveThumbnail(int newActiveIndex);
    void ensureVisibleThumbnail(int index);
    void centerOnThumbnail(int index);

    void adjustThumbnail();

    void longPressTimeout();
    void mousePressAndHoldEvent(QPoint pos);
    void updateThumbnailPixmap(const QRectF region);

protected:
    virtual bool event(QEvent* event);
    virtual void resizeEvent(QResizeEvent *event);

    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent* event);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void scrollContentsBy(int dx, int dy);

signals:
    void mousePressAndHoldEventRequired(QPoint pos);
    void moveThumbnailRequired(int from, int to);

private:
    std::shared_ptr<UBDocument> mDocument;

    int mThumbnailWidth;
    const int mThumbnailMinWidth;
    const int mMargin;

    UBThumbnail* mDropSource;
    UBThumbnail* mDropTarget;
    QGraphicsRectItem *mDropBar;

    int mLongPressInterval;
    QTimer mLongPressTimer;
    QPoint mLastPressedMousePos;

    int mCurrentIndex{-1};
    bool mScrollbarVisible{false};

    /**
     * @brief The UBBoardThumbnailArranger class is the arranger for Board mode.
     */
    class UBBoardThumbnailArranger : public UBThumbnailArranger
    {
    public:
        UBBoardThumbnailArranger(UBBoardThumbnailsView* thumbnailView);

        virtual int columnCount() const override;
        virtual double thumbnailWidth() const override;
    };
};

#endif // UBBOARDTHUMBNAILSVIEW_H
