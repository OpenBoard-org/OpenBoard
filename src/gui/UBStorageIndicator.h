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


#ifndef UBSTORAGEINDICATOR_H_
#define UBSTORAGEINDICATOR_H_

#include <QFrame>
#include <QPointer>
#include <QTimer>

#include <functional>

class QLabel;

/**
 * A discreet badge in the bottom left corner of a view, shown while the document location
 * is not available. It does not block: the teacher can continue working.
 *
 * Clicking the badge shows or hides the explanation, which stays until the badge is clicked
 * again. When the location is available again, the badge tells it for a few seconds, as an
 * information. The badge is placed right of the left palette when that palette is visible.
 */
class UBStorageIndicator : public QFrame
{
    Q_OBJECT

public:
    // The badge is a child of parent, placed at the bottom left of anchor (parent or one of
    // its descendants) and right of leftNeighbour when it is a visible child of parent.
    UBStorageIndicator(QWidget* parent, QWidget* anchor = nullptr, QWidget* leftNeighbour = nullptr);

    void setStorageAvailable(bool available);

    // descendants of parent which must not be covered, e.g. the stylus palette or messages
    void setObstacles(std::function<QList<QWidget*>()> obstacles);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void showUnavailable();
    void showAvailableAgain();
    void setExpanded(bool expanded);
    void updatePosition();

    QPointer<QWidget> mAnchor;
    QPointer<QWidget> mLeftNeighbour;
    std::function<QList<QWidget*>()> mObstacles;
    QLabel* mIcon;
    QLabel* mTitle;
    QLabel* mExplanation;
    QTimer mInformationTimer;
    bool mUnavailable{false};
};

#endif /* UBSTORAGEINDICATOR_H_ */
