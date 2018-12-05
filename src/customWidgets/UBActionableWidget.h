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



#ifndef UBACTIONABLEWIDGET_H
#define UBACTIONABLEWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QToolButton>
#include <QPushButton>

#define ACTIONSIZE  16

typedef enum{
    eAction_Close,
    eAction_MoveUp,
    eAction_MoveDown
}eAction;

class UBActionableWidget : public QWidget
{
    Q_OBJECT
public:
    UBActionableWidget(QWidget* parent=0, const char* name="UBActionableWidget");
    ~UBActionableWidget();
    void addAction(eAction act);
    void removeAction(eAction act);
    void removeAllActions();
    void setActionsVisible(bool bVisible);

signals:
    void close(QWidget* w);

protected:
    void setActionsParent(QWidget* parent);
    void unsetActionsParent();
    QVector<eAction> mActions;
    QPushButton mCloseButtons;

private slots:
    void onCloseClicked();

private:
    bool mShowActions;

};

#endif // UBACTIONABLEWIDGET_H
