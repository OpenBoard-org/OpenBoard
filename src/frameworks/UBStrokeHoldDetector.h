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

#include <QList>
#include <QObject>
#include <QPointF>
#include <QTimer>

/**
 * @brief Detect when the pen is held still while drawing a stroke.
 *
 * The detector records the points of a stroke and emits held() when the
 * pen did not move by more than a tolerance during a given delay. Small
 * movements within the tolerance, as caused by an unsteady hand or a
 * noisy tablet, do not restart the delay.
 */
class UBStrokeHoldDetector : public QObject
{
    Q_OBJECT

public:
    explicit UBStrokeHoldDetector(QObject* parent = nullptr);

    /// start a new stroke at position. tolerance is in the unit of the positions, delay in milliseconds.
    void start(const QPointF& position, qreal tolerance, int delay);

    /// record a new position of the current stroke, ignored if not active
    void move(const QPointF& position);

    /// end the current stroke, held() will not be emitted any more
    void stop();

    bool isActive() const;

    /// the points recorded for the current stroke
    const QList<QPointF>& points() const;

signals:
    void held();

private:
    QTimer mTimer;
    QList<QPointF> mPoints;
    QPointF mAnchor;
    qreal mTolerance{0};
    bool mActive{false};
};
