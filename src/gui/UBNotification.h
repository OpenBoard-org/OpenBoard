/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
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

#include <QFrame>
#include <QPointer>
#include <QTimer>

class QLabel;

/**
 * @brief A discreet notification in the bottom left corner of a view. It does not block: the
 * user can continue working.
 *
 * A warning stays until it is replaced or hidden; clicking it shows or hides its details. An
 * information is shown with its details for a few seconds, and clicking it closes it. The
 * notification is placed right of the left palette when that palette is visible, and does not
 * cover the stylus palette nor the messages shown in the same corner.
 */
class UBNotification : public QFrame
{
    Q_OBJECT

public:
    // The notification is a child of parent, placed at the bottom left of anchor (parent or one of
    // its descendants) and right of leftNeighbour when it is a visible child of parent.
    UBNotification(QWidget* parent, QWidget* anchor = nullptr, QWidget* leftNeighbour = nullptr);

    // status: what is being done, shown under the title; details: shown when the warning is clicked
    void showWarning(const QString& title, const QString& status, const QString& details);
    void showInformation(const QString& title, const QString& details);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void setExpanded(bool expanded);
    void updatePosition();
    QList<QWidget*> obstacles() const;

    QPointer<QWidget> mAnchor;
    QPointer<QWidget> mLeftNeighbour;
    QLabel* mIcon{nullptr};
    QLabel* mTitle{nullptr};
    QLabel* mStatus{nullptr};
    QLabel* mDetails{nullptr};
    QTimer mInformationTimer;
};
