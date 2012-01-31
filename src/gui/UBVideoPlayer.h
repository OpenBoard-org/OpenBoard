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
