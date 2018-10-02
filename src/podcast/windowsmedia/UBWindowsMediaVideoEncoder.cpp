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




#include "UBWindowsMediaVideoEncoder.h"

#include <QtGui>

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"


#include "core/memcheck.h"

UBWindowsMediaVideoEncoder::UBWindowsMediaVideoEncoder(QObject* pParent)
    : UBAbstractVideoEncoder(pParent)
    , mWMVideo(0)
    , mWaveRecorder(0)
    , mRecordAudio(true)
    , mLastAudioLevel(0)
    , mIsPaused(false)
{
    // NOOP
}


UBWindowsMediaVideoEncoder::~UBWindowsMediaVideoEncoder()
{
    // NOOP
}


bool UBWindowsMediaVideoEncoder::start()
{
    QString profile = UBFileSystemUtils::readTextFile(":/podcast/OpenBoard.prx");

    profile.replace("{in.videoWidth}", QString("%1").arg(videoSize().width()));
    profile.replace("{in.videoHeight}", QString("%1").arg(videoSize().height()));
    profile.replace("{in.bitsPerSecond}", QString("%1").arg(videoBitsPerSecond()));
    profile.replace("{in.nanoSecondsPerFrame}", QString("%1").arg(10000000 / framesPerSecond()));

    qDebug() << profile;

    if(mRecordAudio)
    {
        mWaveRecorder = new UBWaveRecorder(this);

        bool audioAvailable = mWaveRecorder->init(audioRecordingDevice()) && mWaveRecorder->start();

        if (!audioAvailable)
        {
            mWaveRecorder->deleteLater();
            mWaveRecorder = 0;
            mRecordAudio = false;
        }
    }

    mWMVideo = new UBWindowsMediaFile(this);

    if(!mWMVideo->init(videoFileName().replace("/", "\\"), profile, framesPerSecond(),
        videoSize().width(), videoSize().height(), 32))
    {
        mWMVideo->deleteLater();
        return false;
    }

    if (mRecordAudio)
    {
        connect(mWaveRecorder, SIGNAL(newWaveBuffer(WAVEHDR*, long)), mWMVideo
                        , SLOT(appendAudioBuffer(WAVEHDR*, long)), Qt::DirectConnection);
        connect(mWaveRecorder, SIGNAL(newWaveBuffer(WAVEHDR*, long)), this
                        , SLOT(processAudioBuffer(WAVEHDR*, long)), Qt::DirectConnection);
    }

    mIsRecording = true;

    return true;
}


bool UBWindowsMediaVideoEncoder::stop()
{
    bool audioOk = true;

    if (mWaveRecorder)
    {
        disconnect(mWaveRecorder, SIGNAL(newWaveBuffer(WAVEHDR*, long)), mWMVideo
                , SLOT(appendAudioBuffer(WAVEHDR*, long)));
        disconnect(mWaveRecorder, SIGNAL(newWaveBuffer(WAVEHDR*, long)), this
                , SLOT(processAudioBuffer(WAVEHDR*, long)));

        mWaveRecorder->stop();
        audioOk = mWaveRecorder->close();
        mLastErrorMessage = mWaveRecorder->lastErrorMessage();
        mWaveRecorder->deleteLater();
        mWaveRecorder = 0;
        emit audioLevelChanged(0);
    }

    bool videoOk = true;

    if (mWMVideo)
    {
        videoOk = mWMVideo->close();
        mLastErrorMessage = mWMVideo->lastErrorMessage();
        mWMVideo->deleteLater();
        mWMVideo = 0;
    }

    bool ok = audioOk && videoOk;

    emit encodingFinished(ok);

    mIsRecording = false;

    return ok;
}


void UBWindowsMediaVideoEncoder::newPixmap(const QImage& pPix, long timestamp)
{
    if(mWMVideo && !mIsPaused)
    {
        if(!mWMVideo->appendVideoFrame(pPix, timestamp))
        {
            qWarning() << "Error adding new video frame" << mWMVideo->lastErrorMessage();
        }
    }
}


void UBWindowsMediaVideoEncoder::newChapter(const QString& pLabel, long timestamp)
{
    if(mWMVideo)
       mWMVideo->startNewChapter(pLabel, timestamp);
}


void UBWindowsMediaVideoEncoder::setRecordAudio(bool pRecordAudio)
{
    if (mRecordAudio != pRecordAudio)
    {
        mRecordAudio = pRecordAudio;

        if (mRecordAudio)
        {
            connect(mWaveRecorder, SIGNAL(newWaveBuffer(WAVEHDR*, long)), mWMVideo, SLOT(appendAudioBuffer(WAVEHDR*, long)), Qt::DirectConnection);
        }
        else
        {
            disconnect(mWaveRecorder, SIGNAL(newWaveBuffer(WAVEHDR*, long)), mWMVideo, SLOT(appendAudioBuffer(WAVEHDR*, long)));
            emit audioLevelChanged(0);
        }
    }
}


void UBWindowsMediaVideoEncoder::processAudioBuffer(WAVEHDR* waveBuffer, long timestamp)
{
    Q_UNUSED(timestamp);

    if(mWaveRecorder && mRecordAudio)
    {
        long samplesCount = waveBuffer->dwBytesRecorded / 2;
        qint16* samples = (qint16*)waveBuffer->lpData;
        quint16 maxRMS = 0;

        for(long i = 0; i < samplesCount; i++)
        {
            quint8 current = qAbs(samples[i] / 128);

            quint16 currentRMS = current * current;
            maxRMS = qMax(maxRMS, currentRMS);
        }

        quint8 max = sqrt((qreal)maxRMS);

        if (max != mLastAudioLevel)
        {
            mLastAudioLevel = max;
            emit audioLevelChanged(mLastAudioLevel);
        }
    }

}


bool UBWindowsMediaVideoEncoder::pause()
{
    bool result = true;

    if(!mIsPaused && mIsRecording)
    {
        if(mWaveRecorder)
        {
            result = mWaveRecorder->stop();
            emit audioLevelChanged(0);
        }

        mIsPaused = true;
    }

    return result;
}


bool UBWindowsMediaVideoEncoder::unpause()
{
    bool result = true;

    if (mIsPaused && mIsRecording)
    {
        if(mWaveRecorder)
        {
            result = mWaveRecorder->start();
        }

        mIsPaused = false;
    }

    return result;
}




