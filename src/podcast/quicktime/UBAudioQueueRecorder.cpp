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




#include "UBAudioQueueRecorder.h"

#include "core/memcheck.h"

AudioStreamBasicDescription UBAudioQueueRecorder::sAudioFormat;

UBAudioQueueRecorder::UBAudioQueueRecorder(QObject* pParent)
    : QObject(pParent)
    , mQueue(0)
    , mIsRecording(false)
    , mBufferLengthInMs(500)
{

    int sampleSize = sizeof(float); // TODO: check if this is system/ microphone-dependant
    
    sAudioFormat.mSampleRate = 44100.0;
    sAudioFormat.mFormatID = kAudioFormatLinearPCM;
    sAudioFormat.mChannelsPerFrame = 1;

    sAudioFormat.mBytesPerFrame = sampleSize;
    sAudioFormat.mBitsPerChannel = 8 * sampleSize;
    sAudioFormat.mBytesPerPacket = sampleSize;
    sAudioFormat.mFramesPerPacket = 1;
    sAudioFormat.mFormatFlags = kAudioFormatFlagIsFloat | 
                                kAudioFormatFlagsNativeEndian | 
                                kAudioFormatFlagIsPacked;
    
    
}


UBAudioQueueRecorder::~UBAudioQueueRecorder()
{
    // NOOP
}


QStringList UBAudioQueueRecorder::waveInDevices()
{
    QStringList deviceNames;

    QList<AudioDeviceID> devices = inputDeviceIDs();

    for (int i = 0; i < devices.length(); i++)
    {
        QString name = deviceNameFromDeviceID(devices[i]);
        if (name.length() > 0)
                deviceNames << name;
    }

    return deviceNames;
}


QList<AudioDeviceID> UBAudioQueueRecorder::inputDeviceIDs()
{
    QList<AudioDeviceID> inputDeviceIDs;
    UInt32 deviceIDsArraySize(0);

    AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &deviceIDsArraySize, 0);

    AudioDeviceID deviceIDs[deviceIDsArraySize / sizeof(AudioDeviceID)];

    AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &deviceIDsArraySize, deviceIDs);

    int deviceIDsCount = deviceIDsArraySize / sizeof(AudioDeviceID);

    for (int i = 0; i < deviceIDsCount; i ++)
    {
        AudioStreamBasicDescription sf;
        UInt32 size = sizeof(AudioStreamBasicDescription);

        if (noErr == AudioDeviceGetProperty(deviceIDs[i], 0, true, kAudioDevicePropertyStreamFormat,  &size, &sf))
        {
                inputDeviceIDs << deviceIDs[i];
        }
    }

    /*
    foreach(AudioDeviceID id, inputDeviceIDs)
    {
            qDebug() << "Device" << id <<  deviceNameFromDeviceID(id) << deviceUIDFromDeviceID(id);
    }
    */

    return inputDeviceIDs;
}


AudioDeviceID UBAudioQueueRecorder::defaultInputDeviceID()
{
    AudioDeviceID deviceID;
    UInt32 size = sizeof(deviceID);

    if (noErr == AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &size, &deviceID))
    {
        return deviceID;
    }
    else
    {
        return 0;
    }
}


AudioDeviceID UBAudioQueueRecorder::deviceIDFromDeviceName(const QString& name)
{
    QList<AudioDeviceID> devices = inputDeviceIDs();

    for (int i = 0; i < devices.length(); i++)
    {
        QString deviceName = deviceNameFromDeviceID(devices[i]);
        if (deviceName == name)
                return devices[i];
    }

    if (devices.length() > 0)
        return devices[0];
    else
        return 0;
}


QString UBAudioQueueRecorder::deviceUIDFromDeviceID(AudioDeviceID id)
{
    CFStringRef name;
    UInt32 size = sizeof(CFStringRef);
    QString uid;

    if (noErr == AudioDeviceGetProperty(id, 0, true, kAudioDevicePropertyDeviceUID, &size, &name))
    {
        char *cname = new char[1024];

        CFStringGetCString (name, cname, 1024, kCFStringEncodingISOLatin1);
        int length = CFStringGetLength (name);

        uid = QString::fromLatin1(cname, length);

        delete cname;

    }

    CFRelease(name);

    return uid;
}


QString UBAudioQueueRecorder::deviceNameFromDeviceID(AudioDeviceID id)
{
    CFStringRef name;
    UInt32 size = sizeof(CFStringRef);
    QString deviceName;

    if (noErr == AudioDeviceGetProperty(id, 0, true, kAudioObjectPropertyName, &size, &name))
    {
        char cname[1024];
        memset(cname,0,1024);
        CFStringGetCString (name, cname, 1024, kCFStringEncodingUTF8);
        deviceName = QString::fromUtf8(cname);
    }

    CFRelease(name);

    return deviceName;

}


