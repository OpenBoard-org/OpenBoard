/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#pragma once

#include <QToolBar>
#include <QWidget>

class UBToolbarExtensionPalette : public QWidget
{
    Q_OBJECT

public:
    explicit UBToolbarExtensionPalette(QToolBar* toolbar = nullptr, QWidget* parent = nullptr);

    void setSpan(QWidget* from, QWidget* to);

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private slots:
    void updatePosition(QVariant atTop);

private:
    QToolBar* mToolBar{nullptr};
    QWidget* mFromWidget{nullptr};
    bool mToolbarAtTop{true};
};
