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

#include <QtGui>
#include <QtSvg>
#include <QDrag>

#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsItemUndoCommand.h"
#include "UBGraphicsItemTransformUndoCommand.h"

#include "board/UBBoardController.h" // TODO UB 4.x clean that dependency
#include "board/UBBoardView.h" // TODO UB 4.x clean that dependency

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"
#include "core/UBPersistenceManager.h"

#include "document/UBDocumentProxy.h"

#include "UBGraphicsWidgetItem.h"

#include "domain/UBAbstractWidget.h"

#include "web/UBWebController.h"

#include "frameworks/UBFileSystemUtils.h"
#include "board/UBDrawingController.h"

#include "core/memcheck.h"

class UBGraphicsParaschoolEditorWidgetItem;

UBGraphicsItemDelegate::UBGraphicsItemDelegate(QGraphicsItem* pDelegated, QObject * parent, bool respectRatio, bool canRotate)
    : QObject(parent)
    , mDelegated(pDelegated)
    , mDeleteButton(NULL)
    , mDuplicateButton(NULL)
    , mMenuButton(NULL)
    , mMenu(0)
    , mLockAction(0)
    , mShowOnDisplayAction(0)
    , mGotoContentSourceAction(0)
    , mFrame(0)
    , mFrameWidth(UBSettings::settings()->objectFrameWidth)
    , mAntiScaleRatio(1.0)
    , mCanRotate(canRotate)
    , mCanDuplicate(true)
    , mRespectRatio(respectRatio)
    , mMimeData(NULL)
{
    // NOOP
}

void UBGraphicsItemDelegate::init()
{
    mFrame = new UBGraphicsDelegateFrame(this, QRectF(0, 0, 0, 0), mFrameWidth, mRespectRatio);
    mFrame->hide();
    mFrame->setZValue(UBGraphicsScene::toolLayerStart + 1);
    mFrame->setFlag(QGraphicsItem::ItemIsSelectable, true);

    mDeleteButton = new DelegateButton(":/images/close.svg", mDelegated, mFrame);
    mButtons << mDeleteButton;
    connect(mDeleteButton, SIGNAL(clicked()), this, SLOT(remove()));
    if (canDuplicate()){
        mDuplicateButton = new DelegateButton(":/images/duplicate.svg", mDelegated, mFrame);
        connect(mDuplicateButton, SIGNAL(clicked(bool)), this, SLOT(duplicate()));
        mButtons << mDuplicateButton;
    }
    mMenuButton = new DelegateButton(":/images/menu.svg", mDelegated, mFrame);
    connect(mMenuButton, SIGNAL(clicked()), this, SLOT(showMenu()));
    mButtons << mMenuButton;

    buildButtons();

    foreach(DelegateButton* button, mButtons)
    {
        button->hide();
        button->setZValue(UBGraphicsScene::toolLayerStart + 2);
        button->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
}


UBGraphicsItemDelegate::~UBGraphicsItemDelegate()
{
    // do not release mMimeData.
    // the mMimeData is owned by QDrag since the setMimeData call as specified in the documentation
}

QVariant UBGraphicsItemDelegate::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if ((change == QGraphicsItem::ItemSelectedHasChanged
         || change == QGraphicsItem::ItemPositionHasChanged
         || change == QGraphicsItem::ItemTransformHasChanged)
        && mDelegated->scene())
        {
        mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());

        if (mDelegated->isSelected())
        {
            QList<QGraphicsItem*> items = mDelegated->scene()->selectedItems();

            foreach(QGraphicsItem* item, items)
            {
                if (item != mDelegated)
                {
                    item->setSelected(false);
                }
            }
        }

        positionHandles();
    }

    if (change == QGraphicsItem::ItemPositionHasChanged
        || change == QGraphicsItem::ItemTransformHasChanged
        || change == QGraphicsItem::ItemZValueHasChanged)
    {
        UBGraphicsScene* ubScene = qobject_cast<UBGraphicsScene*>(mDelegated->scene());
        if(ubScene)
            ubScene->setModified(true);
    }

    return value;
}


