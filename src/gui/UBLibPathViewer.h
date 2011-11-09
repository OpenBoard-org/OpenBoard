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
#ifndef UBLIBPATHVIEWER_H
#define UBLIBPATHVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QVector>
#include <QPixmap>
#include <QResizeEvent>
#include <QGraphicsLinearLayout>
#include <QMouseEvent>
#include <QTime>
#include <QMap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QPaintEvent>
#include <QShowEvent>

#include "board/UBLibraryController.h"

#define PATHITEMWIDTH   32
#define STARTDRAGTIME   1000000

class UBPathScene : public QGraphicsScene
{
    Q_OBJECT
public:
    UBPathScene(QWidget* parent=0);
    ~UBPathScene();
    QMap<QGraphicsWidget*, UBChainedLibElement*>* mapWidgetToChainedElem(){return &mMapWidgetToChainedElem;}

signals:
    void mouseClick(UBChainedLibElement* elem);
    void elementsDropped(QList<QString> elements, UBLibElement* target);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

private slots:
    void onAllDownloadsFinished();

private:
    UBLibElement* elementFromPos(QPointF p);
    /** The drag start position */
    QPointF mDragStartPos;
    /** A timer used to detect a click or a drag */
    QTime mClickTime;
    /** A map between a widget and a chained element */
    QMap<QGraphicsWidget*, UBChainedLibElement*> mMapWidgetToChainedElem;
};

class UBLibPathViewer : public QGraphicsView
{
    Q_OBJECT
public:
    UBLibPathViewer(QWidget* parent=0, const char* name="UBLibPathViewer");
    ~UBLibPathViewer();

    void displayPath(UBChainedLibElement* elementsChain);
    int widgetAt(QPointF p);
    void updateScrolls();

public slots:
    void showBack();

signals:
    void mouseClick(UBChainedLibElement* elem);
    void elementsDropped(QList<QString> elements, UBLibElement* target);

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void onMouseClicked(UBChainedLibElement* elem);
    void onElementsDropped(QList<QString> elements, UBLibElement* target);
    void onSliderMoved(int value);

private:
    void refreshPath();
    void addItem(UBChainedLibElement* elem);

    /** The chained path elements */
    UBChainedLibElement* mpElems;
    /** The backup chained path elements */
    UBChainedLibElement* mpElemsBackup;
    /** The scene */
    UBPathScene* mpScene;
    //QGraphicsScene* mpScene;
    /** The layout */
    QGraphicsLinearLayout* mpLayout;
    /** The container that will display the path properly */
    QGraphicsWidget* mpContainer;
    /** The list of path items (icons + arrows) */
    QVector<QGraphicsWidget*> mVItems;
    /** The total width of the element in the scene */
    int mSceneWidth;
    /** The back element */
    UBLibElement* mpBackElem;
};


class UBFolderPath : public QGraphicsProxyWidget
{
public:
    UBFolderPath();
    ~UBFolderPath();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QDropEvent *pEvent);
    void dragMoveEvent(QDragMoveEvent *pEvent);

private:
    void processMimeData(const QMimeData* pData);


};

#endif // UBLIBPATHVIEWER_H
