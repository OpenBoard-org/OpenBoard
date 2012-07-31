/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#include "UBGraphicsMediaItemDelegate.h"
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
#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsGroupContainerItem.h"

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

UBGraphicsItemDelegate::UBGraphicsItemDelegate(QGraphicsItem* pDelegated, QObject * parent, bool respectRatio, bool canRotate, bool useToolBar)
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
    , mToolBarItem(NULL)
    , mToolBarUsed(useToolBar)
{
    // NOOP
}

void UBGraphicsItemDelegate::init()
{
    if (mToolBarUsed)
        mToolBarItem = new UBGraphicsToolBarItem(mDelegated);

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

    mZOrderUpButton = new DelegateButton(":/images/z_layer_up.svg", mDelegated, mFrame, Qt::BottomLeftSection);
    connect(mZOrderUpButton, SIGNAL(clicked()), this, SLOT(increaseZLevelUp()));
    connect(mZOrderUpButton, SIGNAL(longClicked()), this, SLOT(increaseZlevelTop()));
    mButtons << mZOrderUpButton;

    mZOrderDownButton = new DelegateButton(":/images/z_layer_down.svg", mDelegated, mFrame, Qt::BottomLeftSection);
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
        mDelegated->setData(UBGraphicsItemData::ItemLocked, QVariant(isLocked()));
        updateFrame();

        if (UBStylusTool::Play != UBDrawingController::drawingController()->stylusTool())
            mFrame->show();

        updateButtons(true);

        if (mToolBarItem && mToolBarItem->isVisibleOnBoard())
        {
            mToolBarItem->positionHandles();
            mToolBarItem->update();
            mToolBarItem->show();
        }
    } else {
        foreach(DelegateButton* button, mButtons)
            button->hide();

        mFrame->hide();
        if (mToolBarItem)
            mToolBarItem->hide();
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

        /* this is performed because when removing delegated from scene while it contains flash content, segfault happens because of QGraphicsScene::removeItem() */ 
        UBGraphicsWebView *mDelegated_casted = static_cast<UBGraphicsWebView*>(mDelegated);
        if (mDelegated_casted)
            mDelegated_casted->setHtml(QString());
        scene->removeItem(mDelegated);

        if (canUndo)
        {
            UBGraphicsItemUndoCommand *uc = new UBGraphicsItemUndoCommand(scene, mDelegated, 0);
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
    positionHandles();
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


void UBGraphicsItemDelegate::buildButtons()
{
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
    mMinWidth(200),
    mInitialHeight(26),
    mElementsPadding(2)
{
    QRectF rect = this->rect();
    rect.setHeight(mInitialHeight);
    rect.setWidth(parent->boundingRect().width());
    this->setRect(rect);

  //  setBrush(QColor(UBSettings::paletteColor));             
    setPen(Qt::NoPen);
    hide();

    update();
}


void UBGraphicsToolBarItem::positionHandles()
{
    int itemXOffset = 0;
    foreach (QGraphicsItem* item, mItemsOnToolBar)
    {
        item->setPos(itemXOffset, 0);
        itemXOffset += (item->boundingRect().width()+mElementsPadding);
        item->show();
    }
}

void UBGraphicsToolBarItem::update()
{
    QGraphicsRectItem::update();
}

void UBGraphicsToolBarItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);  

    if (parentItem() && parentItem()->data(UBGraphicsItemData::ItemLocked).toBool())
    {
        QColor baseColor = UBSettings::paletteColor;
        baseColor.setAlphaF(baseColor.alphaF() / 3);
        setBrush(QBrush(baseColor));
    }
    else
    {
        setBrush(QBrush(UBSettings::paletteColor));
    }

    painter->fillPath(path, brush());
}

MediaTimer::MediaTimer(QGraphicsItem * parent): QGraphicsRectItem(parent) 
{}

MediaTimer::~MediaTimer()
{}

void MediaTimer::drawString(const QString &s, QPainter &p,
                                   QBitArray *newPoints, bool newString)
{
    QPoint  pos;

    int digitSpace = smallPoint ? 2 : 1;
    int xSegLen    = (rect().width()/1)*5/(ndigits*(5 + digitSpace) + digitSpace);
    int ySegLen    = rect().height()*5/12;
    int segLen     = ySegLen > xSegLen ? xSegLen : ySegLen;
    int xAdvance   = segLen*(5 + digitSpace)/5;
    int xOffset    = rect().x() + (rect().width()/1 - ndigits*xAdvance + segLen/5)/2;
    int yOffset    = (rect().height() - segLen*2)/2;

    for (int i=0;  i<ndigits; i++) {
        pos = QPoint(xOffset + xAdvance*i, yOffset);
        if (newString)
            drawDigit(pos, p, segLen, s[i].toLatin1(), digitStr[i].toLatin1());
        else
            drawDigit(pos, p, segLen, s[i].toLatin1());
        if (newPoints) {
            char newPoint = newPoints->testBit(i) ? '.' : ' ';
            if (newString) {
                char oldPoint = points.testBit(i) ? '.' : ' ';
                drawDigit(pos, p, segLen, newPoint, oldPoint);
            } else {
                drawDigit(pos, p, segLen, newPoint);
            }
        }
    }
    if (newString) {
        digitStr = s;
        digitStr.truncate(ndigits);
        if (newPoints)
            points = *newPoints;
    }
}

void MediaTimer::drawDigit(const QPoint &pos, QPainter &p, int segLen,
                                  char newCh, char oldCh)
{
     char updates[18][2];        // can hold 2 times number of segments, only
                                 // first 9 used if segment table is correct
     int  nErases;
     int  nUpdates;
     const char *segs;
     int  i,j;
 
     const char erase      = 0;
     const char draw       = 1;
     const char leaveAlone = 2;
 
     segs = getSegments(oldCh);
     for (nErases=0; segs[nErases] != 99; nErases++) {
         updates[nErases][0] = erase;            // get segments to erase to
         updates[nErases][1] = segs[nErases];    // remove old char
     }
     nUpdates = nErases;
     segs = getSegments(newCh);
     for(i = 0 ; segs[i] != 99 ; i++) {
         for (j=0;  j<nErases; j++)
             if (segs[i] == updates[j][1]) {   // same segment ?
                 updates[j][0] = leaveAlone;     // yes, already on screen
                 break;
             }
         if (j == nErases) {                   // if not already on screen
             updates[nUpdates][0] = draw;
             updates[nUpdates][1] = segs[i];
             nUpdates++;
         }
     }
     for (i=0; i<nUpdates; i++) {
         if (updates[i][0] == draw)
             drawSegment(pos, updates[i][1], p, segLen);
         if (updates[i][0] == erase)
             drawSegment(pos, updates[i][1], p, segLen, true);
     }
}

char* MediaTimer::getSegments(char ch)               // gets list of segments for ch
{
     char segments[30][8] =
        { 
              { 0, 1, 2, 4, 5, 6,99, 0},             // 0    0
              { 2, 5,99, 0, 0, 0, 0, 0},             // 1    1
              { 0, 2, 3, 4, 6,99, 0, 0},             // 2    2
              { 0, 2, 3, 5, 6,99, 0, 0},             // 3    3
              { 1, 2, 3, 5,99, 0, 0, 0},             // 4    4
              { 0, 1, 3, 5, 6,99, 0, 0},             // 5    5
              { 0, 1, 3, 4, 5, 6,99, 0},             // 6    6
              { 0, 2, 5,99, 0, 0, 0, 0},             // 7    7
              { 0, 1, 2, 3, 4, 5, 6,99},             // 8    8
              { 0, 1, 2, 3, 5, 6,99, 0},             // 9    9
              { 8, 9,99, 0, 0, 0, 0, 0},             // 10   :
              {99, 0, 0, 0, 0, 0, 0, 0}              // 11   empty
        };
 
     int n;
     if (ch >= '0' && ch <= '9')
        return segments[ch - '0'];
     if (ch == ':')
        n = 10;
     if (ch == ' ')
        n = 11;
     
     return segments[n];
}

void MediaTimer::drawSegment(const QPoint &pos, char segmentNo, QPainter &p,
                                    int segLen, bool erase)
{
    Q_UNUSED(erase);

    QPoint ppt;
    QPoint pt = pos;
    int width = segLen/5;
 
#define LINETO(X,Y) addPoint(a, QPoint(pt.x() + (X),pt.y() + (Y)))
#define LIGHT
#define DARK

    if (fill) {
        QPolygon a(0);
        switch (segmentNo) {
        case 0 :
            ppt = pt;
            LIGHT;
            LINETO(segLen - 1,0);
            DARK;
            LINETO(segLen - width - 1,width);
            LINETO(width,width);
            LINETO(0,0);
            break;
        case 1 :
            pt += QPoint(0 , 1);
            ppt = pt;
            LIGHT;
            LINETO(width,width);
            DARK;
            LINETO(width,segLen - width/2 - 2);
            LINETO(0,segLen - 2);
            LIGHT;
            LINETO(0,0);
            break;
        case 2 :
            pt += QPoint(segLen - 1 , 1);
            ppt = pt;
            DARK;
            LINETO(0,segLen - 2);
            LINETO(-width,segLen - width/2 - 2);
            LIGHT;
            LINETO(-width,width);
            LINETO(0,0);
            break;
        case 3 :
            pt += QPoint(0 , segLen);
            ppt = pt;
            LIGHT;
            LINETO(width,-width/2);
            LINETO(segLen - width - 1,-width/2);
            LINETO(segLen - 1,0);
            DARK;
            if (width & 1) {            // adjust for integer division error
                LINETO(segLen - width - 3,width/2 + 1);
                LINETO(width + 2,width/2 + 1);
            } else {
                LINETO(segLen - width - 1,width/2);
                LINETO(width,width/2);
            }
            LINETO(0,0);
            break;
        case 4 :
            pt += QPoint(0 , segLen + 1);
            ppt = pt;
            LIGHT;
            LINETO(width,width/2);
            DARK;
            LINETO(width,segLen - width - 2);
            LINETO(0,segLen - 2);
            LIGHT;
            LINETO(0,0);
            break;
        case 5 :
            pt += QPoint(segLen - 1 , segLen + 1);
            ppt = pt;
            DARK;
            LINETO(0,segLen - 2);
            LINETO(-width,segLen - width - 2);
            LIGHT;
            LINETO(-width,width/2);
            LINETO(0,0);
            break;
        case 6 :
            pt += QPoint(0 , segLen*2);
            ppt = pt;
            LIGHT;
            LINETO(width,-width);
            LINETO(segLen - width - 1,-width);
            LINETO(segLen - 1,0);
            DARK;
            LINETO(0,0);
            break;
        case 7 :
            pt += QPoint(segLen/2 , segLen*2);
            ppt = pt;
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
        case 8 :
            pt += QPoint(segLen/2 - width/2 + 1 , segLen/2 + width);
            ppt = pt;
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
        case 9 :
            pt += QPoint(segLen/2 - width/2 + 1 , 3*segLen/2 + width);
            ppt = pt;
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
        default :
            break;
        }
        // End exact copy
        p.setPen(Qt::white);
        p.setBrush(Qt::white);
        p.drawPolygon(a);
        p.setBrush(Qt::NoBrush);

        pt = pos;
    }
#undef LINETO
#undef LIGHT
#undef DARK
}

void MediaTimer::addPoint(QPolygon &a, const QPoint &p)
{
    uint n = a.size();
    a.resize(n + 1);
    a.setPoint(n, p);
}

void MediaTimer::paint(QPainter *p,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QFont f = p->font();
    f.setPointSizeF(f.pointSizeF());
    p->setFont(f);

    if (smallPoint)
        drawString(digitStr, *p, &points, false);
    else
        drawString(digitStr, *p, 0, false);
}

void MediaTimer::internalSetString(const QString& s)
{
    QString buffer;
    int i;
    int len = s.length();
    QBitArray newPoints(ndigits);

    if (!smallPoint) {
        if (len == ndigits)
            buffer = s;
        else
            buffer = s.right(ndigits).rightJustified(ndigits, QLatin1Char(' '));
    } else {
        int  index = -1;
        bool lastWasPoint = true;
        newPoints.clearBit(0);
        for (i=0; i<len; i++) {
            if (s[i] == QLatin1Char('.')) {
                if (lastWasPoint) {           // point already set for digit?
                    if (index == ndigits - 1) // no more digits
                        break;
                    index++;
                    buffer[index] = QLatin1Char(' ');        // 2 points in a row, add space
                }
                newPoints.setBit(index);        // set decimal point
                lastWasPoint = true;
            } else {
                if (index == ndigits - 1)
                    break;
                index++;
                buffer[index] = s[i];
                newPoints.clearBit(index);      // decimal point default off
                lastWasPoint = false;
            }
        }
        if (index < ((int) ndigits) - 1) {
            for(i=index; i>=0; i--) {
                buffer[ndigits - 1 - index + i] = buffer[i];
                newPoints.setBit(ndigits - 1 - index + i,
                                   newPoints.testBit(i));
            }
            for(i=0; i<ndigits-index-1; i++) {
                buffer[i] = QLatin1Char(' ');
                newPoints.clearBit(i);
            }
        }
    }

    if (buffer == digitStr)
        return;

    digitStr = buffer;
    if (smallPoint)
        points = newPoints;
    update();
}

void MediaTimer::init()
{
    val        = 0;
    smallPoint = false;
    setNumDigits(4);
}

void MediaTimer::display(const QString &s)
{
    val = 0;
    bool ok = false;
    double v = s.toDouble(&ok);
    if (ok)
        val = v;
    internalSetString(s);
}

void MediaTimer::setNumDigits(int numDigits)
{
    if (numDigits > 99) {
        qWarning("QLCDNumber::setNumDigits: (%s) Max 99 digits allowed");
        numDigits = 99;
    }
    if (numDigits < 0) {
        qWarning("QLCDNumber::setNumDigits: (%s) Min 0 digits allowed");
        numDigits = 0;
    }
    if (digitStr.isNull()) {                  // from constructor
        ndigits = numDigits;
        digitStr.fill(QLatin1Char(' '), ndigits);
        points.fill(0, ndigits);
        digitStr[ndigits - 1] = QLatin1Char('0');            // "0" is the default number
    } else {
        if (numDigits == ndigits)             // no change
            return;
        register int i;
        int dif;
        if (numDigits > ndigits) {            // expand
            dif = numDigits - ndigits;
            QString buf;
            buf.fill(QLatin1Char(' '), dif);
            digitStr.insert(0, buf);
            points.resize(numDigits);
            for (i=numDigits-1; i>=dif; i--)
                points.setBit(i, points.testBit(i-dif));
            for (i=0; i<dif; i++)
                points.clearBit(i);
        } else {                                        // shrink
            dif = ndigits - numDigits;
            digitStr = digitStr.right(numDigits);
            QBitArray tmpPoints = points;
            points.resize(numDigits);
            for (i=0; i<(int)numDigits; i++)
                points.setBit(i, tmpPoints.testBit(i+dif));
        }
        ndigits = numDigits;
        update();
    }
}

DelegateMediaControl::DelegateMediaControl(UBGraphicsMediaItem* pDelegated, QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , mDelegate(pDelegated)
    , mDisplayCurrentTime(false)
    , mCurrentTimeInMs(0)
    , mTotalTimeInMs(0)
    , mStartWidth(200)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setBrush(QBrush(Qt::white));
    setPen(Qt::NoPen);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    lcdTimer = new MediaTimer(this);
    lcdTimer->init();

    update();
}


void DelegateMediaControl::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    QPainterPath path;

    mLCDTimerArea.setHeight(rect().height());
    mLCDTimerArea.setWidth(rect().height());

    mSeecArea = rect();
    mSeecArea.setWidth(rect().width()-mLCDTimerArea.width()-2);

    path.addRoundedRect(mSeecArea, mSeecArea.height()/2, mSeecArea.height()/2);
    painter->fillPath(path, brush());

    qreal frameWidth = rect().height() / 2;
    int position = frameWidth;

    if (mTotalTimeInMs > 0)
    {
        position = frameWidth + ((mSeecArea.width() - (2 * frameWidth)) / mTotalTimeInMs) * mCurrentTimeInMs;
    }

    int clearance = 2;
    int radius = frameWidth-clearance;
    QRectF r(position - radius, clearance, radius * 2, radius * 2);

    painter->setBrush(UBSettings::documentViewLightColor);
    painter->drawEllipse(r);
}