bool UBGraphicsItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(NULL != mMimeData)
    {
        QDrag* mDrag = new QDrag(event->widget());
        mDrag->setMimeData(mMimeData);
        mDrag->start();
    }

    startUndoStep();

    if (!mDelegated->isSelected())
    {
        mDelegated->setSelected(true);

        UBGraphicsScene* scene = static_cast<UBGraphicsScene*>(mDelegated->scene());

        qDebug() << mDelegated->zValue();
        qDebug() << scene->currentObjectZIndex();

        if (mDelegated->zValue() < scene->currentObjectZIndex() && !isLocked())
            mDelegated->setZValue(scene->getNextObjectZIndex());

        positionHandles();

        return true;
    }
    else
    {
        return false;
    }
}

void UBGraphicsItemDelegate::setMimeData(QMimeData *mimeData)
{
    mMimeData = mimeData;
}

bool UBGraphicsItemDelegate::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(isLocked())
    {
        event->accept();
        return true;
    }
    else
        return false;
}


bool UBGraphicsItemDelegate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    commitUndoStep();

    return true;
}


void UBGraphicsItemDelegate::positionHandles()
{
    if (mDelegated->isSelected())
    {
        if (mFrame && !mFrame->scene() && mDelegated->scene())
        {
            mDelegated->scene()->addItem(mFrame);
        }

        mFrame->setAntiScale(mAntiScaleRatio);
        mFrame->positionHandles();
        mFrame->show();

        QTransform tr;
        tr.scale(mAntiScaleRatio, mAntiScaleRatio);

        mDeleteButton->setTransform(tr);

        qreal x = mFrame->rect().left()- mDeleteButton->renderer()->viewBox().width() * mAntiScaleRatio / 2;
        qreal y = mFrame->rect().top() - mDeleteButton->renderer()->viewBox().height() * mAntiScaleRatio / 2;

        mDeleteButton->setPos(x, y);

        if (!mDeleteButton->scene())
        {
            mDeleteButton->setParentItem(mFrame);//update parent for the case the item has been previously removed from scene
            if (mDelegated->scene())
                mDelegated->scene()->addItem(mDeleteButton);
        }

        mDeleteButton->show();

        bool shownOnDisplay = mDelegated->data(UBGraphicsItemData::ItemLayerType).toInt() != UBItemLayerType::Control;
        showHide(shownOnDisplay);

        lock(isLocked());

        for(int i = 1 ; i < mButtons.length(); i++)
        {
            DelegateButton* button = mButtons[i];

            button->setTransform(tr);
            button->setPos(x + (i * 1.6 * mFrameWidth * mAntiScaleRatio), y);
            if (!button->scene())
            {
                button->setParentItem(mFrame);//update parent for the case the item has been previously removed from scene
                if (mDelegated->scene())
                    mDelegated->scene()->addItem(button);
            }
            button->show();
        }
    }
    else
    {
        foreach(DelegateButton* button, mButtons)
            button->hide();

        mFrame->hide();
    }
}


void UBGraphicsItemDelegate::remove(bool canUndo)
{
    QGraphicsScene* scene = mDelegated->scene();

    if (scene)
    {
        foreach(DelegateButton* button, mButtons)
            scene->removeItem(button);

        scene->removeItem(mFrame);
        scene->removeItem(mDelegated);

        if (canUndo)
        {
            UBGraphicsItemUndoCommand *uc = new UBGraphicsItemUndoCommand((UBGraphicsScene*) scene, mDelegated, 0);
            UBApplication::undoStack->push(uc);
        }
    }
}


bool UBGraphicsItemDelegate::isLocked()
{
    return mDelegated->data(UBGraphicsItemData::ItemLocked).toBool();
}


void UBGraphicsItemDelegate::duplicate()
{
    // TODO UB 4.x .. rewrite .. .this is absurde ... we know what we are duplicating

    UBApplication::boardController->copy();
    UBApplication::boardController->paste();
}


void UBGraphicsItemDelegate::lock(bool locked)
{
    if (locked)
    {
        mDelegated->setData(UBGraphicsItemData::ItemLocked, QVariant(true));
    }
    else
    {
        mDelegated->setData(UBGraphicsItemData::ItemLocked, QVariant(false));
    }

    mDelegated->update();
    mFrame->positionHandles();
}


void UBGraphicsItemDelegate::showHide(bool show)
{
    if (show)
    {
        mDelegated->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Object));
    }
    else
    {
        mDelegated->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    }

    mDelegated->update();

    emit showOnDisplayChanged(show);
}


