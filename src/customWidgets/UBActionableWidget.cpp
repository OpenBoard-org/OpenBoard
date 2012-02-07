#include <QPainter>
#include <QDebug>

#include "UBActionableWidget.h"

UBActionableWidget::UBActionableWidget(QWidget *parent, const char *name):QWidget(parent)
  , mShowActions(false)
{
    setObjectName(name);
    mActions.clear();
    mCloseButtons.setIcon(QIcon(QPixmap(":images/close.svg")));
    mCloseButtons.setGeometry(0, 0, 2*ACTIONSIZE, ACTIONSIZE);
    mCloseButtons.setVisible(false);
    connect(&mCloseButtons, SIGNAL(clicked()), this, SLOT(onCloseClicked()));
}

UBActionableWidget::~UBActionableWidget()
{

}

void UBActionableWidget::addAction(eAction act)
{
    if(!mActions.contains(act)){
        mActions << act;
    }
}

void UBActionableWidget::removeAction(eAction act)
{
    if(mActions.contains(act)){
        mActions.remove(mActions.indexOf(act));
    }
}

void UBActionableWidget::removeAllActions()
{
    mActions.clear();
}

void UBActionableWidget::setActionsVisible(bool bVisible)
{
    //mShowActions = bVisible;
    if(!mActions.empty() && mActions.contains(eAction_Close)){
        mCloseButtons.setVisible(bVisible);
    }
}

void UBActionableWidget::onCloseClicked()
{
    emit close(this);
}

bool UBActionableWidget::shouldClose(QPoint p)
{
    bool close = false;

//    if(mShowActions &&
//       p.x() >= 0 &&
//       p.x() <= ACTIONSIZE &&
//       p.y() >= 0 &&
//       p.y() <= ACTIONSIZE){
//       close = true;
//    }

    return close;
}

void UBActionableWidget::paintEvent(QPaintEvent* ev)
{
//    if(mShowActions){
//        QPainter p(this);
//        if(mActions.contains(eAction_Close)){
//            p.drawPixmap(0, 0, 16, 16, QPixmap(":images/close.svg"));
//        }else if(mActions.contains(eAction_MoveUp)){
//            // Implement me later
//        }else if(mActions.contains(eAction_MoveDown)){
//            // Implement me later
//        }
//    }
}

void UBActionableWidget::setActionsParent(QWidget *parent)
{
    if(mActions.contains(eAction_Close)){
        mCloseButtons.setParent(parent);
    }
}

void UBActionableWidget::unsetActionsParent()
{
    if(mActions.contains(eAction_Close)){
        mCloseButtons.setParent(this);
    }
}
