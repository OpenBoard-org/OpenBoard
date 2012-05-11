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
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsAudioItem.h"
#include "domain/UBGraphicsVideoItem.h"
#include "domain/ubgraphicsgroupcontaineritem.h"

#include "web/UBWebController.h"

#include "frameworks/UBFileSystemUtils.h"
#include "board/UBDrawingController.h"

#include "core/memcheck.h"

class UBGraphicsParaschoolEditorWidgetItem;

DelegateButton::DelegateButton(const QString & fileName, QGraphicsItem* pDelegated, QGraphicsItem * parent, Qt::WindowFrameSection section)
    : QGraphicsSvgItem(fileName, parent)
    , mDelegated(pDelegated)
    , mIsTransparentToMouseEvent(false)
    , mButtonAlignmentSection(section)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
}

DelegateButton::~DelegateButton()
{
    // NOOP
}

void DelegateButton::setFileName(const QString & fileName)
{
    QGraphicsSvgItem::setSharedRenderer(new QSvgRenderer (fileName, this));
}


void DelegateButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // make sure delegate is selected, to avoid control being hidden
    mPressedTime = QTime::currentTime();
//    mDelegated->setSelected(true);

    event->setAccepted(!mIsTransparentToMouseEvent);
 }

void DelegateButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    int timeto = qAbs(QTime::currentTime().msecsTo(mPressedTime));

    if (timeto < UBSettings::longClickInterval) {
        emit clicked();
    } else {
        emit longClicked();
    }

    event->setAccepted(!mIsTransparentToMouseEvent);
}

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
    , mFlippable(false)
{
    // NOOP
}

void UBGraphicsItemDelegate::init()
{
    //mToolBarItem = new UBGraphicsToolBarItem(delegated());

    mFrame = new UBGraphicsDelegateFrame(this, QRectF(0, 0, 0, 0), mFrameWidth, mRespectRatio);
    mFrame->hide();
    mFrame->setFlag(QGraphicsItem::ItemIsSelectable, true);

    mDeleteButton = new DelegateButton(":/images/close.svg", mDelegated, mFrame, Qt::TopLeftSection);
    mButtons << mDeleteButton;
    connect(mDeleteButton, SIGNAL(clicked()), this, SLOT(remove()));
    if (canDuplicate()){
        mDuplicateButton = new DelegateButton(":/images/duplicate.svg", mDelegated, mFrame, Qt::TopLeftSection);
        connect(mDuplicateButton, SIGNAL(clicked(bool)), this, SLOT(duplicate()));
        mButtons << mDuplicateButton;
    }
    mMenuButton = new DelegateButton(":/images/menu.svg", mDelegated, mFrame, Qt::TopLeftSection);
    connect(mMenuButton, SIGNAL(clicked()), this, SLOT(showMenu()));
    mButtons << mMenuButton;

    mZOrderUpButton = new DelegateButton(":/images/plus.svg", mDelegated, mFrame, Qt::BottomLeftSection);
    connect(mZOrderUpButton, SIGNAL(clicked()), this, SLOT(increaseZLevelUp()));
    connect(mZOrderUpButton, SIGNAL(longClicked()), this, SLOT(increaseZlevelTop()));
    mButtons << mZOrderUpButton;

    mZOrderDownButton = new DelegateButton(":/images/minus.svg", mDelegated, mFrame, Qt::BottomLeftSection);
    connect(mZOrderDownButton, SIGNAL(clicked()), this, SLOT(increaseZLevelDown()));
    connect(mZOrderDownButton, SIGNAL(longClicked()), this, SLOT(increaseZlevelBottom()));
    mButtons << mZOrderDownButton;

    buildButtons();

    foreach(DelegateButton* button, mButtons)
    {
        if (button->getSection() != Qt::TitleBarArea)
        {
        button->hide();
        button->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
}
}


UBGraphicsItemDelegate::~UBGraphicsItemDelegate()
{
    qDeleteAll(mButtons);
    // do not release mMimeData.
    // the mMimeData is owned by QDrag since the setMimeData call as specified in the documentation
}

QVariant UBGraphicsItemDelegate::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemChildAddedChange){

    }else if (change == QGraphicsItem::ItemSelectedHasChanged) {
        bool ok;
        bool selected = value.toUInt(&ok);
        if (ok) {
            UBGraphicsScene *ubScene = castUBGraphicsScene();
            if (ubScene) {
                if (selected) {
                    ubScene->setSelectedZLevel(delegated());
                } else {
                    ubScene->setOwnZlevel(delegated());
                }
            }
        }
    }

    if ((change == QGraphicsItem::ItemSelectedHasChanged
         || change == QGraphicsItem::ItemPositionHasChanged
         || change == QGraphicsItem::ItemTransformHasChanged)
        && mDelegated->scene())
        {
        mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());

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

UBGraphicsScene *UBGraphicsItemDelegate::castUBGraphicsScene()
{
    UBGraphicsScene *castScene = dynamic_cast<UBGraphicsScene*>(delegated()->scene());

    return castScene;
}

