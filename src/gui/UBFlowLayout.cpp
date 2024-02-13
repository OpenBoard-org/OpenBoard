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



#include "UBFlowLayout.h"

#include <QWidget>

// inspired by https://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html

UBFlowLayout::UBFlowLayout(QWidget* parent, int margin, int hSpacing, int vSpacing)
    : QLayout{parent}
    , mHSpace{hSpacing}
    , mVSpace{vSpacing}
{
    setContentsMargins(margin, margin, margin, margin);
}

UBFlowLayout::UBFlowLayout(int margin, int hSpacing, int vSpacing)
    : mHSpace{hSpacing}
    , mVSpace{vSpacing}
{
    setContentsMargins(margin, margin, margin, margin);
}

UBFlowLayout::~UBFlowLayout()
{
    QLayoutItem* item;

    while ((item = UBFlowLayout::takeAt(0)))
    {
        delete item;
    }
}

void UBFlowLayout::addItem(QLayoutItem* item)
{
    mItemList.append(item);
}

void UBFlowLayout::insertItem(int index, QLayoutItem* item)
{
    mItemList.insert(index, item);
}

int UBFlowLayout::horizontalSpacing() const
{
    if (mHSpace >= 0)
    {
        return mHSpace;
    }
    else
    {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int UBFlowLayout::verticalSpacing() const
{
    if (mVSpace >= 0)
    {
        return mVSpace;
    }
    else
    {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int UBFlowLayout::count() const
{
    return mItemList.size();
}

QLayoutItem* UBFlowLayout::itemAt(int index) const
{
    return mItemList.value(index);
}

QLayoutItem* UBFlowLayout::takeAt(int index)
{
    if (index >= 0 && index < mItemList.size())
    {
        return mItemList.takeAt(index);
    }

    return nullptr;
}

QLayoutItem* UBFlowLayout::replaceAt(int index, QLayoutItem* item)
{
    if (!item)
    {
        return nullptr;
    }

    auto b = mItemList.value(index);

    if (!b)
    {
        return nullptr;
    }

    mItemList.replace(index, item);
    invalidate();
    return b;
}

Qt::Orientations UBFlowLayout::expandingDirections() const
{
    return {};
}

bool UBFlowLayout::hasHeightForWidth() const
{
    return true;
}

int UBFlowLayout::heightForWidth(int width) const
{
    auto height = doLayout({0, 0, width, 0}, true);
    return height;
}

void UBFlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize UBFlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize UBFlowLayout::minimumSize() const
{
    QSize size;

    for (const auto item : qAsConst(mItemList))
    {
        size = size.expandedTo(item->minimumSize());
    }

    const auto margins = contentsMargins();
    size += QSize{margins.left() + margins.right(), margins.top() + margins.bottom()};
    return size;
}

int UBFlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    const auto effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    auto x = effectiveRect.x();
    auto y = effectiveRect.y();
    int lineHeight = 0;

    for (auto item : qAsConst(mItemList))
    {
        const auto wid = item->widget();
        auto spaceX = horizontalSpacing();

        if (spaceX == -1)
        {
            spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        }

        auto spaceY = verticalSpacing();

        if (spaceY == -1)
        {
            spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }

        auto nextX = x + item->sizeHint().width() + spaceX;

        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
        {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
        {
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }

    return y + lineHeight - rect.y() + bottom;
}

int UBFlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    auto parent = this->parent();

    if (!parent)
    {
        return -1;
    }
    else if (parent->isWidgetType())
    {
        const auto pw = static_cast<QWidget*>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    else
    {
        return static_cast<QLayout*>(parent)->spacing();
    }
}
