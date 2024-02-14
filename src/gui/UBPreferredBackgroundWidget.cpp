/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
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



#include "UBPreferredBackgroundWidget.h"

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "gui/UBBackgroundManager.h"
#include "gui/UBFlowLayout.h"


UBPreferredBackgroundWidget::UBPreferredBackgroundWidget(QWidget* parent)
    : QWidget{parent}
{
    setAcceptDrops(true);

    mLayout = new UBFlowLayout{this, -1, 0, 0};
    mLayout->setSizeConstraint(QLayout::SetMaximumSize);
    setLayout(mLayout);

    mPlaceholder = new QLabel{this};
    QPixmap pixmap{QSize{128, 128}};
    pixmap.fill(QColor{Qt::transparent});

    QPainter painter{&pixmap};
    QPen pen = painter.pen();
    pen.setWidth(4);
    pen.setColor(QColor{0xbbbbbb});
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);
    painter.drawRoundedRect(6, 20, 115, 88, 15, 15);

    mPlaceholder->setPixmap(pixmap);
    mPlaceholder->setVisible(false);

    QPixmap trash{":/images/trash-document-page.png"};
    painter.drawPixmap({26, 30, 68, 68}, trash);

    mTrash = new QLabel{this};
    mTrash->setPixmap(pixmap);
    mTrash->setVisible(false);

    updateBackgroundButtons();
    updateStyle();

    connect(UBApplication::boardController->backgroundManager(), &UBBackgroundManager::backgroundListChanged, this,
            &UBPreferredBackgroundWidget::updateBackgroundButtons);
}

void UBPreferredBackgroundWidget::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
    QWidget::dragEnterEvent(event);
}

void UBPreferredBackgroundWidget::dragMoveEvent(QDragMoveEvent* event)
{
    // auto-scroll if widget is in a scroll area
    auto scrollArea = dynamic_cast<QScrollArea*>(parentWidget()->parentWidget());

    if (scrollArea)
    {
        const auto p = event->pos();
        scrollArea->ensureVisible(p.x(), p.y(), 15);
    }

    auto child = childAt(event->pos());

    // walk up to a direct child
    while (child && child->parentWidget() != this)
    {
        child = child->parentWidget();
    }

    if (!child)
    {
        return;
    }

    // is the child the placeholder or trash label?
    if (child == mPlaceholder || child == mTrash)
    {
        return;
    }

    // move the placeholder to the appropiate place
    const auto pl = mLayout->indexOf(mPlaceholder);
    const auto ix = mLayout->indexOf(child);

    if (pl >= 0 && ix >= 0)
    {
        auto plItem = mLayout->takeAt(pl);
        mLayout->insertItem(ix, plItem);
        updateStyle();
    }
}

void UBPreferredBackgroundWidget::dropEvent(QDropEvent* event)
{
    auto child = childAt(event->pos());

    // is the child the trash?
    if (child == mTrash)
    {
        // ask user whether to delete the background ruling
        const auto uuid = mDraggedItem->widget()->property("uuid").toUuid();
        auto bgManager = UBApplication::boardController->backgroundManager();
        const auto background = bgManager->background(uuid);
        const auto lang = QLocale().name().left(2);

        const auto answer =
            QMessageBox::question(this, tr("Delete background"),
                                  tr("Do you want to delete the background") + " " + background->description(lang));

        if (answer == QMessageBox::Yes)
        {
            bgManager->deleteBackground(uuid);
            delete mDraggedItem->widget();
            delete mDraggedItem;
            mDraggedItem = nullptr;
            mLayout->removeWidget(mPlaceholder);
            mPlaceholder->setVisible(false);
            mTrash->setVisible(false);
        }

        return;
    }

    // child must be placeholder
    if (child != mPlaceholder)
    {
        return;
    }

    auto ix = mLayout->indexOf(child);

    if (ix < 0)
    {
        return;
    }

    mLayout->replaceAt(ix, mDraggedItem);
    mLayout->removeWidget(mPlaceholder);
    mPlaceholder->setVisible(false);
    mTrash->setVisible(false);
    updateStyle();

    event->acceptProposedAction();
}

