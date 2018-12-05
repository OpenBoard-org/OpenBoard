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

#ifndef WBSEARCHLINEEDIT_H
#define WBSEARCHLINEEDIT_H

#include "WBUrlLineEdit.h"

#include <QtGui>
#include <QAbstractButton>

class WBSearchButton;

/*
    Clear button on the right hand side of the search widget.
    Hidden by default
    "A circle with an X in it"
 */
class WBClearButton : public QAbstractButton
{
    Q_OBJECT;

    public:
        WBClearButton(QWidget *parent = 0);
        void paintEvent(QPaintEvent *event);

    public slots:
        void textChanged(const QString &text);
};


class WBSearchLineEdit : public WBExLineEdit
{
    Q_OBJECT;
    Q_PROPERTY(QString inactiveText READ inactiveText WRITE setInactiveText)

    signals:
        void textChanged(const QString &text);

    public:
        WBSearchLineEdit(QWidget *parent = 0);

        QString inactiveText() const;
        void setInactiveText(const QString &text);

        QMenu *menu() const;
        void setMenu(QMenu *menu);

        void setVisible(bool pVisible);

    protected:
        void resizeEvent(QResizeEvent *event);
        void paintEvent(QPaintEvent *event);

    private:
        void updateGeometries();

        WBSearchButton *mSearchButton;
        QString mInactiveText;
};

#endif // WBSEARCHLINEEDIT_H

