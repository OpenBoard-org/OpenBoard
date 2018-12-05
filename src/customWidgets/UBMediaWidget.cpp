/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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



#include "core/UBApplication.h"
#include "globals/UBGlobals.h"
#include "UBMediaWidget.h"

#include "core/memcheck.h"

/**
  * \brief Constructor
  * @param type as the media type
  * @param parent as the parent widget
  * @param name as the object name
  */
UBMediaWidget::UBMediaWidget(eMediaType type, QWidget *parent, const char *name):UBActionableWidget(parent, name)
  , mpMediaObject(NULL)
  , mpVideoWidget(NULL)
  , mpAudioOutput(NULL)
  , mpLayout(NULL)
  , mpSeekerLayout(NULL)
  , mpPlayStopButton(NULL)
  , mpPauseButton(NULL)
  , mpSlider(NULL)
  , mAutoUpdate(false)
  , mGeneratingThumbnail(false)
  , mBorder(5)
  , mpMediaContainer(NULL)
  , mMediaLayout(NULL)
  , mpCover(NULL)
{
    SET_STYLE_SHEET();

    addAction(eAction_Close);
    mType = type;
    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpPlayStopButton = new UBMediaButton(this);
    mpPlayStopButton->setPixmap(QPixmap(":images/play.svg"));
    mpPauseButton = new UBMediaButton(this);
    mpPauseButton->setPixmap(QPixmap(":images/pause.svg"));
    mpPauseButton->setEnabled(false);
    mpSlider = new QSlider(this);
    mpSlider->setOrientation(Qt::Horizontal);
    mpSlider->setMinimum(0);
    mpSlider->setMaximum(0);

    mpSeekerLayout = new QHBoxLayout();
    mpSeekerLayout->addWidget(mpPlayStopButton, 0);
    mpSeekerLayout->addWidget(mpPauseButton, 0);
    mpSeekerLayout->addWidget(mpSlider, 1);
    mpSeekerLayout->setContentsMargins(0, 0, 0, 0);

    connect(mpPlayStopButton, SIGNAL(clicked()), this, SLOT(onPlayStopClicked()));
    connect(mpPauseButton, SIGNAL(clicked()), this, SLOT(onPauseClicked()));
    connect(mpSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
}

/**
  * \brief Destructor
  */
UBMediaWidget::~UBMediaWidget()
{
    unsetActionsParent();
    DELETEPTR(mpMediaObject);
    DELETEPTR(mpSlider);
    DELETEPTR(mpPauseButton);
    DELETEPTR(mpPlayStopButton);
    DELETEPTR(mpAudioOutput);
    DELETEPTR(mpVideoWidget);
    DELETEPTR(mpCover);
    DELETEPTR(mpMediaContainer);
    DELETEPTR(mpSeekerLayout);
    DELETEPTR(mpLayout);
}

/**
  * \brief Set the media file
  * @param filePath as the media file path
  */
void UBMediaWidget::setFile(const QString &filePath)
{
    Q_ASSERT("" != filePath);
    mFilePath = filePath;
    mpMediaObject = new Phonon::MediaObject(this);
    mpMediaObject->setTickInterval(TICK_INTERVAL);
    connect(mpMediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(onStateChanged(Phonon::State,Phonon::State)));
    connect(mpMediaObject, SIGNAL(totalTimeChanged(qint64)), this, SLOT(onTotalTimeChanged(qint64)));
    connect(mpMediaObject, SIGNAL(tick(qint64)), this, SLOT(onTick(qint64)));
    mpMediaObject->setCurrentSource(Phonon::MediaSource(filePath));
    createMediaPlayer();
}

/**
  * \brief Get the media type
  * @returns the media type
  */
eMediaType UBMediaWidget::mediaType()
{
    return mType;
}

void UBMediaWidget::showEvent(QShowEvent* event)
{
	if(mType == eMediaType_Audio){
		return;
	}else{
		if(!mpVideoWidget){
			mpVideoWidget = new Phonon::VideoWidget(this);
			mMediaLayout->addStretch(1);
			mMediaLayout->addWidget(mpVideoWidget);
			mMediaLayout->addStretch(1);
			Phonon::createPath(mpMediaObject, mpVideoWidget);
			adaptSizeToVideo();
			mpMediaObject->play();
			mpMediaObject->stop();
		}
		QWidget::showEvent(event);
	}
}

void UBMediaWidget::hideEvent(QHideEvent* event)
{
    if(mpMediaObject->state() == Phonon::PlayingState)
        mpMediaObject->stop();
    UBActionableWidget::hideEvent(event);
}

/**
  * \brief Create the media player
  */
void UBMediaWidget::createMediaPlayer()
{
    mpMediaContainer = new QWidget();
    mpMediaContainer->setObjectName("UBMediaVideoContainer");
    mMediaLayout = new QHBoxLayout();
    mpMediaContainer->setLayout(mMediaLayout);

    if(eMediaType_Video == mType){
        mMediaLayout->setContentsMargins(10, 10, 10, 10);
        if(isVisible()){
            mpVideoWidget = new Phonon::VideoWidget(this);
            mMediaLayout->addStretch(1);
            mMediaLayout->addWidget(mpVideoWidget);
            mMediaLayout->addStretch(1);
            Phonon::createPath(mpMediaObject, mpVideoWidget);
            adaptSizeToVideo();
        }
        mpAudioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
        Phonon::createPath(mpMediaObject, mpAudioOutput);
    }else if(eMediaType_Audio == mType){
        mMediaLayout->setContentsMargins(10, 10, 10, 10);
        mpCover = new QLabel(mpMediaContainer);
        //mpMediaContainer->setStyleSheet(QString("background: none;"));
        setAudioCover(":images/libpalette/soundIcon.svg");
        mpCover->setScaledContents(true);
        mMediaLayout->addStretch(1);
        mMediaLayout->addWidget(mpCover);
        mMediaLayout->addStretch(1);
        mpAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        Phonon::createPath(mpMediaObject, mpAudioOutput);
    }
    mpLayout->addWidget(mpMediaContainer, 1);
    mpLayout->addLayout(mpSeekerLayout, 0);
    setActionsParent(mpMediaContainer);
}

/**
  * \brief Adapt the widget size to the video in order to keep the good aspect ratio
  */
void UBMediaWidget::adaptSizeToVideo()
{
    if(NULL != mpMediaContainer){
        int origW = mpMediaContainer->width();
        int origH = mpMediaContainer->height();
        int newW = width();
        float scaleFactor = (float)origW/(float)newW;
        int newH = origH/scaleFactor;
        resize(newW, height() + newH);
    }
}

/**
  * \brief Handle the media state change notification
  * @param newState as the new state
  * @param oldState as the old state
  */
void UBMediaWidget::onStateChanged(Phonon::State newState, Phonon::State oldState)
{
    if(!mGeneratingThumbnail){
        if(Phonon::LoadingState == oldState && Phonon::StoppedState == newState){
            if(eMediaType_Video == mType){
                // We do that here to generate the thumbnail of the video
                mGeneratingThumbnail = true;
                mpMediaObject->play();
                mpMediaObject->pause();
                mGeneratingThumbnail = false;
            }
        }else if(Phonon::PlayingState == oldState && Phonon::PausedState == newState){
            mpPlayStopButton->setPixmap(QPixmap(":images/play.svg"));
            mpPauseButton->setEnabled(false);
        }else if((Phonon::PausedState == oldState && Phonon::PlayingState == newState) ||
                 (Phonon::StoppedState == oldState && Phonon::PlayingState == newState)){
            mpPlayStopButton->setPixmap(QPixmap(":images/stop.svg"));
            mpPauseButton->setEnabled(true);
        }else if(Phonon::PlayingState == oldState && Phonon::StoppedState == newState){
            mpPlayStopButton->setPixmap(QPixmap(":images/play.svg"));
            mpPauseButton->setEnabled(false);
            mpSlider->setValue(0);
        }

    }
    //    if(mType == eMediaType_Video)
    //        updateView(newState);
}

/**
  * \brief Handles the total time change notification
  * @param total as the new total time
  */
void UBMediaWidget::onTotalTimeChanged(qint64 total)
{
    mpSlider->setMaximum(total);
}

/**
  * \brief Handles the tick notification
  * @param currentTime as the current time
  */
void UBMediaWidget::onTick(qint64 currentTime)
{
    mAutoUpdate = true;
    mpSlider->setValue((int)currentTime);
    mAutoUpdate = false;
}

/**
  * \brief Handles the seeker value change notification
  * @param value as the new seeker value
  */
void UBMediaWidget::onSliderChanged(int value)
{
    if(!mAutoUpdate){
        mpMediaObject->seek(value);
    }
}

/**
  * \brief Toggle Play-Stop
  */
void UBMediaWidget::onPlayStopClicked()
{
    switch(mpMediaObject->state()){
    case Phonon::PlayingState:
        mpMediaObject->stop();
        break;

    case Phonon::StoppedState:
    case Phonon::PausedState:
        mpMediaObject->play();
        break;
    default:
        break;
    }
}

/**
  * \brief Pause the media
  */
void UBMediaWidget::onPauseClicked()
{
    mpMediaObject->pause();
}

/**
  * Get the border
  * @returns the actual border
  */
int UBMediaWidget::border()
{
    return mBorder;
}

/**
  * \brief Handles the resize event
  * @param ev as the resize event
  */
void UBMediaWidget::resizeEvent(QResizeEvent* ev)
{
    Q_UNUSED(ev);
}

/**
  * \brief Set the audio cover
  * @param coverPath as the cover image file path
  */
void UBMediaWidget::setAudioCover(const QString &coverPath)
{
    if(NULL != mpCover){
        mpCover->setPixmap(QPixmap(coverPath));
    }
}

// -----------------------------------------------------------------------------------------------------------
/**
  * \brief Constructor
  * @param parent as the parent widget
  * @param name as the object name
  */
UBMediaButton::UBMediaButton(QWidget *parent, const char *name):QLabel(parent)
  , mPressed(false)
{
    setObjectName(name);
    resize(UBMEDIABUTTON_SIZE, UBMEDIABUTTON_SIZE);
    setStyleSheet(QString("padding:0px 0px 0px 0px; margin:0px 0px 0px 0px;"));
}

/**
  * \brief Destructor
  */
UBMediaButton::~UBMediaButton()
{

}

/**
  * \brief Handles the mouse press notification
  * @param ev as the mouse press event
  */
void UBMediaButton::mousePressEvent(QMouseEvent* ev)
{
    Q_UNUSED(ev);
    mPressed = true;
}

/**
  * \brief Handles the mouse release notification
  * @param ev as the mouse release event
  */
void UBMediaButton::mouseReleaseEvent(QMouseEvent* ev)
{
    Q_UNUSED(ev);
    if(mPressed){
        mPressed = false;
        emit clicked();
    }
}
