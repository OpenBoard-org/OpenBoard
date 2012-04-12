#ifndef UBTGWIDGETTREEDELEGATE_H
#define UBTGWIDGETTREEDELEGATE_H

class QPainter;
class QStyleOptionViewItem;
class QModelIndex;

#include <QStyledItemDelegate>


typedef enum
{
    eUBTGAddSubItemWidgetType_None,
    eUBTGAddSubItemWidgetType_Action ,
    eUBTGAddSubItemWidgetType_Media,
    eUBTGAddSubItemWidgetType_Url
}eUBTGAddSubItemWidgetType;


class UBTGWidgetTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit UBTGWidgetTreeDelegate(QObject *parent = 0);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:

};

#endif // UBTGWIDGETTREEDELEGATE_H
