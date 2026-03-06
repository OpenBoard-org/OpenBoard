/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#include <QLayout>
#include <QAction>

#include "UBActionPalette.h"

#include "gui/shapes/UBAbstractSubPalette.h"

#include "core/memcheck.h"

UBActionPalette::UBActionPalette(QList<QAction*> actions, Qt::Orientation orientation, QWidget * parent)
    : UBFloatingPalette(Qt::TopRightCorner, parent)
{
    init(orientation);
    setActions(actions);
}


UBActionPalette::UBActionPalette(Qt::Orientation orientation, QWidget * parent)
     : UBFloatingPalette(Qt::TopRightCorner, parent)
{
    init(orientation);
}


UBActionPalette::UBActionPalette(QWidget * parent)
     : UBFloatingPalette(Qt::TopRightCorner, parent)
{
    init(Qt::Vertical);
}


UBActionPalette::UBActionPalette(Qt::Corner corner, QWidget * parent, Qt::Orientation orient)
     : UBFloatingPalette(corner, parent)
{
    init(orient);
}


void UBActionPalette::init(Qt::Orientation orientation)
{
    m_customCloseProcessing = false;

    mOrientation = orientation;
    mButtonSize = QSize(32, 32);
    mAutoClose = false;
    mActionGroup = nullptr;
    mToolButtonStyle = Qt::ToolButtonIconOnly;
    mButtons.clear();

    if (orientation == Qt::Horizontal)
        new QHBoxLayout(this);
    else
        new QVBoxLayout(this);

    updateLayout();
}

void UBActionPalette::setActions(QList<QAction*> actions)
{
    mMapActionToButton.clear();

    foreach(QAction* action, actions)
    {
        addAction(action);
    }

    actionChanged();
}

UBActionPaletteButton* UBActionPalette::createPaletteButton(QAction* action, QWidget *parent, UBAbstractSubPalette* subPalette)
{
    UBActionPaletteButton* button = subPalette
            ? new UBActionSubPaletteButton(action, parent, subPalette)
            : new UBActionPaletteButton(action, parent);

    button->setIconSize(mButtonSize);
    button->setToolButtonStyle(mToolButtonStyle);
    action->setProperty("id", mButtons.length());

    if (mActionGroup)
        mActionGroup->addAction(action);

    mButtons << button;

    mMapActionToButton[action] = button;

    connect(button, &UBActionPaletteButton::clicked,
            this, &UBActionPalette::buttonClicked);
    connect(action, &QAction::changed,
            this, &UBActionPalette::actionChanged);

    return button;
}

QAction* UBActionPalette::removePaletteButton(UBActionPaletteButton* button)
{
    if (!button)
    {
        return nullptr;
    }

    QAction* action = button->defaultAction();

    mButtons.removeAll(button);
    mActions.removeAll(action);
    mMapActionToButton.remove(action);

    if (mActionGroup)
    {
        mActionGroup->removeAction(action);
    }

    return action;
}

void UBActionPalette::addAction(QAction* action)
{
    UBActionPaletteButton* button = createPaletteButton(action, this);

    layout()->addWidget(button);

    mActions << action;
}

