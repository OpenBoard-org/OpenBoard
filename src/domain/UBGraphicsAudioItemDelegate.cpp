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

#include "UBGraphicsAudioItemDelegate.h"
#include "domain/UBGraphicsAudioItem.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "UBGraphicsScene.h"

#include "core/memcheck.h"

UBGraphicsAudioItemDelegate::UBGraphicsAudioItemDelegate ( UBGraphicsAudioItem* pDelegated, QObject *parent )
        : UBGraphicsItemDelegate ( pDelegated, parent )
        , mDelegated ( pDelegated )
{
    //NOOP
}

UBGraphicsAudioItemDelegate::~UBGraphicsAudioItemDelegate()
{
    //NNOP
}


void UBGraphicsAudioItemDelegate::buildButtons()
{
    mPlayPauseButton = new DelegateButton ( ":/images/play.svg", mDelegated, mFrame );

    mStopButton = new DelegateButton ( ":/images/stop.svg", mDelegated, mFrame );
    mStopButton->hide();

    if ( mDelegated->isMuted() )
        mMuteButton = new DelegateButton ( ":/images/soundOff.svg", mDelegated, mFrame );
    else
        mMuteButton = new DelegateButton ( ":/images/soundOn.svg", mDelegated, mFrame );

    mMuteButton->hide();


    connect ( mPlayPauseButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( togglePlayPause() ) );
    connect ( mStopButton, SIGNAL ( clicked ( bool ) ), mDelegated->mediaObject(), SLOT ( stop() ) );
    connect ( mMuteButton, SIGNAL ( clicked ( bool ) ), mDelegated, SLOT ( toggleMute() ) );
    connect ( mMuteButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( toggleMute() ) );

    connect ( mDelegated->mediaObject(), SIGNAL ( stateChanged ( Phonon::State, Phonon::State ) ), this, SLOT ( mediaStateChanged ( Phonon::State, Phonon::State ) ) );
    connect ( mDelegated->mediaObject(), SIGNAL ( finished() ), this, SLOT ( updatePlayPauseState() ) );

    mButtons << mPlayPauseButton << mStopButton << mMuteButton;

}

void UBGraphicsAudioItemDelegate::togglePlayPause()
{
    if ( mDelegated && mDelegated->mediaObject() )
    {
        Phonon::MediaObject* media = mDelegated->mediaObject();

        if ( media->state() == Phonon::StoppedState )
        {
            media->play();
        }
        else if ( media->state() == Phonon::PlayingState )
        {
            if ( media->remainingTime() <= 0 )
            {
                media->stop();
                media->play();
            }
            else
            {
                media->pause();
                if ( mDelegated->scene() )
                    mDelegated->scene()->setModified ( true );
            }
        }
        else if ( media->state() == Phonon::PausedState )
        {
            if ( media->remainingTime() <= 0 )
            {
                media->stop();
            }

            media->play();
        }
        else  if ( media->state() == Phonon::LoadingState ){
            mDelegated->mediaObject()->setCurrentSource(mDelegated->mediaFileUrl());
            media->play();
        }
        else{
          qDebug() << "Media state "<< media->state() << " not supported";
        }
    }
}

void UBGraphicsAudioItemDelegate::toggleMute()
{
    if ( mDelegated->isMuted() )
        mMuteButton->setFileName ( ":/images/soundOff.svg" );
    else
        mMuteButton->setFileName ( ":/images/soundOn.svg" );
}

void UBGraphicsAudioItemDelegate::updatePlayPauseState()
{
    Phonon::MediaObject* media = mDelegated->mediaObject();

    if ( media->state() == Phonon::PlayingState )
        mPlayPauseButton->setFileName ( ":/images/pause.svg" );
    else
        mPlayPauseButton->setFileName ( ":/images/play.svg" );
}

void UBGraphicsAudioItemDelegate::mediaStateChanged ( Phonon::State newstate, Phonon::State oldstate )
{
    Q_UNUSED ( newstate );
    Q_UNUSED ( oldstate );
    updatePlayPauseState();
}

void UBGraphicsAudioItemDelegate::remove ( bool canUndo )
{
    mDelegated->mediaObject()->stop();
    UBGraphicsItemDelegate::remove ( canUndo );
}
