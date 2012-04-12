
#include <QApplication>
#include <QStyleOptionButton>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QModelIndex>
#include "UBTGWidgetTreeDelegate.h"

UBTGWidgetTreeDelegate::UBTGWidgetTreeDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    //NOOP
}

void UBTGWidgetTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if(index.data(Qt::UserRole) != eUBTGAddSubItemWidgetType_None){
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setBackground(QBrush(QColor(Qt::red)));
        QStyleOptionButton styleButton;
        styleButton.text = "pipo";
        styleButton.rect = option.rect;
        QApplication::style()->drawControl(QStyle::CE_PushButtonLabel,&styleButton,painter);
    }
    else
        QStyledItemDelegate::paint(painter,option,index);
}

QSize UBTGWidgetTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option,index);
    return size;
}