void UBActionPalette::attachSubPalette(QAction* action, UBAbstractSubPalette* subPalette, bool sameActionGroup)
{
    mSubPalette = subPalette;

    auto button = getButtonFromAction(action);

    if (button)
    {
        // use first action of subpalette for the new button
        action = subPalette->actions().first();

        // replace button by an UBActionSubPaletteButton
        mSubPaletteButton = new UBActionSubPaletteButton(action, this, subPalette);
        auto layoutItem = layout()->replaceWidget(button, mSubPaletteButton);

        mMapActionToButton[action] = mSubPaletteButton;

        const auto index = mButtons.indexOf(button);

        if (index >= 0)
        {
            mButtons.replace(index, mSubPaletteButton);
        }

        delete layoutItem;
        delete button;

        subPalette->setActionPaletteButtonParent(mSubPaletteButton);

        // set position of subpalette
        auto positionSubPalette = [this](){
            auto pos = mapToParent(mSubPaletteButton->pos());

            if (orientation() == Qt::Horizontal)
            {
                pos -= QPoint{0, mSubPalette->height() + height() - mSubPaletteButton->height()};

                if (pos.y() < 0)
                {
                    pos += QPoint{0, mSubPalette->height() + 2 * height() - mSubPaletteButton->height()};
                }
            }
            else
            {
                pos += QPoint{width(), 0};

                if (pos.x() + mSubPalette->width() > parentWidget()->width())
                {
                    pos -= QPoint{mSubPalette->width() + 2 * width() - mSubPaletteButton->width() , 0};
                }
            }

            mSubPalette->move(pos);
        };

        connect(this, &UBFloatingPalette::moving, this, positionSubPalette);
        connect(subPalette, &UBActionPalette::paletteVisible, this, positionSubPalette);

        // close subpalette when other action is clicked
        connect(this, &UBActionPalette::buttonGroupClicked, this, [this, action, subPalette](QAction* a){
            if (action != a)
            {
                subPalette->hide();
            }
        });

        if (sameActionGroup && mActionGroup)
        {
            // include subpalette actions in my action group
            for (auto action : subPalette->actions())
            {
                mActionGroup->addAction(action);
            }
        }
    }
}

void UBActionPalette::buttonClicked()
{
    if (mAutoClose)
    {
        close();
    }
}

QList<QAction*> UBActionPalette::actions()
{
    return mActions;
}


UBActionPalette::~UBActionPalette()
{
    qDeleteAll(mButtons.begin(), mButtons.end());
    mButtons.clear();

    if (mSubPalette)
    {
        delete mSubPalette;
    }
}


void UBActionPalette::setButtonIconSize(const QSize& size)
{
    foreach(QToolButton* button, mButtons)
        button->setIconSize(size);

    mButtonSize = size;
}


void UBActionPalette::groupActions()
{
    mActionGroup = new QActionGroup(this);
    int i = 0;
    foreach(QAction* action, mActions)
    {
        if (!action->property("ungrouped").toBool())
        {
            action->setProperty("id", i);
            mActionGroup->addAction(action);
            ++i;

            connect(action, &QAction::triggered, this, [this,action](bool checked){
                if (checked)
                {
                    emit buttonGroupClicked(action);
                }
            });
        }
    }
}


void UBActionPalette::setToolButtonStyle(Qt::ToolButtonStyle tbs)
{
    foreach(QToolButton* button, mButtons)
        button->setToolButtonStyle(tbs);

    mToolButtonStyle = tbs;

    updateLayout();

}

void UBActionPalette::updateLayout()
{
    if (mToolButtonStyle == Qt::ToolButtonIconOnly)
    {
        layout()->setContentsMargins (sLayoutContentMargin / 2  + border(), sLayoutContentMargin / 2  + border()
                , sLayoutContentMargin / 2  + border(), sLayoutContentMargin / 2  + border());
    }
    else
    {
        layout()->setContentsMargins (sLayoutContentMargin  + border(), sLayoutContentMargin  + border()
                , sLayoutContentMargin  + border(), sLayoutContentMargin + border());

    }
   update();
}


void UBActionPalette::setClosable(bool pClosable)
{
    UBFloatingPalette::setClosable(pClosable);

    updateLayout();
}


int UBActionPalette::border()
{
    if (isClosable())
        return 10;
    else
        return 5;
}


void UBActionPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);
}

void UBActionPalette::onCloseButtonClicked()
{
    close();
}


void UBActionPalette::close()
{
    if(!m_customCloseProcessing)
        hide();

    emit closed();
}


void UBActionPalette::mouseReleaseEvent(QMouseEvent * event)
{
    UBFloatingPalette::mouseReleaseEvent(event);
}

void UBActionPalette::showEvent(QShowEvent* event)
{
    UBFloatingPalette::showEvent(event);

    emit paletteVisible();
}


void UBActionPalette::actionChanged()
{
    for(int i = 0; i < mActions.length() && i < mButtons.length(); i++)
    {
        mButtons.at(i)->setVisible(mActions.at(i)->isVisible());
    }
}

