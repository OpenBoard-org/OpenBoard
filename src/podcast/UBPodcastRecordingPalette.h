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

#ifndef UBPODCASTRECORDINGPALETTE_H_
#define UBPODCASTRECORDINGPALETTE_H_

#include "gui/UBActionPalette.h"
#include "UBPodcastController.h"

#include <QtGui>

class UBVuMeter;

class UBPodcastRecordingPalette : public UBActionPalette
{
    Q_OBJECT;

    public:
        UBPodcastRecordingPalette(QWidget *parent = 0);
        virtual ~UBPodcastRecordingPalette();

    public slots:

        void recordingStateChanged(UBPodcastController::RecordingState);
        void recordingProgressChanged(qint64 ms);
        void audioLevelChanged(quint8 level);

    private:
        QLabel *mTimerLabel;
        UBVuMeter *mLevelMeter;
};


class UBVuMeter : public QWidget
{

    public:
        UBVuMeter(QWidget* pParent);
        virtual ~UBVuMeter();

        void setVolume(quint8 pVolume);
    protected:

        virtual void paintEvent(QPaintEvent* e);

    private:
        quint8 mVolume;

};

#endif /* UBPODCASTRECORDINGPALETTE_H_ */
