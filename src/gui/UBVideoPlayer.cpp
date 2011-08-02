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

#include "UBVideoPlayer.h"

#include <QtGui>

#include <phonon/MediaObject>
#include <phonon/SeekSlider>
#include <phonon/VolumeSlider>

#include "core/memcheck.h"

UBVideoPlayer::UBVideoPlayer(QWidget* pParent)
    : QWidget(pParent)
    , mVideoPlayer(0)
{
    mVideoPlayer = new Phonon::VideoPlayer(Phonon::VideoCategory, this);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    //mainLayout->setSpacing(0);
    mainLayout->addWidget(mVideoPlayer);

    QHBoxLayout* transportLayout = new QHBoxLayout(this);
    mainLayout->addLayout(transportLayout);

    mPlayPause = new QPushButton(this);
    transportLayout->addWidget(mPlayPause);

    Phonon::SeekSlider *slider = new Phonon::SeekSlider(this);
    slider->setMediaObject(mVideoPlayer->mediaObject());
    slider->setPageStep(100);
    transportLayout->addWidget(slider);

    connect(mPlayPause, SIGNAL(clicked()),
            this, SLOT(tooglePlayPause()));

    connect(mVideoPlayer->mediaObject(), SIGNAL(stateChanged ( Phonon::State, Phonon::State)),
            this, SLOT(mediaStateChanged ( Phonon::State, Phonon::State)));

}


UBVideoPlayer::~UBVideoPlayer()
{
    // NOOP
}


void UBVideoPlayer::tooglePlayPause()
{
    if(mVideoPlayer->mediaObject()->state() == Phonon::PlayingState)
    {
        mVideoPlayer->mediaObject()->pause();
    }
    else
    {
         mVideoPlayer->mediaObject()->play();
    }

}


void UBVideoPlayer::mediaStateChanged( Phonon::State newstate, Phonon::State oldstate )
{
    Q_UNUSED(oldstate);

    if (newstate == Phonon::PlayingState)
    {
        mPlayPause->setIcon(QPixmap(":/images/pause.svg"));
    }
    else
    {
        mPlayPause->setIcon(QPixmap(":/images/play.svg"));
    }
}

