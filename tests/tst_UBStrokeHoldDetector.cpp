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


#include <QtTest>

#include "frameworks/UBStrokeHoldDetector.h"

namespace
{
    constexpr qreal tolerance = 5.;
    constexpr int delay = 300;
}

class TestUBStrokeHoldDetector : public QObject
{
    Q_OBJECT

private slots:
    void isInactiveInitially();
    void emitsHeldWhenPenIsStill();
    void ignoresMovementWithinTolerance();
    void restartsDelayWhenPenMoves();
    void doesNotEmitWhileDrawing();
    void emitsOncePerHold();
    void emitsAgainAfterMovingOn();
    void doesNotEmitAfterStop();
    void recordsPoints();
    void ignoresMovesWhenInactive();
    void startResetsPreviousStroke();
};

void TestUBStrokeHoldDetector::isInactiveInitially()
{
    UBStrokeHoldDetector detector;
    QVERIFY(!detector.isActive());
    QVERIFY(detector.points().isEmpty());
}

void TestUBStrokeHoldDetector::emitsHeldWhenPenIsStill()
{
    UBStrokeHoldDetector detector;
    QSignalSpy spy(&detector, &UBStrokeHoldDetector::held);

    QElapsedTimer elapsed;
    elapsed.start();
    detector.start({0, 0}, tolerance, delay);
    QVERIFY(detector.isActive());

    QVERIFY(spy.wait(10 * delay));
    QCOMPARE(spy.count(), 1);
    // a coarse timer may fire up to 5% early
    QVERIFY(elapsed.elapsed() >= delay * 95 / 100 - 1);
}

void TestUBStrokeHoldDetector::ignoresMovementWithinTolerance()
{
    UBStrokeHoldDetector detector;
    QSignalSpy spy(&detector, &UBStrokeHoldDetector::held);

    detector.start({100, 100}, tolerance, delay);

    // an unsteady hand: jitter around the hold position does not restart the delay
    for (int i = 0; i < 8; ++i)
    {
        QTest::qWait(delay / 10);
        detector.move(QPointF(100 + (i % 2 ? 3 : -3), 100 + (i % 3) - 1));
    }

    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, delay);
}

void TestUBStrokeHoldDetector::restartsDelayWhenPenMoves()
{
    UBStrokeHoldDetector detector;
    QSignalSpy spy(&detector, &UBStrokeHoldDetector::held);

    detector.start({0, 0}, tolerance, delay);
    QTest::qWait(delay * 6 / 10);
    detector.move({20, 0});

    // without the move, the signal would be emitted now
    QTest::qWait(delay * 6 / 10);
    QCOMPARE(spy.count(), 0);

    QVERIFY(spy.wait(10 * delay));
    QCOMPARE(spy.count(), 1);
}

void TestUBStrokeHoldDetector::doesNotEmitWhileDrawing()
{
    UBStrokeHoldDetector detector;
    QSignalSpy spy(&detector, &UBStrokeHoldDetector::held);

    detector.start({0, 0}, tolerance, delay);

    for (int i = 1; i <= 30; ++i)
    {
        QTest::qWait(delay / 10);
        detector.move(QPointF(i * 10, 0));
    }

    QCOMPARE(spy.count(), 0);
}

void TestUBStrokeHoldDetector::emitsOncePerHold()
{
    UBStrokeHoldDetector detector;
    QSignalSpy spy(&detector, &UBStrokeHoldDetector::held);

    detector.start({0, 0}, tolerance, delay);
    QVERIFY(spy.wait(10 * delay));

    // keep holding still
    detector.move({1, 1});
    QTest::qWait(3 * delay);
    QCOMPARE(spy.count(), 1);
}

void TestUBStrokeHoldDetector::emitsAgainAfterMovingOn()
{
    UBStrokeHoldDetector detector;
    QSignalSpy spy(&detector, &UBStrokeHoldDetector::held);

    detector.start({0, 0}, tolerance, delay);
    QVERIFY(spy.wait(10 * delay));

    detector.move({50, 50});
    QVERIFY(spy.wait(10 * delay));
    QCOMPARE(spy.count(), 2);
}

void TestUBStrokeHoldDetector::doesNotEmitAfterStop()
{
    UBStrokeHoldDetector detector;
    QSignalSpy spy(&detector, &UBStrokeHoldDetector::held);

    detector.start({0, 0}, tolerance, delay);
    QTest::qWait(delay / 2);
    detector.stop();
    QVERIFY(!detector.isActive());

    QTest::qWait(2 * delay);
    QCOMPARE(spy.count(), 0);
}

void TestUBStrokeHoldDetector::recordsPoints()
{
    UBStrokeHoldDetector detector;
    detector.start({1, 2}, tolerance, delay);
    detector.move({3, 4});
    detector.move({3.5, 4}); // within tolerance, still recorded
    detector.move({30, 40});

    const QList<QPointF> expected{{1, 2}, {3, 4}, {3.5, 4}, {30, 40}};
    QCOMPARE(detector.points(), expected);

    // the points are kept after stop, so that they can still be used
    detector.stop();
    QCOMPARE(detector.points(), expected);
}

void TestUBStrokeHoldDetector::ignoresMovesWhenInactive()
{
    UBStrokeHoldDetector detector;
    QSignalSpy spy(&detector, &UBStrokeHoldDetector::held);

    detector.move({10, 10});
    QVERIFY(detector.points().isEmpty());

    detector.start({0, 0}, tolerance, delay);
    detector.stop();
    detector.move({10, 10});
    QCOMPARE(detector.points().size(), 1);

    QTest::qWait(2 * delay);
    QCOMPARE(spy.count(), 0);
}

void TestUBStrokeHoldDetector::startResetsPreviousStroke()
{
    UBStrokeHoldDetector detector;
    detector.start({0, 0}, tolerance, delay);
    detector.move({100, 0});

    detector.start({5, 5}, tolerance, delay);
    QCOMPARE(detector.points(), QList<QPointF>{QPointF(5, 5)});
}

QTEST_GUILESS_MAIN(TestUBStrokeHoldDetector)

#include "tst_UBStrokeHoldDetector.moc"
