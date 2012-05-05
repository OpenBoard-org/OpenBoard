/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef UBGRAPHICSVIDEOITEMDELEGATE_H_
#define UBGRAPHICSVIDEOITEMDELEGATE_H_

#include <QtGui>
#include <phonon/MediaObject>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"

class QGraphicsSceneMouseEvent;
class QGraphicsItem;
class UBGraphicsVideoItem;

class DelegateVideoControl: public QGraphicsRectItem
{
    public:

        DelegateVideoControl(UBGraphicsVideoItem* pDelegated, QGraphicsItem * parent = 0);

        virtual ~DelegateVideoControl()
        {
            // NOOP
        }

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

        QPainterPath shape() const;

        void setAntiScale(qreal antiScale){ mAntiScale = antiScale; }

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        void updateTicker(qint64 time);
        void totalTimeChanged( qint64 newTotalTime);

   protected:


        void seekToMousePos(QPointF mousePos);

        UBGraphicsVideoItem* mDelegate;
        bool mDisplayCurrentTime;

        qreal mAntiScale;
        qint64 mCurrentTimeInMs;
        qint64 mTotalTimeInMs;

    private:
        int mStartWidth;
        QRectF mBalloon;
};


class UBGraphicsVideoItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT

    public:
        UBGraphicsVideoItemDelegate(UBGraphicsVideoItem* pDelegated, Phonon::MediaObject* pMedia, QObject * parent = 0);
        virtual ~UBGraphicsVideoItemDelegate();

        virtual void positionHandles();

    public slots:

        void toggleMute();
        void updateTicker(qint64 time);

    protected slots:

        virtual void remove(bool canUndo = true);

        void togglePlayPause();

        void mediaStateChanged ( Phonon::State newstate, Phonon::State oldstate );

        void updatePlayPauseState();

        void totalTimeChanged( qint64 newTotalTime);

    protected:

        virtual void buildButtons();

    private:

        UBGraphicsVideoItem* delegated();

        DelegateButton* mPlayPauseButton;
        DelegateButton* mStopButton;
        DelegateButton* mMuteButton;
        DelegateVideoControl *mVideoControl;

        Phonon::MediaObject* mMedia;
};


#endif /* UBGRAPHICSVIDEOITEMDELEGATE_H_ */
