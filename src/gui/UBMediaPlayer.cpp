#include "UBMediaPlayer.h"



#include <QtGui>

#define SLIDER_RANGE 8


MediaVideoWidget::MediaVideoWidget(UBMediaPlayer *player, QWidget *parent) :
    Phonon::VideoWidget(parent), m_player(player)/*, m_action(this)*/
{
//    m_action.setCheckable(true);
//    m_action.setChecked(false);
//    m_action.setShortcut(QKeySequence( Qt::AltModifier + Qt::Key_Return));
//    m_action.setShortcutContext(Qt::WindowShortcut);
//    connect(&m_action, SIGNAL(toggled(bool)), SLOT(setFullScreen(bool)));
//    addAction(&m_action);
//    setAcceptDrops(true);
}

void MediaVideoWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_timer.timerId()) {
        //let's store the cursor shape
#ifndef QT_NO_CURSOR
        setCursor(Qt::BlankCursor);
#endif
    }
    Phonon::VideoWidget::timerEvent(e);
}


UBMediaPlayer::UBMediaPlayer() :
    playButton(0),
    m_AudioOutput(Phonon::VideoCategory),
    m_videoWidget(new MediaVideoWidget(this))
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    m_videoWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    QSize buttonSize(26, 20);

//    QPushButton *openButton = new QPushButton(this);

////    openButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
////    QPalette bpal;
////    QColor arrowcolor = bpal.buttonText().color();
////    if (arrowcolor == Qt::black)
////        arrowcolor = QColor(80, 80, 80);
////    bpal.setBrush(QPalette::ButtonText, arrowcolor);
////    openButton->setPalette(bpal);

//    rewindButton = new QPushButton(this);
//    rewindButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

//    forwardButton = new QPushButton(this);
//    forwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
//    forwardButton->setEnabled(false);

    playButton = new QPushButton(this);
    playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    playButton->setIcon(playIcon);

    slider = new Phonon::SeekSlider(this);
    slider->setMediaObject(&m_MediaObject);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(1, 1, 1, 1);

//    QHBoxLayout *layout = new QHBoxLayout();

//    info = new QLabel(this);
//    info->setMinimumHeight(70);
//    info->setAcceptDrops(false);
//    info->setMargin(2);
//    info->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
//    info->setLineWidth(2);
//    info->setAutoFillBackground(true);

//    QPalette palette;
//    palette.setBrush(QPalette::WindowText, Qt::white);
#ifndef Q_WS_MAC
//    rewindButton->setMinimumSize(buttonSize);
//    forwardButton->setMinimumSize(buttonSize);
    playButton->setMinimumSize(buttonSize);
#endif
//    info->setStyleSheet("border-image:url(:/images/screen.png) ; border-width:3px");
//    info->setPalette(palette);
//    info->setText(tr("<center>No media</center>"));


//    layout->addWidget(rewindButton);
//    layout->addWidget(playButton);
//    layout->addWidget(forwardButton);

//    layout->addStretch();

//    vLayout->addWidget(info);
    initVideoWindow();
    vLayout->addWidget(&m_videoWindow);
//    m_videoWidget->setStyleSheet(QString("background:red;"));
    QVBoxLayout *buttonPanelLayout = new QVBoxLayout();
#ifndef Q_WS_WIN
        m_videoWindow.hide();
#endif
//    buttonPanelLayout->addLayout(layout);

//    timeLabel = new QLabel(this);
    progressLabel = new QLabel(this);
    QWidget *sliderPanel = new QWidget(this);
//    sliderPanel->setStyleSheet(QString("background:green;"));
    QHBoxLayout *sliderLayout = new QHBoxLayout();
//    playButton->setStyleSheet(QString("background:yellow;"));
    sliderLayout->addWidget(playButton);
    sliderLayout->addWidget(slider);
//    sliderLayout->addWidget(timeLabel);
    sliderLayout->addWidget(progressLabel);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderPanel->setLayout(sliderLayout);

    buttonPanelLayout->addWidget(sliderPanel);
    buttonPanelLayout->setContentsMargins(0, 0, 0, 0);
#ifdef Q_OS_MAC
//    layout->setSpacing(4);
    buttonPanelLayout->setSpacing(0);
