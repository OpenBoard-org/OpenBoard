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


#include "UBNotification.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>
#include <QVBoxLayout>

#include "gui/UBMessageWindow.h"
#include "gui/UBStylusPalette.h"

static const int margin = 12;
static const int wrappedWidth = 380;
static const int minimumWrappedWidth = 240;

UBNotification::UBNotification(QWidget* parent, QWidget* anchor, QWidget* leftNeighbour)
    : QFrame(parent)
    , mAnchor(anchor ? anchor : parent)
    , mLeftNeighbour(leftNeighbour)
{
    setObjectName("UBNotification");
    setCursor(Qt::PointingHandCursor);

    mIcon = new QLabel(this);
    mIcon->setAlignment(Qt::AlignTop);

    mTitle = new QLabel(this);
    QFont font = mTitle->font();
    font.setBold(true);
    mTitle->setFont(font);

    mStatus = new QLabel(this);
    mStatus->setWordWrap(true);

    mDetails = new QLabel(this);
    mDetails->setWordWrap(true);

    mInformationTimer.setSingleShot(true);
    mInformationTimer.setInterval(10000);
    connect(&mInformationTimer, &QTimer::timeout, this, &QWidget::hide);

    QVBoxLayout* text = new QVBoxLayout();
    text->setSpacing(6);
    text->addWidget(mTitle);
    text->addWidget(mStatus);
    text->addWidget(mDetails);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 10, 6);
    layout->setSpacing(8);
    layout->addWidget(mIcon, 0, Qt::AlignTop);
    layout->addLayout(text, 1);

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

void UBNotification::showWarning(const QString& title, const QString& status, const QString& details)
{
    mInformationTimer.stop();

    setStyleSheet("#UBNotification { background-color: #fff4ce; border: 1px solid #f5a623; border-radius: 6px; }"
                  "#UBNotification QLabel { color: black; background: transparent; }");
    mIcon->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(20, 20));
    mTitle->setText(title);
    mStatus->setText(status);
    mStatus->setVisible(!status.isEmpty());
    mDetails->setText(details);

    show();
    raise();
    setExpanded(false);
}

void UBNotification::showInformation(const QString& title, const QString& details)
{
    setStyleSheet("#UBNotification { background-color: #e5f1fb; border: 1px solid #2b88d8; border-radius: 6px; }"
                  "#UBNotification QLabel { color: black; background: transparent; }");
    mIcon->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(20, 20));
    mTitle->setText(title);
    mStatus->hide();
    mDetails->setText(details);

    show();
    raise();
    setExpanded(true);
    mInformationTimer.start();
}

bool UBNotification::eventFilter(QObject* watched, QEvent* event)
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

            // palettes may be raised above the notification
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

void UBNotification::mousePressEvent(QMouseEvent* event)
{
    // not passed to the board: clicking the notification does not draw
    event->accept();

    if (mInformationTimer.isActive())
    {
        mInformationTimer.stop();
        hide();
        return;
    }

    setExpanded(mDetails->isHidden());
}

void UBNotification::setExpanded(bool expanded)
{
    mDetails->setVisible(expanded);
    setToolTip(expanded ? QString() : mDetails->text().section("\n\n", 0, 0));
    updatePosition();
}

void UBNotification::updatePosition()
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

    // the status and the details wrap; a short text keeps the notification narrow
    const bool wraps = mStatus->isVisibleTo(this) || mDetails->isVisibleTo(this);
    const auto heightFor = [this](int width) {
        const int height = heightForWidth(width);
        return height > 0 ? height : sizeHint().height();
    };

    int width = wraps ? qMin(wrappedWidth, sizeHint().width()) : sizeHint().width();
    QRect geometry(left, 0, width, heightFor(width));
    geometry.moveBottom(area.bottom() - margin);

    // do not cover the palettes: narrower if there is enough room left of them, else above them
    for (QWidget* obstacle : obstacles())
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

        if (wraps && available >= minimumWrappedWidth)
        {
            geometry.setWidth(available);
            geometry.setTop(geometry.bottom() - heightFor(available) + 1);
        }

        if (geometry.intersects(avoided))
        {
            geometry.moveBottom(avoided.top() - 1);
        }
    }

    // not setFixedSize(): it would also bound the size hint of the next texts to this size
    setGeometry(geometry);
}

QList<QWidget*> UBNotification::obstacles() const
{
    QList<QWidget*> obstacles;

    // messages are shown in the same corner
    for (UBMessageWindow* message : mAnchor->findChildren<UBMessageWindow*>(Qt::FindDirectChildrenOnly))
    {
        obstacles << message;
    }

    // the stylus palette of the board, created again when its orientation changes
    for (UBStylusPalette* stylusPalette : parentWidget()->findChildren<UBStylusPalette*>(Qt::FindDirectChildrenOnly))
    {
        obstacles << stylusPalette;
    }

    return obstacles;
}
