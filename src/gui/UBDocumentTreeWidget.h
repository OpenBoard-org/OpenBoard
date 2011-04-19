/*
 * UBDocumentTreeWidget.h
 *
 *  Created on: Dec 9, 2008
 *      Author: Luc
 */

#ifndef UBDOCUMENTTREEWIDGET_H_
#define UBDOCUMENTTREEWIDGET_H_

#include <QtGui>

class UBDocumentProxy;
class UBDocumentProxyTreeItem;

class UBDocumentTreeWidget : public QTreeWidget
{
    Q_OBJECT;

    public:
        UBDocumentTreeWidget(QWidget *parent = 0);
        virtual ~UBDocumentTreeWidget();

    protected:
        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dragLeaveEvent(QDragLeaveEvent *event);
        virtual void dropEvent(QDropEvent *event);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void dragMoveEvent(QDragMoveEvent *event);
        virtual void focusInEvent(QFocusEvent *event);
        virtual Qt::DropActions supportedDropActions() const;

    private slots:
        void documentUpdated(UBDocumentProxy *pDocument);

        void itemChangedValidation(QTreeWidgetItem * item, int column);

    private:
        UBDocumentProxyTreeItem *mSelectedProxyTi;
        QTreeWidgetItem *mDropTargetProxyTi;
        QBrush mBackground;
};


class UBDocumentProxyTreeItem : public QTreeWidgetItem
{
    public:

        UBDocumentProxyTreeItem(QTreeWidgetItem * parent, UBDocumentProxy* proxy, bool isEditable = true);

        QPointer<UBDocumentProxy> proxy() const
        {
            return mProxy;
        }

        QPointer<UBDocumentProxy> mProxy;
};

class UBDocumentGroupTreeItem : public QTreeWidgetItem
{
    public:

        UBDocumentGroupTreeItem(QTreeWidgetItem *parent, bool isEditable = true);
        virtual ~UBDocumentGroupTreeItem();

        void setGroupName(const QString &groupName);

        QString groupName() const;

        bool isTrashFolder() const;
        bool isDefaultFolder() const;
};

#endif /* UBDOCUMENTTREEWIDGET_H_ */
