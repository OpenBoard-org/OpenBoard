/*
 * UBVideoPlayer.h
 *
 *  Created on: 24 sept. 2009
 *      Author: Luc
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

    protected:

        Phonon::VideoPlayer* mVideoPlayer;
        QPushButton *mPlayPause;

    private slots:

        void mediaStateChanged(Phonon::State newstate, Phonon::State oldstate);
        void tooglePlayPause();

};


#endif /* UBVIDEOPLAYER_H_ */
