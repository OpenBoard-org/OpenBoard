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

#ifndef UBTHUMBNAILWIDGET_H_
#define UBTHUMBNAILWIDGET_H_

#include <QtGui>
#include <QtSvg>
#include <QTime>
#include <QGraphicsSceneHoverEvent>

#include "frameworks/UBCoreGraphicsScene.h"
#include "core/UBSettings.h"
#include "domain/UBItem.h"

#define STARTDRAGTIME   1000000
#define BUTTONSIZE      48
#define BUTTONSPACING   5

class UBDocumentProxy;
class UBThumbnailTextItem;
class UBThumbnail;

class UBThumbnailWidget : public QGraphicsView
{
    Q_OBJECT;

    public:
        UBThumbnailWidget(QWidget* parent);
        virtual ~UBThumbnailWidget();

        QList<QGraphicsItem*> selectedItems();
        void selectItemAt(int pIndex, bool extend = false);
        void unselectItemAt(int pIndex);

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
        void unselectAll();
        void selectItems(int startIndex, int count);
        int rowCount() const;
        int columnCount() const;

        static bool thumbnailLessThan(QGraphicsItem* item1, QGraphicsItem* item2);

        void deleteLasso();

        UBCoreGraphicsScene mThumbnailsScene;

        QString mMimeType;

        qreal mThumbnailWidth;
        qreal mThumbnailHeight;
        qreal mSpacing;

        UBThumbnail *mLastSelectedThumbnail;
        int mSelectionSpan;
        QGraphicsRectItem *mLassoRectItem;
        QList<QGraphicsItem*> mSelectedThumbnailItems;
        QTime mClickTime;
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
//                        mSelectionItem->setZValue(item->zValue() - 1);
                        UBGraphicsItem::assignZValue(mSelectionItem, item->zValue() - 1);
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

    protected:
        QGraphicsRectItem *mSelectionItem;
        private:
        bool mAddedToScene;

        int mColumn;
        int mRow;
};


class UBThumbnailSvg : public QGraphicsSvgItem, public UBThumbnail
{
    public:
        UBThumbnailSvg(const QString& path)
            : QGraphicsSvgItem(path)
        {
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }

        virtual ~UBThumbnailSvg()
        {
            // NOOP
        }

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
        {
            QStyleOptionGraphicsItem styleOption = UBThumbnail::muteStyleOption(option);
            QGraphicsSvgItem::paint(painter, &styleOption, widget);
        }

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)
        {
            UBThumbnail::itemChange(this, change, value);
            return QGraphicsSvgItem::itemChange(change, value);
        }

};


class UBThumbnailPixmap : public QGraphicsPixmapItem, public UBThumbnail
{
    public:
        UBThumbnailPixmap(const QPixmap& pix)
            : QGraphicsPixmapItem(pix)
        {
            setTransformationMode(Qt::SmoothTransformation); // UB 4.3 may be expensive -- make configurable
            setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }

        virtual ~UBThumbnailPixmap()
        {
            // NOOP
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
        UBSceneThumbnailPixmap(const QPixmap& pix, UBDocumentProxy* proxy, int pSceneIndex)
            : UBThumbnailPixmap(pix)
            , mProxy(proxy)
            , mSceneIndex(pSceneIndex)
        {
            // NOOP
        }

        virtual ~UBSceneThumbnailPixmap()
        {
            // NOOP
        }

        UBDocumentProxy* proxy()
        {
            return mProxy;
        }

        int sceneIndex()
        {
            return mSceneIndex;
        }

        void highlight()
        {
            //NOOP
        }

    private:
        UBDocumentProxy* mProxy;
        int mSceneIndex;
};

class UBSceneThumbnailNavigPixmap : public UBSceneThumbnailPixmap
{
    public:
        UBSceneThumbnailNavigPixmap(const QPixmap& pix, UBDocumentProxy* proxy, int pSceneIndex);
        ~UBSceneThumbnailNavigPixmap();
        void notifyClick(QPointF clickedScenePos);

    protected:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);

    private:
        void updateButtonsState();
        void deletePage();
        void moveUpPage();
        void moveDownPage();
        QPointF clickedPos(QPointF clickedScenePos);

        bool bButtonsVisible;
        bool bCanDelete;
        bool bCanMoveUp;
        bool bCanMoveDown;
};

class UBThumbnailVideo : public UBThumbnailPixmap
{
    public:
        UBThumbnailVideo(const QUrl &path)
            : UBThumbnailPixmap(QPixmap(":/images/movie.svg"))
            , mPath(path)
        {
            // NOOP
        }

        virtual ~UBThumbnailVideo()
        {
            // NOOP
        }

        QUrl path()
        {
            return mPath;
        }

    private:

        QUrl mPath;
};

class UBThumbnailTextItem : public QGraphicsTextItem
{
    public:
        UBThumbnailTextItem(const QString& text)
            : QGraphicsTextItem(text)
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
        };

        qreal width() {return mWidth;}

        void highlight()
        {
                if (!mIsHighlighted)
                {
                        mIsHighlighted = true;
                        computeText();
                }
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


#endif /* UBTHUMBNAILWIDGET_H_ */
