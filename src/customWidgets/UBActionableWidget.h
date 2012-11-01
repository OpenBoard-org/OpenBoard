/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
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
