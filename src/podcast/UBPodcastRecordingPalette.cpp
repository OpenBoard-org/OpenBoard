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




#include "UBPodcastRecordingPalette.h"

#include "UBPodcastController.h"

#include "core/UBApplication.h"

#include "gui/UBResources.h"

#include "core/UBSettings.h"

#include "gui/UBMainWindow.h"

#include "core/memcheck.h"

UBPodcastRecordingPalette::UBPodcastRecordingPalette(QWidget *parent)
     : UBActionPalette(Qt::Horizontal, parent)
{
    addAction(UBApplication::mainWindow->actionPodcastRecord);

    mTimerLabel = new QLabel(this);
    mTimerLabel->setStyleSheet(QString("QLabel {color: white; font-size: 14px; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    recordingProgressChanged(0);

    layout()->addWidget(mTimerLabel);

    mLevelMeter = new UBVuMeter(this);
    mLevelMeter->setMinimumSize(6, 32);

    layout()->addWidget(mLevelMeter);

    addAction(UBApplication::mainWindow->actionPodcastConfig);

    foreach(QWidget* menuWidget,  UBApplication::mainWindow->actionPodcastConfig->associatedWidgets())
    {
        QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

        tb->setIconSize(QSize(16, 16));

        if (tb && !tb->menu())
        {
            tb->setObjectName("ubButtonMenu");
            tb->setPopupMode(QToolButton::InstantPopup);
            QMenu* menu = new QMenu(this);

            foreach(QAction* audioInputAction, UBPodcastController::instance()->audioRecordingDevicesActions())
            {
                menu->addAction(audioInputAction);
            }

            menu->addSeparator();

            foreach(QAction* videoSizeAction, UBPodcastController::instance()->videoSizeActions())
            {
                menu->addAction(videoSizeAction);
            }

            menu->addSeparator();

            QList<QAction*> podcastPublication = UBPodcastController::instance()->podcastPublicationActions();

            foreach(QAction* publicationAction, podcastPublication)
            {
                menu->addAction(publicationAction);
            }

            tb->setMenu(menu);
        }
    }
}


UBPodcastRecordingPalette::~UBPodcastRecordingPalette()
{
    // NOOP
}


void UBPodcastRecordingPalette::recordingStateChanged(UBPodcastController::RecordingState state)
{
    if (state == UBPodcastController::Recording)
    {
        UBApplication::mainWindow->actionPodcastRecord->setChecked(true);
        UBApplication::mainWindow->actionPodcastRecord->setEnabled(true);

        UBApplication::mainWindow->actionPodcastPause->setChecked(false);
        UBApplication::mainWindow->actionPodcastPause->setEnabled(true);

        //UBApplication::mainWindow->actionPodcastMic->setEnabled(false);

        UBApplication::mainWindow->actionPodcastConfig->setEnabled(false);
    }
    else if (state == UBPodcastController::Stopped)
    {
        UBApplication::mainWindow->actionPodcastRecord->setChecked(false);
        UBApplication::mainWindow->actionPodcastRecord->setEnabled(true);

        UBApplication::mainWindow->actionPodcastPause->setChecked(false);
        UBApplication::mainWindow->actionPodcastPause->setEnabled(false);

        //UBApplication::mainWindow->actionPodcastMic->setEnabled(true);
        UBApplication::mainWindow->actionPodcastConfig->setEnabled(true);
    }
    else if (state == UBPodcastController::Paused)
    {
        UBApplication::mainWindow->actionPodcastRecord->setChecked(true);
        UBApplication::mainWindow->actionPodcastRecord->setEnabled(true);

        UBApplication::mainWindow->actionPodcastPause->setChecked(true);
        UBApplication::mainWindow->actionPodcastPause->setEnabled(true);

        //UBApplication::mainWindow->actionPodcastMic->setEnabled(false);
        UBApplication::mainWindow->actionPodcastConfig->setEnabled(false);
    }
    else
    {
        UBApplication::mainWindow->actionPodcastRecord->setEnabled(false);
        UBApplication::mainWindow->actionPodcastPause->setEnabled(false);
        UBApplication::mainWindow->actionPodcastConfig->setEnabled(false);
    }
}


void UBPodcastRecordingPalette::recordingProgressChanged(qint64 ms)
{
    int min = ms / 60000;
    int seconds = (ms / 1000) % 60;

    mTimerLabel->setText(QString("%1:%2").arg(min, 3, 10, QChar(' ')).arg(seconds, 2, 10, QChar('0')));
}


void UBPodcastRecordingPalette::audioLevelChanged(quint8 level)
{
    mLevelMeter->setVolume(level);
}


UBVuMeter::UBVuMeter(QWidget* pParent)
    : QWidget(pParent)
    , mVolume(0)
{
    // NOOP
}


UBVuMeter::~UBVuMeter()
{
    // NOOP
}

void UBVuMeter::setVolume(quint8 pVolume)
{
    if (mVolume != pVolume)
    {
        mVolume = pVolume;
        update();
    }
}


void UBVuMeter::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    QPainter painter(this);

    int h = (height() - 8) * mVolume / 255;
    QRectF rect(0, height() - 4 - h, width(), h);

    painter.fillRect(rect, UBSettings::documentViewLightColor);
}

