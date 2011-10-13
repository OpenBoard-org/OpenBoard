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
#ifndef UBDOCUMENTNAVIGATOR_H
#define UBDOCUMENTNAVIGATOR_H

#include <QResizeEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QThread>

#include "document/UBDocumentProxy.h"
#include "UBThumbnailWidget.h"

#define NO_PAGESELECTED		    -1

class UBDocumentNavigator : public QGraphicsView
{
    Q_OBJECT
public:
    UBDocumentNavigator(QWidget* parent=0, const char* name="documentNavigator");
    ~UBDocumentNavigator();

    void setDocument(UBDocumentProxy* document);
    void setNbColumns(int nbColumns);
    int nbColumns();
    void setThumbnailMinWidth(int width);
    int thumbnailMinWidth();
    int selectedPageNumber();
    UBDocumentProxy* currentDoc();

signals:
    void changeCurrentPage();

public slots:
    void onMovedToIndex(int index);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

private slots:
    void addNewPage();
    void onSelectionChanged();
    void generateThumbnails();

private:
    void setGraphicsItems(QList<QGraphicsItem*> items, QStringList labels);
    void refreshScene();
    void updateSpecificThumbnail(int iPage);
    int border();


    /** The scene */
    QGraphicsScene* mScene;
    /** The current selected item */
    UBSceneThumbnailNavigPixmap* mCrntItem;
    /** The current document */
    UBDocumentProxy* mCrntDoc;
    /** The list of current thumbnails */
    QList<QGraphicsItem*> mThumbnails;
    /** The list of current label items */
    QList<UBThumbnailTextItem*> mLabels;
    /** The list of current labels */
    QStringList mLab;
    /** The current number of columns */
    int mNbColumns;
    /** The current thumbnails width */
    int mThumbnailWidth;
    /** The current thumbnails minimum width */
    int mThumbnailMinWidth;
    /** A flag indicating that a thumbnail refresh is in progress */
    bool bNavig;
};

#endif // UBDOCUMENTNAVIGATOR_H
