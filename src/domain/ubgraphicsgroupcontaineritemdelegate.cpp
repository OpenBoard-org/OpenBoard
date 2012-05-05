#include "ubgraphicsgroupcontaineritemdelegate.h"

#include <QtGui>

#include "UBGraphicsScene.h"
#include "gui/UBResources.h"

#include "domain/UBGraphicsDelegateFrame.h"
#include "domain/ubgraphicsgroupcontaineritem.h"

#include "board/UBBoardController.h"

#include "core/memcheck.h"

UBGraphicsGroupContainerItemDelegate::UBGraphicsGroupContainerItemDelegate(QGraphicsItem *pDelegated, QObject *parent) :
    UBGraphicsItemDelegate(pDelegated, parent), mDestroyGroupButton(0)

{

}

UBGraphicsGroupContainerItem *UBGraphicsGroupContainerItemDelegate::delegated()
{
    return dynamic_cast<UBGraphicsGroupContainerItem*>(mDelegated);
}

void UBGraphicsGroupContainerItemDelegate::decorateMenu(QMenu *menu)
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
}

void UBGraphicsGroupContainerItemDelegate::buildButtons()
{
    UBGraphicsItemDelegate::buildButtons();

    mDestroyGroupButton = new DelegateButton(":/images/font.svg", mDelegated, mFrame, Qt::TopLeftSection);

    mButtons << mDestroyGroupButton;

    connect(mDestroyGroupButton, SIGNAL(clicked()), (UBGraphicsGroupContainerItemDelegate*)this, SLOT(destroyGroup()));
}

void UBGraphicsGroupContainerItemDelegate::destroyGroup()
{
    qDebug() << "got an event";
    foreach (QGraphicsItem *item, delegated()->childItems()) {
        delegated()->removeFromGroup(item);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }

    remove(true);
}
