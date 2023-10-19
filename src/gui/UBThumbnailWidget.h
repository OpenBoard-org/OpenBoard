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




#ifndef UBTHUMBNAILWIDGET_H_
#define UBTHUMBNAILWIDGET_H_

#include <QtGui>
#include <QtSvg>
#include <QTime>
#include <QGraphicsSceneHoverEvent>

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "frameworks/UBCoreGraphicsScene.h"
#include "core/UBSettings.h"
#include "domain/UBItem.h"
#include "gui/UBThumbnailView.h"
#include "document/UBDocumentProxy.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QElapsedTimer>


#define STARTDRAGTIME   1000000

class UBDocumentProxy;
class UBThumbnailTextItem;
class UBThumbnail;

class UBThumbnailWidget : public QGraphicsView
{
    Q_OBJECT

    public:
        UBThumbnailWidget(QWidget* parent);
        virtual ~UBThumbnailWidget();

        QList<QGraphicsItem*> selectedItems();
        void selectItemAt(int pIndex, bool extend = false);
        void unselectItemAt(int pIndex);
        void clearSelection();

        qreal thumbnailWidth()
        {
            return mThumbnailWidth;
        }

        void setBackgroundBrush(const QBrush& brush)
        {
            mThumbnailsScene.setBackgroundBrush(brush);
        }

    public slots:
        void setThumbnailWidth(qreal pThumbnailWidth);
        void setSpacing(qreal pSpacing);
        virtual void setGraphicsItems(const QList<QGraphicsItem*>& pGraphicsItems, const QList<QUrl>& pItemPaths, const QStringList pLabels = QStringList(), const QString& pMimeType = QString(""));
        void insertThumbnailToScene(QGraphicsPixmapItem* newThumbnail, UBThumbnailTextItem* thumbnailTextItem);
        void refreshScene();
        void sceneSelectionChanged();

    signals:
        void resized();
        void selectionChanged();
        void mouseDoubleClick(QGraphicsItem* item, int index);
        void mouseClick(QGraphicsItem* item, int index);


    protected:
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);
        virtual void resizeEvent(QResizeEvent * event);
        void mouseDoubleClickEvent(QMouseEvent * event);

        virtual void keyPressEvent(QKeyEvent *event);
        virtual void focusInEvent(QFocusEvent *event);

        QList<QGraphicsItem*> mGraphicItems;
        QList<UBThumbnailTextItem*> mLabelsItems;
        QPointF mMousePressScenePos;
        QPoint mMousePressPos;

    protected:
        qreal spacing() { return mSpacing; }
        QList<QUrl> mItemsPaths;
        QStringList mLabels;
        bool bSelectionInProgress;
        bool bCanDrag;

    private:
        void selectAll();
        void selectItems(int startIndex, int count);
        int rowCount() const;
        int columnCount() const;

        static bool thumbnailLessThan(QGraphicsItem* item1, QGraphicsItem* item2);

        void deleteLasso();

        UBCoreGraphicsScene mThumbnailsScene;

        QString mMimeType;

        QPointF prevMoveMousePos;

        qreal mThumbnailWidth;
        qreal mThumbnailHeight;
        qreal mSpacing;

        UBThumbnail *mLastSelectedThumbnail;
        int mSelectionSpan;
        QRectF mPrevLassoRect;
        QGraphicsRectItem *mLassoRectItem;
        QList<QGraphicsItem*> mSelectedThumbnailItems;
        QSet<QGraphicsItem*> mPreviouslyIncrementalSelectedItemsX;
        QSet<QGraphicsItem*> mPreviouslyIncrementalSelectedItemsY;
        QElapsedTimer mClickTime;
};

class UBThumbnail
{
    public:
        UBThumbnail();

        virtual ~UBThumbnail();

        QStyleOptionGraphicsItem muteStyleOption(const QStyleOptionGraphicsItem *option)
        {
            // Never draw the rubber band, we draw our custom selection with the DelegateFrame
            QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
            styleOption.state &= ~QStyle::State_Selected;

            return styleOption;
        }