void UBActionPalette::clearLayout()
{
    QLayout* pLayout = layout();
    if(NULL != pLayout)
    {
        while(!pLayout->isEmpty())
        {
            QLayoutItem* pItem = pLayout->itemAt(0);
            QWidget* pW = pItem->widget();
            pLayout->removeItem(pItem);
            delete pItem;
            pLayout->removeWidget(pW);
            delete pW;
        }

        mActions.clear();
        mButtons.clear();
    }
}

UBActionPaletteButton::UBActionPaletteButton(QAction* action, QWidget * parent)
    : QToolButton(parent)
{
    setIconSize(QSize(32, 32));
    setDefaultAction(action);
    setStyleSheet(QString("QToolButton {color: palette(button-text); font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));

    setFocusPolicy(Qt::NoFocus);

    setObjectName("ubActionPaletteButton");
}


UBActionPaletteButton::~UBActionPaletteButton()
{

}

void UBActionPaletteButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit doubleClicked();
}

QSize UBActionPalette::buttonSize()
{
    return mButtonSize;
}

Qt::Orientation UBActionPalette::orientation() const
{
    return mOrientation;
}

/**
 * \brief Returns the button related to the given action
 * @param action as the given action
 */
UBActionPaletteButton* UBActionPalette::getButtonFromAction(QAction *action)
{
    UBActionPaletteButton* pButton = NULL;

    pButton = mMapActionToButton.value(action);

    return pButton;
}

bool UBActionPaletteButton::hitButton(const QPoint &pos) const
{
    Q_UNUSED(pos);
    return true;
}

UBActionSubPaletteButton::UBActionSubPaletteButton(QAction* action, QWidget* parent, UBAbstractSubPalette* subPalette)
    : UBActionPaletteButton{action, parent}
    , mSubPalette{subPalette}
    , mPressedPos{new QPoint}
{
    connect(this, &QToolButton::pressed, this, &UBActionSubPaletteButton::buttonPressed);
    connect(this, &QToolButton::released, this, &UBActionSubPaletteButton::buttonReleased);
    connect(&mPressedTimer, &QTimer::timeout, mSubPalette, &QWidget::show);
}

bool UBActionSubPaletteButton::hitButton(const QPoint& pos) const
{
    *mPressedPos = pos;

    return true;
}

void UBActionSubPaletteButton::paintEvent(QPaintEvent* event)
{
    const auto arrowSize{6};
    const auto areaSize{10};

    QToolButton::paintEvent(event);

    UBActionPalette* palette = dynamic_cast<UBActionPalette*>(parentWidget());

    if (!palette)
    {
        return;
    }

    QPainter painter(this);
    QBrush brush{Qt::black};
    painter.setBrush(brush);
    QPainterPath path;

    if (palette->orientation() == Qt::Horizontal)
    {
        const auto tip = QPointF{size().width() / 2., 0.};
        mArrowRect = QRectF{{tip - QPointF{areaSize, 0}}, QSizeF{2 * areaSize, areaSize}};

        path.moveTo(tip);
        path.lineTo(tip + QPointF{-arrowSize, arrowSize});
        path.lineTo(tip + QPointF{arrowSize, arrowSize});
        path.lineTo(tip);
    }
    else
    {
        const auto tip = QPointF{size().width() - 1., size().height() / 2.};
        mArrowRect = QRectF{{tip - QPointF{areaSize, areaSize}}, QSizeF{areaSize, 2 * areaSize}};

        path.moveTo(tip);
        path.lineTo(tip + QPointF{-arrowSize, arrowSize});
        path.lineTo(tip - QPointF{arrowSize, arrowSize});
        path.lineTo(tip);
    }

    painter.drawPath(path);
}

void UBActionSubPaletteButton::buttonPressed()
{
    if (mArrowRect.contains(*mPressedPos))
    {
        mSubPalette->raise();
        mSubPalette->show();
        return;
    }

    mPressedTimer.setSingleShot(true);
    mPressedTimer.setInterval(350);
    mPressedTimer.start();
}

void UBActionSubPaletteButton::buttonReleased()
{
    mPressedTimer.stop();
}
