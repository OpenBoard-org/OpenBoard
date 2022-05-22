/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBGRAPHICSMEDIAITEMDELEGATE_H_
#define UBGRAPHICSMEDIAITEMDELEGATE_H_

#include <QtGui>
#include <QTimer>
#include <QtMultimedia>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"

class QGraphicsSceneMouseEvent;
class QGraphicsItem;

class UBGraphicsMediaItemDelegate :  public UBGraphicsItemDelegate
{
    Q_OBJECT

    public:
        UBGraphicsMediaItemDelegate(UBGraphicsMediaItem* pDelegated, QObject * parent = 0);
        virtual ~UBGraphicsMediaItemDelegate();

        virtual void positionHandles();

        bool mousePressEvent(QGraphicsSceneMouseEvent* event);

        void showToolBar(bool autohide = true);

    public slots:

        void toggleMute();
        void updateTicker(qint64 time);
        virtual void showHide(bool show);

        void mediaStatusChanged(QMediaPlayer::MediaStatus status);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        void mediaStateChanged(QMediaPlayer::PlaybackState state);
#else
        void mediaStateChanged(QMediaPlayer::State state);
#endif

    protected slots:

        virtual void remove(bool canUndo = true);

        void togglePlayPause();

        void updatePlayPauseState();

        void totalTimeChanged(qint64 newTotalTime);

        void hideToolBar();


    protected:
        virtual void buildButtons();

        UBGraphicsMediaItem* delegated();

        DelegateButton* mPlayPauseButton;
        DelegateButton* mStopButton;
        DelegateButton* mMuteButton;
        DelegateMediaControl* mMediaControl;

        QTimer* mToolBarShowTimer;
        int m_iToolBarShowingInterval;
};

#endif /* UBGRAPHICSMEDIAITEMDELEGATE_H_ */
