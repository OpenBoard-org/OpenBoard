
#include <QtGui>

#include "UBIconButton.h"

UBIconButton::UBIconButton(QWidget *parent, const QIcon &icon)
    : QAbstractButton(parent)
    , mToggleable(false)
{
    setIcon(icon);
    setCheckable(true);
    mIconSize = icon.actualSize(QSize(128, 128));
}

void UBIconButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pixmap = icon().pixmap(mIconSize, isChecked() ? QIcon::Selected : QIcon::Normal);
    painter.drawPixmap((width() - pixmap.width()) / 2, 0, pixmap);
}

void UBIconButton::mousePressEvent(QMouseEvent *event)
{
    if (!mToggleable)
        setChecked(true);

    QAbstractButton::mousePressEvent(event);
}

void UBIconButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit doubleClicked();
}

QSize UBIconButton::minimumSizeHint() const
{
    return mIconSize;
}
