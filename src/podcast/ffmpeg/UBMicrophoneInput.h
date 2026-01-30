/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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

#ifndef UBMICROPHONEINPUT_H
#define UBMICROPHONEINPUT_H

#include <QtCore>
#include <QAudioInput>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QAudioDevice>
#include <QAudioSource>
#endif

/**
 * @brief The UBMicrophoneInput class captures uncompressed sound from a microphone.
 *
 * Audio samples can be read by connecting to the dataAvailable signal.
 */
class UBMicrophoneInput : public QObject
{
    Q_OBJECT

public:
    UBMicrophoneInput();
    virtual ~UBMicrophoneInput();

    bool init();
    void start();
    void stop();

    static QStringList availableDevicesNames();
    void setInputDevice(QString name = "");

    int channelCount();
    int sampleRate();
    int sampleSize();
    int sampleFormat();

signals:
    /// Send the new audio level, between 0 and 255
    void audioLevelChanged(quint8 level);

    /// Emitted when new audio data is available
    void dataAvailable(QByteArray data);

    void error(QString message);

private slots:
    void onAudioInputStateChanged(QAudio::State state);
    void onDataReady();

private:
    double sampleRelativeLevel(const char* sample);
    quint8 audioLevel(const QByteArray& data);
    QString getErrorString(QAudio::Error errorCode);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QAudioDevice mAudioDeviceInfo;
    QAudioSource* mAudioInput;
#else
    QAudioDeviceInfo mAudioDeviceInfo;
    QAudioInput* mAudioInput;
#endif

    QIODevice* mIODevice;
    QAudioFormat mAudioFormat;

    qint64 mSeekPos;
    quint8 mLastAudioLevel;
};

#endif // UBMICROPHONEINPUT_H
