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
//    QAction m_action;
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
//    void updateTime();
    void finished();
    void playPause();

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
//    QPushButton *rewindButton;
//    QPushButton *forwardButton;
    Phonon::SeekSlider *slider;
//    QLabel *timeLabel;
    QLabel *progressLabel;
//    QLabel *info;

    QWidget m_videoWindow;
    Phonon::MediaObject m_MediaObject;
    Phonon::AudioOutput m_AudioOutput;
    MediaVideoWidget *m_videoWidget;
    Phonon::Path m_audioOutputPath;
};

class UBDraggableMediaPlayer : public UBMediaPlayer
{
    Q_OBJECT
public:
    UBDraggableMediaPlayer();
    void setFile(const QString &text);
protected:
    QString mSourcePath;
    void mousePressEvent(QMouseEvent *event);
};


#endif // UBUBMediaPlayer_H
