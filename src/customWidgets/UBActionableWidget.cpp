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
    if(!mActions.empty() && mActions.contains(eAction_Close)){
        mCloseButtons.setVisible(bVisible);
    }
}

void UBActionableWidget::onCloseClicked()
{
    emit close(this);
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