bool UBGraphicsItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mDragStartPosition = event->pos();

    startUndoStep();

    if (!delegated()->isSelected())
    {
        delegated()->setSelected(true);
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
    if(mMimeData) {
        QDrag* mDrag = new QDrag(event->widget());
        mDrag->setMimeData(mMimeData);
        if (!mDragPixmap.isNull()) {
            mDrag->setPixmap(mDragPixmap);
            mDrag->setHotSpot(mDragPixmap.rect().center());
        }
        mDrag->exec();
        mDragPixmap = QPixmap();

        return true;
    }

    if(isLocked()) {
        event->accept();
        return true;
    } else {
        return false;
    }
}

bool UBGraphicsItemDelegate::weelEvent(QGraphicsSceneWheelEvent *event)
{
    Q_UNUSED(event);
    if( delegated()->isSelected() )
    {
//        event->accept();
        return true;
    }
    else
    {
//        event->ignore();
        return false;
    }
}

bool UBGraphicsItemDelegate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    //Deselect all the rest selected items if no ctrl key modifier
    if (delegated()->scene()
            && delegated()->scene()->selectedItems().count()
            && event->modifiers() != Qt::ControlModifier) {
        foreach (QGraphicsItem *item, delegated()->scene()->selectedItems()) {
            if (item != delegated()) {
                item->setSelected(false);
            }
        }
    }

    commitUndoStep();

    return true;
}

void UBGraphicsItemDelegate::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
//    if (!mDelegated->isSelected()) {
//        setZOrderButtonsVisible(true);
//    }
}

void UBGraphicsItemDelegate::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
//    if (!mDelegated->isSelected()) {
//        setZOrderButtonsVisible(false);
//    }
}

QGraphicsItem *UBGraphicsItemDelegate::delegated()
{
    QGraphicsItem *curDelegate = 0;
    if (mDelegated->parentItem() && mDelegated->parentItem()->type() == UBGraphicsGroupContainerItem::Type) {
        curDelegate = mDelegated->parentItem(); // considering delegated item as an item's group which contains everything
    } else {
        curDelegate = mDelegated;
    }

    return curDelegate;
}

void UBGraphicsItemDelegate::positionHandles()
{
    if (mDelegated->isSelected()) {
        bool shownOnDisplay = mDelegated->data(UBGraphicsItemData::ItemLayerType).toInt() != UBItemLayerType::Control;
        showHide(shownOnDisplay);
        lock(isLocked());
        updateFrame();
        mFrame->show();

        updateButtons(true);

//        if (mToolBarItem->isVisibleOnBoard())
//        {
//        updateToolBar();
//           mToolBarItem->show();
//        }
    } else {
        foreach(DelegateButton* button, mButtons)
            button->hide();

        mFrame->hide();
//        mToolBarItem->hide();
    }
}

void UBGraphicsItemDelegate::setZOrderButtonsVisible(bool visible)
{
    if (visible) {
        updateFrame();
        updateButtons();

        QPointF newUpPoint = mFrame->mapToItem(mDelegated, mZOrderUpButton->pos());
        QPointF newDownPoint = mFrame->mapToItem(mDelegated, mZOrderDownButton->pos());


        mZOrderUpButton->setParentItem(mDelegated);
        mZOrderDownButton->setParentItem(mDelegated);

        mZOrderUpButton->setPos(newUpPoint + QPointF(0,0));
        mZOrderDownButton->setPos(newDownPoint + QPointF(0,0));

        mZOrderUpButton->show();
        mZOrderDownButton->show();

    } else {
        mZOrderUpButton->hide();
        mZOrderDownButton->hide();
    }
}