         virtual void itemChange(QGraphicsItem *item, QGraphicsItem::GraphicsItemChange change, const QVariant &value)
        {
            Q_UNUSED(value);

            if ((change == QGraphicsItem::ItemSelectedHasChanged
                    || change == QGraphicsItem::ItemTransformHasChanged
                    || change == QGraphicsItem::ItemPositionHasChanged)
                    &&  item->scene())
            {
                if (item->isSelected())
                {
                    if (!mSelectionItem->scene())
                    {
                        item->scene()->addItem(mSelectionItem);
                        mSelectionItem->setZValue(item->zValue() - 1);
                        mAddedToScene = true;
                    }

                    mSelectionItem->setRect(
                        item->sceneBoundingRect().x() - 5,
                        item->sceneBoundingRect().y() - 5,
                        item->sceneBoundingRect().width() + 10,
                        item->sceneBoundingRect().height() + 10);

                    mSelectionItem->show();
                }
                else
                {
                    mSelectionItem->hide();
                }
            }
        }

        int column() { return mColumn; }
        void setColumn(int column) { mColumn = column; }
        int row() { return mRow; }
        void setRow(int row) { mRow = row; }
        UBThumbnailTextItem *label(){return mLabel;}
        void setLabel(UBThumbnailTextItem *label){mLabel = label;}
        QGraphicsRectItem* selectionItem(){ return mSelectionItem; }

    protected:
        QGraphicsRectItem *mSelectionItem;
        private:
        bool mAddedToScene;

        int mColumn;
        int mRow;
        UBThumbnailTextItem *mLabel;
};

class UBThumbnailTextItem : public QGraphicsTextItem
{
    Q_OBJECT
    public:
        UBThumbnailTextItem(int index)
            : QGraphicsTextItem(tr("Page %0").arg(index+1))
            , mWidth(0)
            , mUnelidedText(toPlainText())
            , mIsHighlighted(false)
        {

        }

        UBThumbnailTextItem(const QString& text)
            : QGraphicsTextItem(text)
            , mWidth(0)
            , mUnelidedText(text)
            , mIsHighlighted(false)
        {
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }

        QRectF boundingRect() const { return QRectF(QPointF(0.0, 0.0), QSize(mWidth, QFontMetricsF(font()).height() + 5));}

        void setWidth(qreal pWidth)
        {
                if (mWidth != pWidth)
                {
                        prepareGeometryChange();
                        mWidth = pWidth;
                        computeText();
                }
        }

        qreal width() {return mWidth;}

        void highlight()
        {
                if (!mIsHighlighted)
                {
                        mIsHighlighted = true;
                        computeText();
                }
        }

        void setPageNumber(int i)
        {
            mUnelidedText = tr("Page %0").arg(i);
            computeText();
        }

        void setText(const QString& text)
        {
            mUnelidedText = text;
            computeText();
        }

        void computeText()
        {
            QFontMetricsF fm(font());
            QString elidedText = fm.elidedText(mUnelidedText, Qt::ElideRight, mWidth);

            if (mIsHighlighted)
            {
                setHtml("<span style=\"color: #6682b5\">" + elidedText + "</span>");
            }
            else
            {
                setPlainText(elidedText);
            }
        }

    private:
        qreal mWidth;
        QString mUnelidedText;
        bool mIsHighlighted;
};

class UBWidgetTextThumbnailElement
{
protected:
    QGraphicsItem* thumbnail;
    UBThumbnailTextItem* caption;
    int border;

public:
    UBWidgetTextThumbnailElement(QGraphicsProxyWidget* proxyWidget, UBThumbnailTextItem* text)
        : thumbnail(proxyWidget)
        , caption(text)
        , border(0)
    {
    }

    QGraphicsItem* getThumbnail() const { return this->thumbnail; }
    void setThumbnail(QGraphicsItem* newGItem) { this->thumbnail = newGItem; }

    UBThumbnailTextItem* getCaption() const { return this->caption; }
    void setCaption(UBThumbnailTextItem* newcaption) { this->caption = newcaption; }

    void Place(int row, int col, qreal width, qreal height);

    int getBorder() const { return this->border; }
    void setBorder(int newBorder) { this->border = newBorder; }
};

class UBThumbnailPixmap : public QGraphicsPixmapItem, public UBThumbnail
{
    public:
        UBThumbnailPixmap(const QPixmap& pix, std::shared_ptr<UBDocumentProxy> proxy, int pSceneIndex)
            : QGraphicsPixmapItem(pix)
        {

        }