bool UBAudioQueueRecorder::init(const QString& waveInDeviceName)
{
    if(mIsRecording)
    {
        setLastErrorMessage("Already recording ...");
        return false;
    }

    OSStatus err = AudioQueueNewInput (&sAudioFormat, UBAudioQueueRecorder::audioQueueInputCallback,
                    this, 0, 0, 0, &mQueue);

    if (err)
    {
        setLastErrorMessage(QString("Cannot acquire audio input %1").arg(err));
        mQueue = 0;
        close();
        return false;
    }

    //qDebug() << "init with waveInDeviceName ..." << waveInDeviceName;

    if (waveInDeviceName.length() > 0 && waveInDeviceName != "Default")
    {
        AudioDeviceID deviceID = deviceIDFromDeviceName(waveInDeviceName);

        if (deviceID)
        {
            QString deviceUID = deviceUIDFromDeviceID(deviceID);
            if (deviceUID.length() > 0)
            {
                CFStringRef sr = CFStringCreateWithCString(0, deviceUID.toUtf8().constData(), kCFStringEncodingUTF8);

                err = AudioQueueSetProperty(mQueue, kAudioQueueProperty_CurrentDevice, &sr, sizeof(CFStringRef));
                if (err)
                {
                    setLastErrorMessage(QString("Cannot set audio input %1 (%2)").arg(waveInDeviceName).arg(err));
                }
                else
                {
                    qDebug() << "recording with input" << waveInDeviceName;
                }
            }
            else
            {
                setLastErrorMessage(QString("Cannot find audio input device UID with ID %1 (%2)").arg(deviceID).arg(err));
            }
        }
        else
        {
            setLastErrorMessage(QString("Cannot find audio input with name %1 (%2)").arg(waveInDeviceName).arg(err));
        }
    }

    UInt32 monitor = true;

    err = AudioQueueSetProperty(mQueue, kAudioQueueProperty_EnableLevelMetering , &monitor, sizeof(UInt32));
    if (err)
    {
        qWarning() << QString("Cannot set recording level monitoring %1").arg(err);
    }

    int nbBuffers = 6;
    mSampleBufferSize = sAudioFormat.mSampleRate *  sAudioFormat.mChannelsPerFrame
                * sAudioFormat.mBitsPerChannel / 8 * mBufferLengthInMs / 1000; 
        // BufferSize [bytes] = Length [s] * 44100 frames per second [Fr./s] * channels per frame [Ch./Fr.] * bytes per channel [bytes/Ch.]

    for (int i = 0; i < nbBuffers; i++)
    {
        AudioQueueBufferRef outBuffer;
        err = AudioQueueAllocateBuffer(mQueue, mSampleBufferSize, &outBuffer);

        if (err)
        {
            setLastErrorMessage(QString("Cannot allocate audio buffer %1").arg(err));
            close();
            return false;
        }

        mBuffers << outBuffer;
    }

    foreach(AudioQueueBufferRef buffer, mBuffers)
    {
        err = AudioQueueEnqueueBuffer(mQueue, buffer, 0, 0);
        if (err)
        {
            setLastErrorMessage(QString("Cannot enqueue audio buffer %1").arg(err));
            close();
            return false;
        }
    }

    mMsTimeStamp = 0;

    err = AudioQueueStart(mQueue, 0);

    if (err)
    {
        setLastErrorMessage(QString("Cannot starting audio queue %1").arg(err));
        close();
        return false;
    }

    mIsRecording = true;

    return true;

}



bool UBAudioQueueRecorder::close()
{
    if(mIsRecording)
    {
        mIsRecording = false;

        OSStatus err = AudioQueueStop(mQueue, true);

        if(err)
        {
            setLastErrorMessage(QString("Cannot stop audio queue %1").arg(err));
        }
    }

    foreach(AudioQueueBufferRef buffer, mBuffers)
    {
        OSStatus err = AudioQueueFreeBuffer(mQueue, buffer);
        if (err)
        {
            setLastErrorMessage(QString("Cannot free audio buffer %1").arg(err));
        }
    }

    OSStatus err = AudioQueueDispose(mQueue, true);
    mQueue = 0;

    if(err)
    {
        setLastErrorMessage(QString("Cannot dispose audio queue %1").arg(err));
        return false;
    }

    return true;
}


void UBAudioQueueRecorder::audioQueueInputCallback (void *inUserData, 
                                                    AudioQueueRef inAQ,
                                                    AudioQueueBufferRef inBuffer, 
                                                    const AudioTimeStamp *inStartTime,
                                                    UInt32 inNumberPacketDescriptions, 
                                                    const AudioStreamPacketDescription *inPacketDescs)
{
    Q_UNUSED(inAQ);
    Q_UNUSED(inStartTime)
    UBAudioQueueRecorder* recorder = (UBAudioQueueRecorder*)inUserData;

    if(recorder && recorder->isRecording() && inBuffer)
    {
        recorder->emitNewWaveBuffer(inBuffer, inNumberPacketDescriptions, inPacketDescs);

        OSStatus err = AudioQueueEnqueueBuffer(recorder->mQueue, inBuffer, 0, 0);

        if(err)
        {
            recorder->setLastErrorMessage(QString("Cannot reenqueue buffer %1").arg(err));
        }
    }
}


void UBAudioQueueRecorder::emitNewWaveBuffer(AudioQueueBufferRef pBuffer,
                                             int inNumberPacketDescriptions, 
                                             const AudioStreamPacketDescription *inPacketDescs)
{

    emit newWaveBuffer(pBuffer->mAudioData, pBuffer->mAudioDataByteSize);

    qreal level = 0;
    UInt32 size;

    if (noErr == AudioQueueGetPropertySize (mQueue, kAudioQueueProperty_CurrentLevelMeter, &size))
    {
        AudioQueueLevelMeterState levels[size / sizeof(AudioQueueLevelMeterState)];
        size = sizeof(levels);

        if (noErr == AudioQueueGetProperty(mQueue, kAudioQueueProperty_CurrentLevelMeter, &levels, &size))
        {
            if (size == 1)
                level = levels[0].mAveragePower;
            else if (size >= 2)
                level = (levels[0].mAveragePower + levels[1].mAveragePower) / 2;
        }
        else
        {
            qDebug() << "cannot retreive audio level";
        }
    }

    emit audioLevelChanged(level * 255);
}


