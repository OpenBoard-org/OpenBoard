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

#include <QtGui>

#include "UBMediaPlayer.h"

#define SLIDER_RANGE 8


MediaVideoWidget::MediaVideoWidget(UBMediaPlayer *player, QWidget *parent) :
    Phonon::VideoWidget(parent), m_player(player)/*, m_action(this)*/
{
    //NOOP
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

    playButton = new QPushButton(this);
    playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    playButton->setIcon(playIcon);

    slider = new Phonon::SeekSlider(this);
    slider->setMediaObject(&m_MediaObject);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(1, 1, 1, 1);


    initVideoWindow();
    vLayout->addWidget(&m_videoWindow);
    QVBoxLayout *buttonPanelLayout = new QVBoxLayout();
#ifndef Q_WS_WIN
        m_videoWindow.hide();
#endif
    progressLabel = new QLabel(this);
    QWidget *sliderPanel = new QWidget(this);
    QHBoxLayout *sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(playButton);
    sliderLayout->addWidget(slider);
    sliderLayout->addWidget(progressLabel);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderPanel->setLayout(sliderLayout);

    buttonPanelLayout->addWidget(sliderPanel);
    buttonPanelLayout->setContentsMargins(0, 0, 0, 0);
#ifdef Q_OS_MAC
    buttonPanelLayout->setSpacing(0);
#endif
    QWidget *buttonPanelWidget = new QWidget(this);
    buttonPanelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    buttonPanelWidget->setLayout(buttonPanelLayout);
    vLayout->addWidget(buttonPanelWidget);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    vLayout->addLayout(labelLayout);
    setLayout(vLayout);

    connect(playButton, SIGNAL(clicked()), this, SLOT(playPause()));
    connect(&m_MediaObject, SIGNAL(finished()), this, SLOT(finished()));
    connect(&m_MediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(&m_MediaObject, SIGNAL(bufferStatus(int)), this, SLOT(bufferStatus(int)));
    connect(&m_MediaObject, SIGNAL(hasVideoChanged(bool)), this, SLOT(hasVideoChanged(bool)));

    playButton->setEnabled(false);

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
        } else {
            m_MediaObject.pause();
        }
        QMessageBox::warning(this, "Phonon UBMediaPlayer", m_MediaObject.errorString(), QMessageBox::Close);
        break;

    case Phonon::StoppedState:
        // Fall through
    case Phonon::PausedState:
        playButton->setIcon(playIcon);
        if (m_MediaObject.currentSource().type() != Phonon::MediaSource::Invalid){
            playButton->setEnabled(true);
        } else {
            playButton->setEnabled(false);
        }
        break;
    case Phonon::PlayingState:
        playButton->setEnabled(true);
        playButton->setIcon(pauseIcon);
        if (m_MediaObject.hasVideo())
            m_videoWindow.show();
        // Fall through
    case Phonon::BufferingState:
        break;
    case Phonon::LoadingState:
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

void UBMediaPlayer::rewind()
{
    m_MediaObject.seek(0);
}

void UBMediaPlayer::forward()
{
    QList<Phonon::MediaSource> queue = m_MediaObject.queue();
    if (queue.size() > 0) {
        m_MediaObject.setCurrentSource(queue[0]);
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
    m_videoWindow.setVisible(bHasVideo);
}

void UBMediaPlayer::resizeEvent(QResizeEvent* pEvent)
{
    Q_UNUSED(pEvent);
}