        UBThumbnailPixmap(const QPixmap& pix)
            : QGraphicsPixmapItem(pix)
        {
            setTransformationMode(Qt::SmoothTransformation); // UB 4.3 may be expensive -- make configurable
            setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }

        virtual QPainterPath shape () const
        {
            QPainterPath path;
            path.addRect(boundingRect());
            return path;
        }

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
        {
            QStyleOptionGraphicsItem styleOption = UBThumbnail::muteStyleOption(option);
            QGraphicsPixmapItem::paint(painter, &styleOption, widget);
        }

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)
        {
            UBThumbnail::itemChange(this, change, value);
            return QGraphicsPixmapItem::itemChange(change, value);
        }      
};

class UBSceneThumbnailPixmap : public UBThumbnailPixmap
{
    public:
        UBSceneThumbnailPixmap(const QPixmap& pix, std::shared_ptr<UBDocumentProxy> proxy, int pSceneIndex)
            : UBThumbnailPixmap(pix)
            , mDocumentProxy(proxy)
            , mSceneIndex(pSceneIndex)
        {
            // NOOP
        }

        virtual ~UBSceneThumbnailPixmap()
        {
            // NOOP
        }

        std::shared_ptr<UBDocumentProxy> documentProxy()
        {
            return mDocumentProxy;
        }

        int sceneIndex()
        {
            return mSceneIndex;
        }

        void setSceneIndex(int i)
        {
            mSceneIndex = i;
        }

        void highlight()
        {

        }

    private:
        std::shared_ptr<UBDocumentProxy> mDocumentProxy;
        int mSceneIndex;
};

class UBSceneThumbnailNavigPixmap : public UBSceneThumbnailPixmap
{
    public:
        UBSceneThumbnailNavigPixmap(const QPixmap& pix, std::shared_ptr<UBDocumentProxy> proxy, int pSceneIndex);
        ~UBSceneThumbnailNavigPixmap();

        bool editable()
        {
            return mEditable;
        }

        bool deletable()
        {
            return documentProxy()->pageCount() > 1;
        }

        bool movableUp()
        {
            return sceneIndex() > 0;
        }

        bool movableDown()
        {
            return sceneIndex() < (documentProxy()->pageCount() -1);
        }

        void showUI()
        {
            setEditable(true);
        }

        void hideUI()
        {
            setEditable(false);
        }

        void setEditable(bool editable)
        {
            mEditable = editable;
        }

    protected:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);

    private:
        void deletePage();
        void duplicatePage();
        void moveUpPage();
        void moveDownPage();

        bool mEditable;
};

class UBImgTextThumbnailElement
{
private:
    UBSceneThumbnailNavigPixmap* thumbnail;
    UBThumbnailTextItem* caption;
    int border;

public:
    UBImgTextThumbnailElement(UBSceneThumbnailNavigPixmap* thumb, UBThumbnailTextItem* text): border(0)
    {
        this->thumbnail = thumb;
        this->caption = text;
    }

    UBSceneThumbnailNavigPixmap* getThumbnail() const { return this->thumbnail; }
    void setThumbnail(UBSceneThumbnailNavigPixmap* newGItem) { this->thumbnail = newGItem; }

    UBThumbnailTextItem* getCaption() const { return this->caption; }
    void setCaption(UBThumbnailTextItem* newcaption) { this->caption = newcaption; }

    void Place(int row, int col, qreal width, qreal height);

    int getBorder() const { return this->border; }
    void setBorder(int newBorder) { this->border = newBorder; }
};

class UBThumbnailPixmapItem : public QGraphicsPixmapItem
{
    public:
        UBThumbnailPixmapItem(std::shared_ptr<UBDocumentProxy> proxy, int index)
            : mDocumentProxy(proxy)
            , mSceneIndex(index)
        {

        }

        std::shared_ptr<UBDocumentProxy> documentProxy()
        {
            return mDocumentProxy;
        }

        void setSceneIndex(int i)
        {
            mSceneIndex = i;
        }

        int sceneIndex()
        {
            return mSceneIndex;
        }

private:
        std::shared_ptr<UBDocumentProxy> mDocumentProxy;
        int mSceneIndex;
};

