#include "UBFlowLayout.h"
#include "qwidget.h"

// inspired by https://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html

UBFlowLayout::UBFlowLayout(QWidget* parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent)
    , m_hSpace(hSpacing)
    , m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

UBFlowLayout::UBFlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing)
    , m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

UBFlowLayout::~UBFlowLayout()
{
    QLayoutItem* item;
    while ((item = UBFlowLayout::takeAt(0)))
        delete item;
}

void UBFlowLayout::addItem(QLayoutItem* item)
{
    itemList.append(item);
}

void UBFlowLayout::insertItem(int index, QLayoutItem* item)
{
    itemList.insert(index, item);
}

int UBFlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0)
    {
        return m_hSpace;
    }
    else
    {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int UBFlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0)
    {
        return m_vSpace;
    }
    else
    {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int UBFlowLayout::count() const
{
    return itemList.size();
}

QLayoutItem* UBFlowLayout::itemAt(int index) const
{
    return itemList.value(index);
}

QLayoutItem* UBFlowLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);

    return nullptr;
}

QLayoutItem* UBFlowLayout::replaceAt(int index, QLayoutItem* item)
{
    if (!item)
    {
        return nullptr;
    }

    QLayoutItem* b = itemList.value(index);

    if (!b)
    {
        return nullptr;
    }

    itemList.replace(index, item);
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
    int height = doLayout(QRect(0, 0, width, 0), true);
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

    for (const QLayoutItem* item : qAsConst(itemList))
        size = size.expandedTo(item->minimumSize());

    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

int UBFlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (QLayoutItem* item : qAsConst(itemList))
    {
        const QWidget* wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1)
            spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
        {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int UBFlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject* parent = this->parent();

    if (!parent)
    {
        return -1;
    }
    else if (parent->isWidgetType())
    {
        QWidget* pw = static_cast<QWidget*>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    else
    {
        return static_cast<QLayout*>(parent)->spacing();
    }
}
