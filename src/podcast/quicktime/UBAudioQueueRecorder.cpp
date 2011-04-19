/*
 * UBAudioQueueRecorder.cpp
 *
 *  Created on: Sep 11, 2009
 *      Author: luc
 */

#include "UBAudioQueueRecorder.h"


AudioStreamBasicDescription UBAudioQueueRecorder::sAudioFormat;

UBAudioQueueRecorder::UBAudioQueueRecorder(QObject* pParent)
    : QObject(pParent)
    , mQueue(0)
    , mIsRecording(false)
    , mBufferLengthInMs(500)
{
    sAudioFormat.mSampleRate = 44100.0;
    sAudioFormat.mFormatID = kAudioFormatMPEG4AAC;
    sAudioFormat.mChannelsPerFrame = 2;

    sAudioFormat.mBytesPerFrame = 0;
    sAudioFormat.mBitsPerChannel = 0;
    sAudioFormat.mBytesPerPacket = 0;
    sAudioFormat.mFramesPerPacket = 0;
    sAudioFormat.mFormatFlags = 0;
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

        bool result = CFStringGetCString (name, cname, 1024, kCFStringEncodingASCII);
        int length = CFStringGetLength (name);

        uid = QString::fromAscii(cname, length);

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
        char *cname = new char[1024];

        bool result = CFStringGetCString (name, cname, 1024, kCFStringEncodingUTF8);
        int length = CFStringGetLength (name);

        deviceName = QString::fromUtf8(cname, length);

        delete cname;
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
                * 2 * mBufferLengthInMs / 1000; // 44.1 Khz * stereo * 16bit * buffer length

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


void UBAudioQueueRecorder::audioQueueInputCallback (void *inUserData, AudioQueueRef inAQ,
                                                            AudioQueueBufferRef inBuffer, const AudioTimeStamp *inStartTime,
                                                            UInt32 inNumberPacketDescriptions, const AudioStreamPacketDescription *inPacketDescs)
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
                int inNumberPacketDescriptions, const AudioStreamPacketDescription *inPacketDescs)
{
    emit newWaveBuffer(pBuffer->mAudioData, pBuffer->mAudioDataByteSize, inNumberPacketDescriptions, inPacketDescs);

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