QPainterPath DelegateMediaControl::shape() const
{
    QPainterPath path;
    path.addRoundedRect(rect(), rect().height()/ 2, rect().height()/2);
    return path;
}

void DelegateMediaControl::positionHandles()
{
    mLCDTimerArea.setWidth(parentItem()->boundingRect().height());
    mLCDTimerArea.setHeight(parentItem()->boundingRect().height());
    lcdTimer->setRect(mLCDTimerArea);
    lcdTimer->setPos(mSeecArea.width()-mLCDTimerArea.width(),0);

    mSeecArea.setWidth(rect().width()-mLCDTimerArea.width());

    QRectF selfRect = rect();
    selfRect.setHeight(parentItem()->boundingRect().height());
    setRect(selfRect);

    lcdTimer->setPos(rect().width() - mLCDTimerArea.width(), 0); 

}

void DelegateMediaControl::update()
{
    QTime t;
    t = t.addMSecs(mCurrentTimeInMs < 0 ? 0 : mCurrentTimeInMs);
    lcdTimer->display(t.toString("m:ss"));

    QGraphicsRectItem::update();
}

void DelegateMediaControl::updateTicker(qint64 time )
{
    mCurrentTimeInMs = time;
    update();
}


void DelegateMediaControl::totalTimeChanged(qint64 newTotalTime)
{
    mTotalTimeInMs = newTotalTime;
    update();
}