void UBGraphicsItemDelegate::gotoContentSource(bool checked)
{
    Q_UNUSED(checked)

    UBItem* item = dynamic_cast<UBItem*>(mDelegated);

    if(item && !item->sourceUrl().isEmpty())
    {
        UBApplication::applicationController->showInternet();
        UBApplication::webController->loadUrl(item->sourceUrl());
    }
}

void UBGraphicsItemDelegate::startUndoStep()
{
    mPreviousPosition = mDelegated->pos();
    mPreviousTransform = mDelegated->transform();
    mPreviousZValue = mDelegated->zValue();
    UBResizableGraphicsItem* resizableItem = dynamic_cast<UBResizableGraphicsItem*>(mDelegated);

    if (resizableItem)
        mPreviousSize = resizableItem->size();
    else
        mPreviousSize = QSizeF();
}


void UBGraphicsItemDelegate::commitUndoStep()
{
    UBResizableGraphicsItem* resizableItem = dynamic_cast<UBResizableGraphicsItem*>(mDelegated);

    if (mDelegated->pos() != mPreviousPosition
        || mDelegated->transform() != mPreviousTransform
        || mDelegated->zValue() != mPreviousZValue
        || (resizableItem && resizableItem->size() != mPreviousSize))
    {
        UBGraphicsItemTransformUndoCommand *uc =
                new UBGraphicsItemTransformUndoCommand(mDelegated,
                                                       mPreviousPosition,
                                                       mPreviousTransform,
                                                       mPreviousZValue,
                                                       mPreviousSize);

        UBApplication::undoStack->push(uc);
    }
}


void UBGraphicsItemDelegate::decorateMenu(QMenu* menu)
{
    mLockAction = menu->addAction(tr("Locked"), this, SLOT(lock(bool)));
    QIcon lockIcon;
    lockIcon.addPixmap(QPixmap(":/images/locked.svg"), QIcon::Normal, QIcon::On);
    lockIcon.addPixmap(QPixmap(":/images/unlocked.svg"), QIcon::Normal, QIcon::Off);
    mLockAction->setIcon(lockIcon);
    mLockAction->setCheckable(true);

    mShowOnDisplayAction = mMenu->addAction(tr("Visible on Extended Screen"), this, SLOT(showHide(bool)));
    mShowOnDisplayAction->setCheckable(true);

    QIcon showIcon;
    showIcon.addPixmap(QPixmap(":/images/eyeOpened.svg"), QIcon::Normal, QIcon::On);
    showIcon.addPixmap(QPixmap(":/images/eyeClosed.svg"), QIcon::Normal, QIcon::Off);
    mShowOnDisplayAction->setIcon(showIcon);

    mGotoContentSourceAction = menu->addAction(tr("Go to Content Source"), this, SLOT(gotoContentSource(bool)));

    QIcon sourceIcon;
    sourceIcon.addPixmap(QPixmap(":/images/toolbar/internet.png"), QIcon::Normal, QIcon::On);
    mGotoContentSourceAction->setIcon(sourceIcon);

}

void UBGraphicsItemDelegate::updateMenuActionState()
{
    if (mLockAction)
        mLockAction->setChecked(isLocked());

    if (mShowOnDisplayAction)
    {
        bool isControl = mDelegated->data(UBGraphicsItemData::ItemLayerType) == UBItemLayerType::Control;
        mShowOnDisplayAction->setChecked(!isControl);
    }

    if (mGotoContentSourceAction)
    {
        UBItem* item = dynamic_cast<UBItem*>(mDelegated);
        mGotoContentSourceAction->setEnabled(item && !item->sourceUrl().isEmpty());
    }
}


void UBGraphicsItemDelegate::showMenu()
{
    if (!mMenu)
    {
        mMenu = new QMenu(UBApplication::boardController->controlView());
        decorateMenu(mMenu);
    }

    updateMenuActionState();

    UBBoardView* cv = UBApplication::boardController->controlView();
    QRect pinPos = cv->mapFromScene(mMenuButton->sceneBoundingRect()).boundingRect();

    mMenu->exec(cv->mapToGlobal(pinPos.bottomRight()));
}