void UBPreferredBackgroundWidget::mousePressEvent(QMouseEvent* event)
{
    auto child = childAt(event->pos());

    // walk up to a direct child
    while (child && child->parentWidget() != this)
    {
        child = child->parentWidget();
    }

    auto backgroundWidget = dynamic_cast<BackgroundWidget*>(child);

    if (!backgroundWidget)
    {
        return;
    }

    mDraggedItemPos = mLayout->indexOf(backgroundWidget);
    auto item = new QWidgetItem{mPlaceholder};
    mDraggedItem = mLayout->replaceAt(mDraggedItemPos, item);
    mPlaceholder->setVisible(true);
    mDraggedItem->widget()->setVisible(false);

    const auto uuid = mDraggedItem->widget()->property("uuid").toUuid();
    const auto background = UBApplication::boardController->backgroundManager()->background(uuid);

    mTrash->setVisible(background && background->isUserProvided());

    auto pixmap = backgroundWidget->pixmap();

    auto mimeData = new QMimeData;
    auto drag = new QDrag{this};
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos() - backgroundWidget->pos());
    const auto dropAction = drag->exec(Qt::MoveAction);

    if (dropAction != Qt::MoveAction)
    {
        // abort DnD and put dragged item at original position
        if (mDraggedItem && mDraggedItemPos >= 0)
        {
            const auto ix = mLayout->indexOf(mPlaceholder);

            if (ix >= 0)
            {
                mLayout->takeAt(ix);
                mLayout->insertItem(mDraggedItemPos, mDraggedItem);
                mLayout->removeWidget(mPlaceholder);
                mPlaceholder->setVisible(false);
                mTrash->setVisible(false);
            }
        }
    }
    else
    {
        savePreferrdBackgrounds();
    }

    if (mDraggedItem && mDraggedItem->widget())
    {
        mDraggedItem->widget()->setVisible(true);
    }
}

void UBPreferredBackgroundWidget::updateStyle() const
{
    for (int i = 0; i < mLayout->count(); ++i)
    {
        auto widget = mLayout->itemAt(i)->widget();
        auto icon = widget->findChild<QLabel*>("backgroundIcon");

        if (icon)
        {
            icon->setStyleSheet(i < 6 ? "background-color: #bbddff" : "");
        }
    }
}

void UBPreferredBackgroundWidget::savePreferrdBackgrounds() const
{
    QList<QUuid> uuidList;

    for (int i = 0; i < mLayout->count() && i < 6; ++i)
    {
        uuidList << mLayout->itemAt(i)->widget()->property("uuid").toUuid();
    }

    UBApplication::boardController->backgroundManager()->savePreferredBackgrounds(uuidList);
}

void UBPreferredBackgroundWidget::updateBackgroundButtons()
{
    while (!mLayout->isEmpty())
    {
        auto item = mLayout->takeAt(0);
        auto widget = item->widget();

        if (widget != mPlaceholder && widget != mTrash)
        {
            delete widget;
        }

        delete item;
    }

    auto* bgManager = UBApplication::boardController->backgroundManager();
    const auto backgrounds = bgManager->backgrounds();

    for (const auto& background : backgrounds)
    {
        auto pixmap = bgManager->createButtonPixmap(*background, false, false);
        auto text = background->description(QLocale().name().left(2));
        auto widget = new BackgroundWidget{pixmap, text, this};
        widget->setProperty("uuid", background->uuid());
        widget->setToolTip(text);

        mLayout->addWidget(widget);
    }

    mLayout->addWidget(mTrash);
}

UBPreferredBackgroundWidget::BackgroundWidget::BackgroundWidget(const QPixmap& pixmap, const QString& text,
                                                                QWidget* parent)
    : QWidget{parent}
    , mPixmap{pixmap}
{
    auto icon = new QLabel{this};
    icon->setPixmap(pixmap);
    icon->setAlignment(Qt::AlignCenter);
    icon->setObjectName("backgroundIcon");

    auto label = new QLabel{this};
    label->setText(text);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setMaximumWidth(pixmap.width());

    auto layout = new QVBoxLayout{this};
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(icon);
    layout->addWidget(label);
    setLayout(layout);
}

QPixmap UBPreferredBackgroundWidget::BackgroundWidget::pixmap() const
{
    return mPixmap;
}
