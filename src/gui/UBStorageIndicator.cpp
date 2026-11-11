/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBStorageIndicator.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>
#include <QVBoxLayout>

namespace
{
    const int margin = 12;
    const int expandedWidth = 380;
    const int minimumExpandedWidth = 240;
}

UBStorageIndicator::UBStorageIndicator(QWidget* parent, QWidget* anchor, QWidget* leftNeighbour)
    : QFrame(parent)
    , mAnchor(anchor ? anchor : parent)
    , mLeftNeighbour(leftNeighbour)
{
    setObjectName("UBStorageIndicator");
    setStyleSheet("#UBStorageIndicator { background-color: #fff4ce; border: 1px solid #f5a623; border-radius: 6px; }"
                  "#UBStorageIndicator QLabel { color: black; background: transparent; }");
    setCursor(Qt::PointingHandCursor);

    mIcon = new QLabel(this);
    mIcon->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(20, 20));
    mIcon->setAlignment(Qt::AlignTop);

    mTitle = new QLabel(tr("Document location unavailable"), this);
    QFont font = mTitle->font();
    font.setBold(true);
    mTitle->setFont(font);

    mExplanation = new QLabel(tr("Your work is saved on this computer and copied automatically to the document "
                                 "location as soon as it is available again. You can continue working normally.\n\n"
                                 "Click to close this message."), this);
    mExplanation->setWordWrap(true);

    QVBoxLayout* text = new QVBoxLayout();
    text->setSpacing(6);
    text->addWidget(mTitle);
    text->addWidget(mExplanation);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 10, 6);
    layout->setSpacing(8);
    layout->addWidget(mIcon, 0, Qt::AlignTop);
    layout->addLayout(text);

    // follow the size of the views and of the left palette
    for (QWidget* widget = mAnchor; widget && widget != parent; widget = widget->parentWidget())
    {
        widget->installEventFilter(this);
    }

    parent->installEventFilter(this);

    if (mLeftNeighbour)
    {
        mLeftNeighbour->installEventFilter(this);
    }

    setExpanded(false);
    hide();
}

void UBStorageIndicator::setStorageAvailable(bool available)
{
    if (available)
    {
        hide();
        return;
    }

    setExpanded(false);
    show();
    raise();
}

void UBStorageIndicator::setObstacles(std::function<QList<QWidget*>()> obstacles)
{
    mObstacles = obstacles;
    updatePosition();
}

bool UBStorageIndicator::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Hide:
    case QEvent::ParentChange:
        if (isVisible() && watched != this)
        {
            updatePosition();

            // palettes may be raised above the badge
            if (watched != parentWidget())
            {
                raise();
            }
        }
        break;

    default:
        break;
    }

    return QFrame::eventFilter(watched, event);
}

void UBStorageIndicator::mousePressEvent(QMouseEvent* event)
{
    // not passed to the board: clicking the badge does not draw
    event->accept();
    setExpanded(mExplanation->isHidden());
}

void UBStorageIndicator::setExpanded(bool expanded)
{
    mExplanation->setVisible(expanded);
    setToolTip(expanded ? QString() : mExplanation->text().section("\n\n", 0, 0));
    updatePosition();
}

void UBStorageIndicator::updatePosition()
{
    QWidget* parent = parentWidget();

    if (!parent || !mAnchor)
    {
        return;
    }

    QRect area = mAnchor == parent ? parent->rect() : QRect(mAnchor->mapTo(parent, QPoint(0, 0)), mAnchor->size());
    int left = area.left() + margin;

    if (mLeftNeighbour && mLeftNeighbour->parentWidget() == parent && mLeftNeighbour->isVisible())
    {
        left = qMax(left, mLeftNeighbour->geometry().right() + margin);
    }

    const bool expanded = mExplanation->isVisibleTo(this);
    const auto heightFor = [this](int width) {
        const int height = heightForWidth(width);
        return height > 0 ? height : sizeHint().height();
    };

    int width = expanded ? expandedWidth : sizeHint().width();
    QRect geometry(left, 0, width, heightFor(width));
    geometry.moveBottom(area.bottom() - margin);

    // do not cover the palettes: narrower if there is enough room left of them, else above them
    const QList<QWidget*> obstacles = mObstacles ? mObstacles() : QList<QWidget*>();

    for (QWidget* obstacle : obstacles)
    {
        if (!obstacle || !parent->isAncestorOf(obstacle))
        {
            continue;
        }

        // e.g. a message is shown or hidden
        obstacle->installEventFilter(this);

        if (!obstacle->isVisible())
        {
            continue;
        }

        const QRect obstacleGeometry(obstacle->mapTo(parent, QPoint(0, 0)), obstacle->size());
        const QRect avoided = obstacleGeometry.adjusted(-margin, -margin, margin, margin);

        if (!geometry.intersects(avoided))
        {
            continue;
        }

        const int available = avoided.left() - left;

        if (expanded && available >= minimumExpandedWidth)
        {
            geometry.setWidth(available);
            geometry.setTop(geometry.bottom() - heightFor(available) + 1);
        }

        if (geometry.intersects(avoided))
        {
            geometry.moveBottom(avoided.top() - 1);
        }
    }

    setFixedSize(geometry.size());
    move(geometry.topLeft());
}
