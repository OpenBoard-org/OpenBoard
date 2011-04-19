/*
 * UBPodcastRecordingPalette.h
 *
 *  Created on: 1 oct. 2009
 *      Author: Luc
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
