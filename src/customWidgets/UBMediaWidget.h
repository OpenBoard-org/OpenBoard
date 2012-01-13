#ifndef UBMEDIAWIDGET_H
#define UBMEDIAWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QMouseEvent>

#include <phonon/MediaObject>
#include <phonon/VideoWidget>
#include <phonon/AudioOutput>

#include "interfaces/IResizeable.h"

#define UBMEDIABUTTON_SIZE              32
#define TICK_INTERVAL                   1000

typedef enum{
    eMediaType_Video,
    eMediaType_Audio
}eMediaType;

class UBMediaButton : public QLabel
{
    Q_OBJECT
public:
    UBMediaButton(QWidget* parent=0, const char* name="UBMediaButton");
    ~UBMediaButton();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* ev);
    void mouseReleaseEvent(QMouseEvent* ev);

private:
    bool mPressed;
};

class UBMediaWidget : public QWidget
{
    Q_OBJECT
public:
    UBMediaWidget(eMediaType type = eMediaType_Video, QWidget* parent=0, const char* name="UBMediaWidget");
    ~UBMediaWidget();
    void setFile(const QString& filePath);
    eMediaType mediaType();
    int border();
    void setAudioCover(const QString& coverPath);

protected:
    void resizeEvent(QResizeEvent* ev);

private slots:
    void onPlayStopClicked();
    void onPauseClicked();
    void onStateChanged(Phonon::State newState, Phonon::State oldState);
    void onTotalTimeChanged(qint64 total);
    void onTick(qint64 currentTime);
    void onSliderChanged(int value);

private:
    void createMediaPlayer();
    void adaptSizeToVideo();

    QString mFilePath;
    eMediaType mType;
    Phonon::MediaObject* mpMediaObject;
    Phonon::VideoWidget* mpVideoWidget;
    Phonon::AudioOutput* mpAudioOutput;
    QVBoxLayout mLayout;
    QHBoxLayout mSeekerLayout;
    UBMediaButton* mpPlayStopButton;
    UBMediaButton* mpPauseButton;
    QSlider* mpSlider;
    bool mAutoUpdate;
    bool mGeneratingThumbnail;
    int mBorder;
    QWidget* mpMediaContainer;
    QHBoxLayout mMediaLayout;
    QLabel* mpCover;
};

#endif // UBMEDIAWIDGET_H
