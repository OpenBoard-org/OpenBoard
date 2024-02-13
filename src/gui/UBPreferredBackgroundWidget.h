/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
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

#include <QWidget>

// forward
class UBFlowLayout;
class QLabel;
class QLayoutItem;

class UBPreferredBackgroundWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UBPreferredBackgroundWidget(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void updateStyle() const;
    void savePreferrdBackgrounds() const;

private slots:
    void updateBackgroundButtons();

private:
    UBFlowLayout* mLayout{nullptr};
    QLabel* mPlaceholder{nullptr};
    QLabel* mTrash{nullptr};
    QLayoutItem* mDraggedItem{nullptr};
    int mDraggedItemPos{-1};

    class BackgroundWidget : public QWidget
    {
    public:
        explicit BackgroundWidget(const QPixmap& pixmap, const QString& text, QWidget* parent = nullptr);

        QPixmap pixmap() const;

    private:
        QPixmap mPixmap;
    };
};
