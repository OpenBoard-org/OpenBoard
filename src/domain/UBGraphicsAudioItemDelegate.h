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
#ifndef UBGRAPHICSAUDIOITEMDELEGATE_H
#define UBGRAPHICSAUDIOITEMDELEGATE_H

#include <QtGui>
#include <phonon/MediaObject>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"


class QGraphicsItem;
class UBGraphicsAudioItem;


class UBGraphicsAudioItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT;

public:

    UBGraphicsAudioItemDelegate ( UBGraphicsAudioItem *pDelegated, QObject *parent = 0 );
    ~UBGraphicsAudioItemDelegate();

private slots:

    void togglePlayPause();
    void toggleMute();
    void remove ( bool canUndo );
    void updatePlayPauseState();

private:

    DelegateButton* mPlayPauseButton;
    DelegateButton* mStopButton;
    DelegateButton* mMuteButton;

    UBGraphicsAudioItem* mDelegated;

protected:

    virtual void buildButtons();

protected slots:

    void mediaStateChanged ( Phonon::State newstate, Phonon::State oldstate );


};

#endif // UBGRAPHICSAUDIOITEMDELEGATE_H
