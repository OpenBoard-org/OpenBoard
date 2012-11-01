/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
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

#define NO_PAGESELECTED		    -1

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

public slots:
    void onScrollToSelectedPage(int index);// { if (mCrntItem) centerOn(mCrntItem); }
    void generateThumbnails(UBDocumentContainer* source);
    void updateSpecificThumbnail(int iPage);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

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
};
 
#endif // UBDOCUMENTNAVIGATOR_H
