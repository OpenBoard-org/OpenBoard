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




#ifndef UBPODCASTRECORDINGPALETTE_H_
#define UBPODCASTRECORDINGPALETTE_H_

#include "gui/UBActionPalette.h"
#include "UBPodcastController.h"

#include <QtGui>
#include <QLabel>

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
