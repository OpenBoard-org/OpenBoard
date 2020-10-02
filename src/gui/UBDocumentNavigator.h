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




#ifndef UBDOCUMENTNAVIGATOR_H
#define UBDOCUMENTNAVIGATOR_H

#include <QResizeEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QThread>

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentContainer.h"
#include "UBThumbnailWidget.h"

#define NO_PAGESELECTED            -1

class UBDocumentNavigator : public QGraphicsView
{
    Q_OBJECT
public:
    UBDocumentNavigator(QWidget* parent=0, const char* name="documentNavigator");
    ~UBDocumentNavigator();

    void setNbColumns(int nbColumns);
    int nbColumns();
    void setThumbnailMinWidth(int width);
    int thumbnailMinWidth();
    UBSceneThumbnailNavigPixmap* clickedThumbnail(const QPoint pos) const;

public slots:
    void onScrollToSelectedPage(int index);// { if (mCrntItem) centerOn(mCrntItem); }
    void generateThumbnails(UBDocumentContainer* source);
    void updateSpecificThumbnail(int iPage);    

    void longPressTimeout();
    void mousePressAndHoldEvent();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent* event);

    virtual void keyPressEvent(QKeyEvent *event);

signals:
    void mousePressAndHoldEventRequired();
    void moveThumbnailRequired(int from, int to);

private:

    void refreshScene();
    int border();

    /** The scene */
    QGraphicsScene* mScene;
    /** The current selected item */
    //UBSceneThumbnailNavigPixmap* mCrntItem;
    /** The list of current thumbnails with labels*/
    QList<UBImgTextThumbnailElement> mThumbsWithLabels;
    /** The current number of columns */
    int mNbColumns;
    /** The current thumbnails width */
    int mThumbnailWidth;
    /** The current thumbnails minimum width */
    int mThumbnailMinWidth;
    /** The selected thumbnail */
    UBSceneThumbnailNavigPixmap* mSelectedThumbnail;
    UBSceneThumbnailNavigPixmap* mLastClickedThumbnail;
    UBSceneThumbnailNavigPixmap* mDropSource;
    UBSceneThumbnailNavigPixmap* mDropTarget;
    QGraphicsRectItem *mDropBar;

    int mLongPressInterval;
    QTimer mLongPressTimer;
    QPoint mLastPressedMousePos;
};

#endif // UBDOCUMENTNAVIGATOR_H

