/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBDOCUMENTTREEWIDGET_H_
#define UBDOCUMENTTREEWIDGET_H_

#include <QtGui>
#include <QTreeWidget>

class UBDocumentProxy;
class UBDocumentProxyTreeItem;
class UBDocumentGroupTreeItem;

class UBDocumentTreeWidget : public QTreeWidget
{
    Q_OBJECT

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
        void autoScroll();

    private:
        bool moveDocument(UBDocumentProxyTreeItem* document, UBDocumentGroupTreeItem* destinationFolder);
        UBDocumentProxyTreeItem *mSelectedProxyTi;
        QTreeWidgetItem *mDropTargetProxyTi;
        QBrush mBackground;
        QTimer* mScrollTimer;
        int mScrollMagnitude;
};


class UBDocumentProxyTreeItem : public QTreeWidgetItem
{
    public:

        UBDocumentProxyTreeItem(QTreeWidgetItem * parent, UBDocumentProxy* proxy, bool isEditable = true);

        QPointer<UBDocumentProxy> proxy() const
        {
            return mProxy;
        }

        bool isInTrash();

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