//    info->setMinimumHeight(100);
//    info->setFont(QFont("verdana", 15));
//    openButton->setFocusPolicy(Qt::NoFocus);
#endif
    QWidget *buttonPanelWidget = new QWidget(this);
    buttonPanelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    buttonPanelWidget->setLayout(buttonPanelLayout);
    vLayout->addWidget(buttonPanelWidget);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    vLayout->addLayout(labelLayout);
    setLayout(vLayout);


    // Setup signal connections:
//    connect(rewindButton, SIGNAL(clicked()), this, SLOT(rewind()));

    connect(playButton, SIGNAL(clicked()), this, SLOT(playPause()));
//    connect(forwardButton, SIGNAL(clicked()), this, SLOT(forward()));

//    connect(&m_MediaObject, SIGNAL(totalTimeChanged(qint64)), this, SLOT(updateTime()));
//    connect(&m_MediaObject, SIGNAL(tick(qint64)), this, SLOT(updateTime()));
    connect(&m_MediaObject, SIGNAL(finished()), this, SLOT(finished()));
    connect(&m_MediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(&m_MediaObject, SIGNAL(bufferStatus(int)), this, SLOT(bufferStatus(int)));
    connect(&m_MediaObject, SIGNAL(hasVideoChanged(bool)), this, SLOT(hasVideoChanged(bool)));

//    rewindButton->setEnabled(false);
    playButton->setEnabled(false);
//    setAcceptDrops(true);

    m_audioOutputPath = Phonon::createPath(&m_MediaObject, &m_AudioOutput);
    Phonon::createPath(&m_MediaObject, m_videoWidget);

    resize(minimumSizeHint());
}

void UBMediaPlayer::stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    if (oldstate == Phonon::LoadingState) {
        QRect videoHintRect = QRect(QPoint(0, 0), m_videoWindow.sizeHint());
        QApplication::desktop()->screenGeometry().intersected(videoHintRect);

        if (m_MediaObject.hasVideo()) {
             qApp->processEvents();
            resize(sizeHint());
        } else
            resize(minimumSize());

    }

    switch (newstate) {
    case Phonon::ErrorState:
        if (m_MediaObject.errorType() == Phonon::FatalError) {
            playButton->setEnabled(false);
//            rewindButton->setEnabled(false);
        } else {
            m_MediaObject.pause();
        }
        QMessageBox::warning(this, "Phonon UBMediaPlayer", m_MediaObject.errorString(), QMessageBox::Close);
        break;

    case Phonon::StoppedState:
//        m_videoWidget-> (false);
        // Fall through
    case Phonon::PausedState:
        playButton->setIcon(playIcon);
        if (m_MediaObject.currentSource().type() != Phonon::MediaSource::Invalid){
            playButton->setEnabled(true);
//            rewindButton->setEnabled(true);
        } else {
            playButton->setEnabled(false);
//            rewindButton->setEnabled(false);
        }
        break;
    case Phonon::PlayingState:
        playButton->setEnabled(true);
        playButton->setIcon(pauseIcon);
        if (m_MediaObject.hasVideo())
            m_videoWindow.show();
        // Fall through
    case Phonon::BufferingState:
//        rewindButton->setEnabled(true);
        break;
    case Phonon::LoadingState:
//        rewindButton->setEnabled(false);
        break;
    }

}

void UBMediaPlayer::setVolume(qreal volume)
{
    m_AudioOutput.setVolume(volume);
}


void UBMediaPlayer::initVideoWindow()
{
    QVBoxLayout *videoLayout = new QVBoxLayout();
    videoLayout->addWidget(m_videoWidget);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    m_videoWindow.setLayout(videoLayout);
    m_videoWindow.setMinimumSize(60, 40);
}

void UBMediaPlayer::playPause()
{
    if (m_MediaObject.state() == Phonon::PlayingState)
        m_MediaObject.pause();
    else {
        if (m_MediaObject.currentTime() == m_MediaObject.totalTime())
            m_MediaObject.seek(0);
        m_MediaObject.play();
    }
}

