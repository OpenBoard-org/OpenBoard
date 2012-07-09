/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UBGRAPHICSMEDIAITEMDELEGATE_H_
#define UBGRAPHICSMEDIAITEMDELEGATE_H_

#include <QtGui>
#include <phonon/MediaObject>
#include <QTimer>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"

class QGraphicsSceneMouseEvent;
class QGraphicsItem;

class UBGraphicsMediaItemDelegate :  public UBGraphicsItemDelegate
{
    Q_OBJECT

    public:
        UBGraphicsMediaItemDelegate(UBGraphicsMediaItem* pDelegated, Phonon::MediaObject* pMedia, QObject * parent = 0);
        virtual ~UBGraphicsMediaItemDelegate();

        virtual void positionHandles();

        bool mousePressEvent(QGraphicsSceneMouseEvent *event);

    public slots:

        void toggleMute();
        void updateTicker(qint64 time);

    protected slots:

        virtual void remove(bool canUndo = true);

        void togglePlayPause();

        void mediaStateChanged ( Phonon::State newstate, Phonon::State oldstate );

        void updatePlayPauseState();

        void totalTimeChanged(qint64 newTotalTime);

        void hideToolBar();

    protected:
        virtual void buildButtons();

        UBGraphicsMediaItem* delegated();

        DelegateButton* mPlayPauseButton;
        DelegateButton* mStopButton;
        DelegateButton* mMuteButton;
        DelegateMediaControl *mMediaControl;

        Phonon::MediaObject* mMedia;

        QTimer *mToolBarShowTimer;
        int m_iToolBarShowingInterval;
};

#endif /* UBGRAPHICSMEDIAITEMDELEGATE_H_ */