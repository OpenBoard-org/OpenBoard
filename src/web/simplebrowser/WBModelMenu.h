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




/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WBMODELMENU_H
#define WBMODELMENU_H

#include <QtGui>
#include <QMenu>

// A QMenu that is dynamically populated from a QAbstractItemModel
class WBModelMenu : public QMenu
{
    Q_OBJECT

    signals:
        void activated(const QModelIndex &index);
        void hovered(const QString &text);

    public:
        WBModelMenu(QWidget *parent = 0);

        void setModel(QAbstractItemModel *model);
        QAbstractItemModel *model() const;

        void setMaxRows(int max);
        int maxRows() const;

        void setFirstSeparator(int offset);
        int firstSeparator() const;

        void setRootIndex(const QModelIndex &index);
        QModelIndex rootIndex() const;

        void setHoverRole(int role);
        int hoverRole() const;

        void setSeparatorRole(int role);
        int separatorRole() const;

        QAction *makeAction(const QIcon &icon, const QString &text, QObject *parent);

    protected:
        // add any actions before the tree, return true if any actions are added.
        virtual bool prePopulated();
        // add any actions after the tree
        virtual void postPopulated();
        // put all of the children of parent into menu up to max
        void createMenu(const QModelIndex &parent, int max, QMenu *parentMenu = 0, QMenu *menu = 0);

    private slots:
        void aboutToShow();
        void triggered(QAction *action);
        void hovered(QAction *action);

    private:
        QAction *makeAction(const QModelIndex &index);
        int m_maxRows;
        int m_firstSeparator;
        int m_maxWidth;
        int m_hoverRole;
        int m_separatorRole;
        QAbstractItemModel *m_model;
        QPersistentModelIndex m_root;
};

#endif // WBMODELMENU_H