void UBMediaPlayer::setFile(const QString &fileName)
{
    setWindowTitle(fileName.right(fileName.length() - fileName.lastIndexOf('/') - 1));
    m_MediaObject.setCurrentSource(Phonon::MediaSource(fileName));
}

void UBMediaPlayer::setLocation(const QString& location)
{
    setWindowTitle(location.right(location.length() - location.lastIndexOf('/') - 1));
    m_MediaObject.setCurrentSource(Phonon::MediaSource(QUrl::fromEncoded(location.toUtf8())));
    m_MediaObject.play();
}


void UBMediaPlayer::openFile()
{
     QStringList fileNames = QFileDialog::getOpenFileNames(this, QString(), QDesktopServices::storageLocation(QDesktopServices::MusicLocation));


    m_MediaObject.clearQueue();
    if (fileNames.size() > 0) {
        QString fileName = fileNames[0];
        setFile(fileName);
        for (int i=1; i<fileNames.size(); i++)
            m_MediaObject.enqueue(Phonon::MediaSource(fileNames[i]));
    }
//    forwardButton->setEnabled(m_MediaObject.queue().size() > 0);
}

void UBMediaPlayer::bufferStatus(int percent)
{
    if (percent == 100)
        progressLabel->setText(QString());
    else {
        QString str = QString::fromLatin1("(%1%)").arg(percent);
        progressLabel->setText(str);
    }
}

//void UBMediaPlayer::updateTime()
//{
//    long len = m_MediaObject.totalTime();
//    long pos = m_MediaObject.currentTime();
//    QString timeString;
//    if (pos || len)
//    {
//        int sec = pos/1000;
//        int min = sec/60;
//        int hour = min/60;
//        int msec = pos;

//        QTime playTime(hour%60, min%60, sec%60, msec%1000);
//        sec = len / 1000;
//        min = sec / 60;
//        hour = min / 60;
//        msec = len;

//        QTime stopTime(hour%60, min%60, sec%60, msec%1000);
//        QString timeFormat = "m:ss";
//        if (hour > 0)
//            timeFormat = "h:mm:ss";
//        timeString = playTime.toString(timeFormat);
//        if (len)
//            timeString += " / " + stopTime.toString(timeFormat);
//    }
//    timeLabel->setText(timeString);
//}

void UBMediaPlayer::rewind()
{
    m_MediaObject.seek(0);
}

void UBMediaPlayer::forward()
{
    QList<Phonon::MediaSource> queue = m_MediaObject.queue();
    if (queue.size() > 0) {
        m_MediaObject.setCurrentSource(queue[0]);
//        forwardButton->setEnabled(queue.size() > 1);
        m_MediaObject.play();
    }
}

void UBMediaPlayer::openUrl()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("BrowserMainWindow"));
    QString sourceURL = settings.value("location").toString();
    bool ok = false;
    sourceURL = QInputDialog::getText(this, tr("Open Location"), tr("Please enter a valid address here:"), QLineEdit::Normal, sourceURL, &ok);
    if (ok && !sourceURL.isEmpty()) {
        setLocation(sourceURL);
        settings.setValue("location", sourceURL);
    }
}

void UBMediaPlayer::finished()
{
}

void UBMediaPlayer::hasVideoChanged(bool bHasVideo)
{
//    info->setVisible(!bHasVideo);
    m_videoWindow.setVisible(bHasVideo);
}

void UBMediaPlayer::resizeEvent(QResizeEvent* pEvent)
{
    Q_UNUSED(pEvent);
//    int origWidth = m_videoWindow.width();
//    int origHeight = m_videoWindow.height();

//    float scaleFactor = (float)origWidth / (float)width();
//    int newWidth = width();
//    int newHeigth = origHeight/scaleFactor;

//    m_videoWindow.resize(newWidth, newHeigth);
}

//*************************************************************************
UBDraggableMediaPlayer::UBDraggableMediaPlayer():UBMediaPlayer()
{
//    setAcceptDrops(true);
}

void UBDraggableMediaPlayer::setFile(const QString &text)
{
    mSourcePath = text;
    UBMediaPlayer::setFile(text);
}

void UBDraggableMediaPlayer::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(mSourcePath);
    mimeData->setUrls(urls);
    mimeData->setText(mSourcePath);


    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->start();
}