void DelegateMediaControl::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   qreal frameWidth =  mSeecArea.height()/2;
    if (boundingRect().contains(event->pos() - QPointF(frameWidth,0)) 
        && boundingRect().contains(event->pos() + QPointF(frameWidth,0)))
    {  
        mDisplayCurrentTime = true;
        seekToMousePos(event->pos());
        this->update();
        event->accept();
        emit used();
    }
}

void DelegateMediaControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qreal frameWidth = rect().height() / 2;
    if (boundingRect().contains(event->pos() - QPointF(frameWidth,0)) 
        && boundingRect().contains(event->pos() + QPointF(frameWidth,0)))
    {   
        seekToMousePos(event->pos());
        this->update();
        event->accept();
        emit used();
    }
}

void DelegateMediaControl::seekToMousePos(QPointF mousePos)
{
    qreal minX, length;
    qreal frameWidth = rect().height() / 2;

    minX = frameWidth;
    length = mSeecArea.width() - lcdTimer->rect().width();

    qreal mouseX = mousePos.x();
    if (mouseX >= (mSeecArea.width() - mSeecArea.height()/2))
        mouseX = mSeecArea.width() - mSeecArea.height()/2;

    if (mTotalTimeInMs > 0 && length > 0 && mDelegate
        && mDelegate->mediaObject() && mDelegate->mediaObject()->isSeekable())
    {
        qint64 tickPos = (mTotalTimeInMs/length)* (mouseX - minX);
        mDelegate->mediaObject()->seek(tickPos);

        //OSX is a bit lazy
        updateTicker(tickPos);
    }
    emit used();
}

void DelegateMediaControl::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mDisplayCurrentTime = false;
    this->update();
    event->accept();
}
