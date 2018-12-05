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


#ifndef UBMEDIAWIDGET_H
#define UBMEDIAWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QMouseEvent>
#include <QStackedWidget>

#include "UBActionableWidget.h"

#define UBMEDIABUTTON_SIZE              32
#define TICK_INTERVAL                   1000

/**
  * \brief The media type
  */
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
    /** And indicator of the press event in progress */
    bool mPressed;
};

class UBMediaWidget : public UBActionableWidget
{
    Q_OBJECT
public:
    UBMediaWidget(eMediaType type = eMediaType_Video, QWidget* parent=0, const char* name="UBMediaWidget");
    ~UBMediaWidget();
    void setFile(const QString& filePath);
    eMediaType mediaType();
    int border();
    void setAudioCover(const QString& coverPath);
    void setUrl(const QString& url){mUrl = url;}
    QString url(){return mUrl;}

protected:
    void resizeEvent(QResizeEvent* ev);
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    /** The current media file path */
    QString mFilePath;

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

    /** The current media type */
    eMediaType mType;
    /** The media object */
    Phonon::MediaObject* mpMediaObject;
    /** The video renderer */
    Phonon::VideoWidget* mpVideoWidget;
    /** The audio renderer */
    Phonon::AudioOutput* mpAudioOutput;
    /** The principal layout of this widget */
    QVBoxLayout* mpLayout;
    /** The seeker layout */
    QHBoxLayout* mpSeekerLayout;
    /** The play-stop button */
    UBMediaButton* mpPlayStopButton;
    /** The pause button */
    UBMediaButton* mpPauseButton;
    /** The seeker slider */
    QSlider* mpSlider;
    /** An indicator of the seeker auto update in progress */
    bool mAutoUpdate;
    /** An indicator of the thumbnail generation in progress */
    bool mGeneratingThumbnail;
    /** The border */
    int mBorder;
    /** A widget that will contain the media */
    QWidget* mpMediaContainer;
    /** The media layout */
    QHBoxLayout* mMediaLayout;
    /** The audio cover */
    QLabel* mpCover;
    /** The media url */
    QString mUrl;
};

#endif // UBMEDIAWIDGET_H