void UBGraphicsItemDelegate::remove(bool canUndo)
{
//    QGraphicsScene* scene = mDelegated->scene();
    UBGraphicsScene* scene = dynamic_cast<UBGraphicsScene*>(mDelegated->scene());
    if (scene)
    {
        foreach(DelegateButton* button, mButtons)
            scene->removeItem(button);

        scene->removeItem(mFrame);
        scene->removeItem(mDelegated);
        //scene->removeItem(mToolBarItem);

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

void UBGraphicsItemDelegate::increaseZLevelUp()
{
    UBGraphicsScene *curScene = castUBGraphicsScene();
    if (curScene) {
        curScene->changeZLevelTo(delegated(), UBZLayerController::up);
    }
}
void UBGraphicsItemDelegate::increaseZlevelTop()
{
    UBGraphicsScene *curScene = castUBGraphicsScene();
    if (curScene) {
        curScene->changeZLevelTo(delegated(), UBZLayerController::top);
    }
}
void UBGraphicsItemDelegate::increaseZLevelDown()
{
    UBGraphicsScene *curScene = castUBGraphicsScene();
    if (curScene) {
        curScene->changeZLevelTo(delegated(), UBZLayerController::down);
    }
}
void UBGraphicsItemDelegate::increaseZlevelBottom()
{
    UBGraphicsScene *curScene = castUBGraphicsScene();
    if (curScene) {
        curScene->changeZLevelTo(delegated(), UBZLayerController::bottom);
    }
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

void UBGraphicsItemDelegate::setFlippable(bool flippable)
{
    mFlippable = flippable;
}

bool UBGraphicsItemDelegate::isFlippable()
{
    return mFlippable;
}

void UBGraphicsItemDelegate::updateFrame()
{
    if (mFrame && !mFrame->scene() && mDelegated->scene())
    {
        mDelegated->scene()->addItem(mFrame);
    }

    mFrame->setAntiScale(mAntiScaleRatio);
    mFrame->positionHandles();
}

void UBGraphicsItemDelegate::updateButtons(bool showUpdated)
{
    QTransform tr;
    tr.scale(mAntiScaleRatio, mAntiScaleRatio);

    mDeleteButton->setParentItem(mFrame);
    mDeleteButton->setTransform(tr);

    qreal topX = mFrame->rect().left() - mDeleteButton->renderer()->viewBox().width() * mAntiScaleRatio / 2;
    qreal topY = mFrame->rect().top() - mDeleteButton->renderer()->viewBox().height() * mAntiScaleRatio / 2;

    qreal bottomX = mFrame->rect().left() - mDeleteButton->renderer()->viewBox().width() * mAntiScaleRatio / 2;
    qreal bottomY = mFrame->rect().bottom() - mDeleteButton->renderer()->viewBox().height() * mAntiScaleRatio / 2;

    mDeleteButton->setPos(topX, topY);

    if (!mDeleteButton->scene())
    {
        if (mDelegated->scene())
            mDelegated->scene()->addItem(mDeleteButton);
    }

    if (showUpdated /*&& mFrame->isResizing()*/)
        mDeleteButton->show();

    int i = 1, j = 0, k = 0;
    while ((i + j + k) < mButtons.size())  {
        DelegateButton* button = mButtons[i + j];

        if (button->getSection() == Qt::TopLeftSection) {
            button->setParentItem(mFrame);
            button->setPos(topX + (i++ * 1.6 * mFrameWidth * mAntiScaleRatio), topY);
            button->setTransform(tr);
        } else if (button->getSection() == Qt::BottomLeftSection) {
            button->setParentItem(mFrame);
            button->setPos(bottomX + (++j * 1.6 * mFrameWidth * mAntiScaleRatio), bottomY);
            button->setTransform(tr);
        } else if (button->getSection() == Qt::TitleBarArea || button->getSection() == Qt::NoSection){
            ++k;
        }
        if (!button->scene())
        {
            if (mDelegated->scene())
                mDelegated->scene()->addItem(button);
        }
        if (showUpdated) {
            button->show();
            button->setZValue(delegated()->zValue());
        }
    }
}

void UBGraphicsItemDelegate::updateToolBar()
{
    QTransform transformForToolbarButtons;
    transformForToolbarButtons.scale(mAntiScaleRatio, 1);

    QRectF toolBarRect = mToolBarItem->rect();
    toolBarRect.setWidth(delegated()->boundingRect().width() - 10);
    mToolBarItem->setRect(toolBarRect);

    if (mToolBarItem->isShifting())
        mToolBarItem->setPos(delegated()->boundingRect().bottomLeft() + QPointF(5 * mAntiScaleRatio, 0));
    else mToolBarItem->setPos(delegated()->boundingRect().bottomLeft() - QPointF(-5 * mAntiScaleRatio, mToolBarItem->rect().height() * 1.1 * mAntiScaleRatio));

    int offsetOnToolBar = 5 * mAntiScaleRatio;
    QList<QGraphicsItem*> itemList = mToolBarItem->itemsOnToolBar();
    foreach (QGraphicsItem* item, itemList)
    {
        item->setPos(offsetOnToolBar, 0);
        offsetOnToolBar += (item->boundingRect().width() + 5) * mAntiScaleRatio;
        item->setTransform(transformForToolbarButtons);
        item->show();
    }

    mToolBarItem->setOffsetOnToolBar(offsetOnToolBar);

    QTransform tr;
    tr.scale(1, mAntiScaleRatio);
    mToolBarItem->setTransform(tr);
}

void UBGraphicsItemDelegate::setButtonsVisible(bool visible)
{
    foreach(DelegateButton* pButton, mButtons){
        pButton->setVisible(visible);
    }
}

UBGraphicsToolBarItem::UBGraphicsToolBarItem(QGraphicsItem * parent) : 
    QGraphicsRectItem(parent),
    mShifting(true),
    mVisible(false),
    mMinWidth(200)
{
    QRectF rect = this->rect();
    rect.setHeight(26);
    this->setRect(rect);

    setBrush(QColor(UBSettings::paletteColor));             
    setPen(Qt::NoPen);
    hide();
}

void UBGraphicsToolBarItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);  

    painter->fillPath(path, brush());
}
