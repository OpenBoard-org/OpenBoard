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


#include "UBStrokeHoldDetector.h"

#include <QLineF>

UBStrokeHoldDetector::UBStrokeHoldDetector(QObject* parent)
    : QObject(parent)
{
    mTimer.setSingleShot(true);
    connect(&mTimer, &QTimer::timeout, this, &UBStrokeHoldDetector::held);
}

void UBStrokeHoldDetector::start(const QPointF& position, qreal tolerance, int delay)
{
    mPoints = {position};
    mAnchor = position;
    mTolerance = tolerance;
    mActive = true;
    mTimer.start(delay);
}

void UBStrokeHoldDetector::move(const QPointF& position)
{
    if (!mActive)
    {
        return;
    }

    mPoints << position;

    if (QLineF(mAnchor, position).length() > mTolerance)
    {
        // the pen moved, wait for the next hold
        mAnchor = position;
        mTimer.start();
    }
}

void UBStrokeHoldDetector::stop()
{
    mActive = false;
    mTimer.stop();
}

bool UBStrokeHoldDetector::isActive() const
{
    return mActive;
}

const QList<QPointF>& UBStrokeHoldDetector::points() const
{
    return mPoints;
}
