/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBVIDEOPLAYER_H_
#define UBVIDEOPLAYER_H_


#include <QtGui>
#include <phonon/VideoPlayer>

class UBVideoPlayerTransport;

class UBVideoPlayer : public QWidget
{
    Q_OBJECT;

    public:
        UBVideoPlayer(QWidget* pParent = 0);
        virtual ~UBVideoPlayer();

        Phonon::VideoPlayer* videoPlayer()
        {
            return mVideoPlayer;
        }

        void loadMedia(QUrl url)
        {
            mVideoPlayer->load(Phonon::MediaSource(url));
        }

    protected:

        Phonon::VideoPlayer* mVideoPlayer;
        QPushButton *mPlayPause;

    private slots:

        void mediaStateChanged(Phonon::State newstate, Phonon::State oldstate);
        void tooglePlayPause();

};


#endif /* UBVIDEOPLAYER_H_ */
