#include "core/UBApplication.h"
#include "UBGlobals.h"
#include "UBMediaWidget.h"

UBMediaWidget::UBMediaWidget(eMediaType type, QWidget *parent, const char *name):QWidget(parent)
  , mpMediaObject(NULL)
  , mpVideoWidget(NULL)
  , mpAudioOutput(NULL)
  , mpPlayStopButton(NULL)
  , mpPauseButton(NULL)
  , mpSlider(NULL)
  , mAutoUpdate(false)
  , mGeneratingThumbnail(false)
  , mBorder(5)
  , mpMediaContainer(NULL)
  , mpCover(NULL)
{
    setObjectName(name);

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    mType = type;
    setLayout(&mLayout);

    mpPlayStopButton = new UBMediaButton(this);
    mpPlayStopButton->setPixmap(QPixmap(":images/play.svg"));
    mpPauseButton = new UBMediaButton(this);
    mpPauseButton->setPixmap(QPixmap(":images/pause.svg"));
    mpPauseButton->setEnabled(false);
    mpSlider = new QSlider(this);
    mpSlider->setOrientation(Qt::Horizontal);
    mpSlider->setMinimum(0);
    mpSlider->setMaximum(0);

    mSeekerLayout.addWidget(mpPlayStopButton, 0);
    mSeekerLayout.addWidget(mpPauseButton, 0);
    mSeekerLayout.addWidget(mpSlider, 1);
    mSeekerLayout.setContentsMargins(0, 0, 0, 0);

    connect(mpPlayStopButton, SIGNAL(clicked()), this, SLOT(onPlayStopClicked()));
    connect(mpPauseButton, SIGNAL(clicked()), this, SLOT(onPauseClicked()));
    connect(mpSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
}

UBMediaWidget::~UBMediaWidget()
{
    DELETEPTR(mpSlider);
    DELETEPTR(mpPauseButton);
    DELETEPTR(mpPlayStopButton);
    DELETEPTR(mpAudioOutput);
    DELETEPTR(mpVideoWidget);
    DELETEPTR(mpMediaObject);
    DELETEPTR(mpCover);
}

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

eMediaType UBMediaWidget::mediaType()
{
    return mType;
}

void UBMediaWidget::createMediaPlayer()
{
    mpMediaContainer = new QWidget(this);
    mpMediaContainer->setObjectName("UBMediaVideoContainer");
    mpMediaContainer->setLayout(&mMediaLayout);

    if(eMediaType_Video == mType){
        mMediaLayout.setContentsMargins(10, 10, 25, 10);
        mpVideoWidget = new Phonon::VideoWidget(this);
        mMediaLayout.addStretch(1);
        mMediaLayout.addWidget(mpVideoWidget, 0);
        mMediaLayout.addStretch(1);
        Phonon::createPath(mpMediaObject, mpVideoWidget);
        adaptSizeToVideo();
        mpAudioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
        Phonon::createPath(mpMediaObject, mpAudioOutput);
    }else if(eMediaType_Audio == mType){
        mMediaLayout.setContentsMargins(10, 10, 10, 10);
        mpCover = new QLabel(mpMediaContainer);
        mpMediaContainer->setStyleSheet(QString("background: none;"));
        setAudioCover(":images/libpalette/soundIcon.svg");
        mpCover->setScaledContents(true);
        mMediaLayout.addStretch(1);
        mMediaLayout.addWidget(mpCover, 0);
        mMediaLayout.addStretch(1);
        mpAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        Phonon::createPath(mpMediaObject, mpAudioOutput);
    }
    mLayout.addWidget(mpMediaContainer, 1);
    mLayout.addLayout(&mSeekerLayout, 0);
}

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
}

void UBMediaWidget::onTotalTimeChanged(qint64 total)
{
    mpSlider->setMaximum(total);
}

void UBMediaWidget::onTick(qint64 currentTime)
{
    mAutoUpdate = true;
    mpSlider->setValue((int)currentTime);
    mAutoUpdate = false;
}

void UBMediaWidget::onSliderChanged(int value)
{
    if(!mAutoUpdate){
        mpMediaObject->seek(value);
    }
}

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

void UBMediaWidget::onPauseClicked()
{
    mpMediaObject->pause();
}

int UBMediaWidget::border()
{
    return mBorder;
}

void UBMediaWidget::resizeEvent(QResizeEvent* ev)
{
    Q_UNUSED(ev);
}

void UBMediaWidget::setAudioCover(const QString &coverPath)
{
    if(NULL != mpCover){
        mpCover->setPixmap(QPixmap(coverPath));
    }
}

// -----------------------------------------------------------------------------------------------------------
UBMediaButton::UBMediaButton(QWidget *parent, const char *name):QLabel(parent)
  , mPressed(false)
{
    setObjectName(name);
    resize(UBMEDIABUTTON_SIZE, UBMEDIABUTTON_SIZE);
    setStyleSheet(QString("padding:0px 0px 0px 0px; margin:0px 0px 0px 0px;"));
}

UBMediaButton::~UBMediaButton()
{

}

void UBMediaButton::mousePressEvent(QMouseEvent* ev)
{
    Q_UNUSED(ev);
    mPressed = true;
}

void UBMediaButton::mouseReleaseEvent(QMouseEvent* ev)
{
    Q_UNUSED(ev);
    if(mPressed){
        mPressed = false;
        emit clicked();
    }
}
