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

#ifndef UBUBMediaPlayer_H
#define UBUBMediaPlayer_H

#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtCore/QTimerEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QIcon>
#include <QtCore/QBasicTimer>
#include <QtGui/QAction>

#include <phonon/audiooutput.h>
#include <phonon/backendcapabilities.h>
#include <phonon/effect.h>
#include <phonon/effectparameter.h>
#include <phonon/effectwidget.h>
#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#include <phonon/videowidget.h>
#include <phonon/volumeslider.h>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QSlider;
class QTextEdit;
QT_END_NAMESPACE

class UBMediaPlayer;

class MediaVideoWidget : public Phonon::VideoWidget
{
    Q_OBJECT;

public:
    MediaVideoWidget(UBMediaPlayer *player, QWidget *parent = 0);

protected:
    void timerEvent(QTimerEvent *e);

private:
    UBMediaPlayer* m_player;
    QBasicTimer m_timer;
};

class UBMediaPlayer : public QWidget
{
    Q_OBJECT
public:
    UBMediaPlayer();

    void setFile(const QString &text);
    void setLocation(const QString &location);
    void setVolume(qreal volume);

public slots:
    void openFile();
    void rewind();
    void forward();
    void finished();
    void playPause();

protected:
    void resizeEvent(QResizeEvent* pEvent);

private slots:
    void stateChanged(Phonon::State newstate, Phonon::State oldstate);
    void bufferStatus(int percent);
    void openUrl();

    void hasVideoChanged(bool);

private:
    void initVideoWindow();

    QIcon playIcon;
    QIcon pauseIcon;
    QPushButton *playButton;
    Phonon::SeekSlider *slider;
    QLabel *progressLabel;

    QWidget m_videoWindow;
    Phonon::MediaObject m_MediaObject;
    Phonon::AudioOutput m_AudioOutput;
    MediaVideoWidget *m_videoWidget;
    Phonon::Path m_audioOutputPath;
};



#endif // UBUBMediaPlayer_H