class UBDraggableThumbnailItem : public QObject, public UBThumbnailPixmapItem
{
    Q_OBJECT

    public:
        UBDraggableThumbnailItem(std::shared_ptr<UBDocumentProxy> documentProxy, int index)
        : UBThumbnailPixmapItem(documentProxy, index)
        , mEditable(false)
        {
            setAcceptHoverEvents(true);
        }

        bool editable()
        {
            return mEditable;
        }

        bool deletable()
        {
            return documentProxy()->pageCount() > 1;
        }

        bool movableUp()
        {
            return sceneIndex() > 0;
        }

        bool movableDown()
        {
            return sceneIndex() < (documentProxy()->pageCount() -1);
        }

        void showUI()
        {
            setEditable(true);
        }

        void hideUI()
        {
            setEditable(false);
        }

        void setEditable(bool editable)
        {
            mEditable = editable;
        }

    protected:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    private:
        void deletePage();
        void duplicatePage();
        void moveUpPage();
        void moveDownPage();

        bool mEditable;
};


class UBDraggableLivePixmapItem : public UBDraggableThumbnailItem
{
    Q_OBJECT
    public:
        UBDraggableLivePixmapItem(std::shared_ptr<UBGraphicsScene> pageScene, std::shared_ptr<UBDocumentProxy> documentProxy, int index);

        ~UBDraggableLivePixmapItem()
        {
            delete mPageNumber; // not a child of "this" QObject so it has to be deleted manually
        }

        void updatePos(qreal w, qreal h);

        UBThumbnailTextItem* pageNumber()
        {
            return mPageNumber;
        }

        void setPageNumber(int i)
        {
            mPageNumber->setPlainText(tr("Page %0").arg(i+1));

            if (UBApplication::boardController->activeSceneIndex() == i)
                mPageNumber->setHtml("<span style=\";font-weight:bold;color: #6682b5\">" + tr("Page %0").arg(i+1) + "</span>");
            else
                mPageNumber->setHtml("<span style=\";color: #000000\">" + tr("Page %0").arg(i+1) + "</span>");
        }

        void setHighlighted(bool highlighted);

        QGraphicsRectItem* selectionItem()
        {
            return mSelectionItem;
        }

        void setExposed(bool exposed);

        bool isExposed();

    public slots:
        void updatePixmap(const QRectF &region = QRectF());

    private:
        static const int sSelectionItemMargin = 5;
        QGraphicsRectItem *mSelectionItem;
        std::shared_ptr<UBGraphicsScene> mScene;
        UBThumbnailTextItem* mPageNumber;
        bool mExposed;
        QSizeF mSize;
        QTransform mTransform;
        QTimer updateTimer;
        int updateCount;
        bool ignoreNextEvent;
};

namespace UBThumbnailUI
{
    const int ICONSIZE      = 32;
    const int ICONSPACING   = 10;

    class UBThumbnailUIIcon : public QPixmap
    {
        public:

            UBThumbnailUIIcon(const QString& filename, int pos)
                : QPixmap(QSize(ICONSIZE, ICONSIZE))
                , mPos(pos)
            {
                QSvgRenderer svgRenderer(filename);
                QPainter painter;
                fill(Qt::transparent);
                painter.begin(this);
                svgRenderer.render(&painter);
                painter.end();
            }

            int pos() const
            {
                return mPos;
            }

            bool triggered(qreal x) const
            {
                using namespace UBThumbnailUI;
                return (x >= pos()*(ICONSIZE + ICONSPACING) && x <= (pos()+1)*ICONSIZE + pos()*ICONSPACING);
            }

        private:
            int mPos;
    };

    namespace _private
    {
        //do not use this directly
        static QMap<QString, UBThumbnailUIIcon*> catalog;
        void initCatalog();
    }

    UBThumbnailUIIcon* addIcon(const QString& thumbnailIcon, int pos);
    UBThumbnailUIIcon* getIcon(const QString& thumbnailIcon);
    void draw(QPainter* painter, const UBThumbnailUIIcon& thumbnailIcon);
    bool triggered(qreal y);
}

#endif /* UBTHUMBNAILWIDGET_H_ */
