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

#ifndef UBGRAPHICSITEMDELEGATE_H_
#define UBGRAPHICSITEMDELEGATE_H_

#include <QtGui>
#include <QtSvg>
#include <QMimeData>

#include "core/UB.h"

class QGraphicsSceneMouseEvent;
class QGraphicsItem;
class UBGraphicsProxyWidget;
class UBGraphicsDelegateFrame;
class UBGraphicsWidgetItem;

class DelegateButton: public QGraphicsSvgItem
{
    Q_OBJECT;

    public:
        DelegateButton(const QString & fileName, QGraphicsItem* pDelegated, QGraphicsItem * parent = 0)
            : QGraphicsSvgItem(fileName, parent)
            , mDelegated(pDelegated)
            , mIsTransparentToMouseEvent(false)
        {
            setAcceptedMouseButtons(Qt::LeftButton);
            setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
        }

        virtual ~DelegateButton()
        {
            // NOOP
        }

        void setTransparentToMouseEvent(bool tr)
        {
            mIsTransparentToMouseEvent = tr;
        }

        void setFileName(const QString & fileName)
        {
            QGraphicsSvgItem::setSharedRenderer(new QSvgRenderer (fileName, this));
        }

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event)
        {
            // make sure delegate is selected, to avoid control being hidden
            mDelegated->setSelected(true);

            event->setAccepted(!mIsTransparentToMouseEvent);
        }

        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
        {
            emit clicked();

            event->setAccepted(!mIsTransparentToMouseEvent);
        }

        void modified();

    private:

        QGraphicsItem* mDelegated;

        bool mIsTransparentToMouseEvent;

    signals:
        void clicked (bool checked = false);

};


class UBGraphicsItemDelegate : public QObject
{
    Q_OBJECT;

    public:
        UBGraphicsItemDelegate(QGraphicsItem* pDelegated, QObject * parent = 0,  bool respectRatio = true, bool canRotate = false);

        virtual ~UBGraphicsItemDelegate();

        void init();

        virtual bool mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                const QVariant &value);

        QGraphicsItem* delegated()
        {
            return mDelegated;
        }

        void setCanDuplicate(bool allow){ mCanDuplicate = allow; }

        virtual void positionHandles();

        void startUndoStep();
        void commitUndoStep();

        UBGraphicsDelegateFrame* frame() { return mFrame; }

        bool canRotate() { return mCanRotate; }
        bool isLocked();
        bool canDuplicate() { return mCanDuplicate; }

        QMimeData* mimeData(){ return mMimeData; }
        void setMimeData(QMimeData* mimeData);

    signals:

        void showOnDisplayChanged(bool shown);
        void lockChanged(bool locked);

    public slots:

        virtual void remove(bool canUndo = true);
        void showMenu();

    protected:
        virtual void buildButtons() {};
        virtual void decorateMenu(QMenu *menu);
        virtual void updateMenuActionState();

        QGraphicsItem* mDelegated;

        DelegateButton* mDeleteButton;
        DelegateButton* mDuplicateButton;
        DelegateButton* mMenuButton;

        QMenu* mMenu;

        QAction* mLockAction;
        QAction* mShowOnDisplayAction;
        QAction* mGotoContentSourceAction;

        UBGraphicsDelegateFrame* mFrame;
        qreal mFrameWidth;
        qreal mAntiScaleRatio;

        QList<DelegateButton*> mButtons;

    protected slots:

        virtual void gotoContentSource(bool checked);
        virtual void showHide(bool show);
        virtual void lock(bool lock);
        virtual void duplicate();

     private:

        QPointF mOffset;
        QTransform mPreviousTransform;
        QPointF mPreviousPosition;
        qreal mPreviousZValue;
        QSizeF mPreviousSize;
        bool mCanRotate;
        bool mCanDuplicate;
        bool mRespectRatio;
        QMimeData* mMimeData;
};


#endif /* UBGRAPHICSITEMDELEGATE_H_ */
