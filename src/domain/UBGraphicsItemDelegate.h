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
    Q_OBJECT

    public:
        DelegateButton(const QString & fileName, QGraphicsItem* pDelegated, QGraphicsItem * parent = 0, Qt::WindowFrameSection section = Qt::TopLeftSection)
            : QGraphicsSvgItem(fileName, parent)
            , mDelegated(pDelegated)
            , mIsTransparentToMouseEvent(false)
            , mButtonAlignmentSection(section)
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

        void setSection(Qt::WindowFrameSection section) {mButtonAlignmentSection =  section;}
        Qt::WindowFrameSection getSection() const {return mButtonAlignmentSection;}

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
        Qt::WindowFrameSection mButtonAlignmentSection;

    signals:
        void clicked (bool checked = false);

};


class UBGraphicsItemDelegate : public QObject
{
    Q_OBJECT

    public:
        UBGraphicsItemDelegate(QGraphicsItem* pDelegated, QObject * parent = 0,  bool respectRatio = true, bool canRotate = false);

        virtual ~UBGraphicsItemDelegate();

        void init();

        virtual bool mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual bool weelEvent(QGraphicsSceneWheelEvent *event);

        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                const QVariant &value);

        void printMessage(const QString &mess) {qDebug() << mess;}

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

        void setFlippable(bool flippable);
        bool isFlippable();

        void setButtonsVisible(bool visible);

    signals:
        void showOnDisplayChanged(bool shown);
        void lockChanged(bool locked);

    public slots:
        virtual void remove(bool canUndo = true);
        void showMenu();

        virtual void showHide(bool show);
        virtual void lock(bool lock);
        virtual void duplicate();

        virtual void increaseZLevel() {increaseZLevel(1);}
        virtual void decreaseZLevel() {increaseZLevel(-1);}

    protected:
        virtual void buildButtons() {;}
        virtual void decorateMenu(QMenu *menu);
        virtual void updateMenuActionState();

        QGraphicsItem* mDelegated;

        //buttons from the top left section of delegate frame
        DelegateButton* mDeleteButton;
        DelegateButton* mDuplicateButton;
        DelegateButton* mMenuButton;

        //buttons from the bottom left section of delegate frame
        DelegateButton *mZOrderUpButton;
        DelegateButton *mZOrderDownButton;

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

private:

        virtual void increaseZLevel(int delta);

        QPointF mOffset;
        QTransform mPreviousTransform;
        QPointF mPreviousPosition;
        qreal mPreviousZValue;
        QSizeF mPreviousSize;
        bool mCanRotate;
        bool mCanDuplicate;
        bool mRespectRatio;
        QMimeData* mMimeData;

        /** A boolean saying that this object can be flippable (mirror effect) */
        bool mFlippable;
};


#endif /* UBGRAPHICSITEMDELEGATE_H_ */
