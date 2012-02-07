#include <QPainter>
#include <QDebug>

#include "UBActionableWidget.h"

UBActionableWidget::UBActionableWidget(QWidget *parent, const char *name):QWidget(parent)
  , mShowActions(false)
{
    setObjectName(name);
    mActions.clear();
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
    mShowActions = bVisible;
}

bool UBActionableWidget::shouldClose(QPoint p)
{
    qDebug() << "Should close: " << p.x() << "," << p.y();
    bool close = false;

    if(mShowActions &&
       p.x() >= 0 &&
       p.x() <= ACTIONSIZE &&
       p.y() >= 0 &&
       p.y() <= ACTIONSIZE){
       close = true;
    }

    return close;
}

void UBActionableWidget::paintEvent(QPaintEvent* ev)
{
    Q_UNUSED(ev);
    if(mShowActions){
        QPainter p(this);
        if(mActions.contains(eAction_Close)){
            p.drawPixmap(0, 0, 16, 16, QPixmap(":images/close.svg"));
        }else if(mActions.contains(eAction_MoveUp)){
            // Implement me later
        }else if(mActions.contains(eAction_MoveDown)){
            // Implement me later
        }
    }
}
