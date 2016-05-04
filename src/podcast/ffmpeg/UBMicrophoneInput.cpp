#include "UBMicrophoneInput.h"

UBMicrophoneInput::UBMicrophoneInput()
    : mAudioInput(NULL)
    , mIODevice(NULL)
    , mSeekPos(0)
{
}

UBMicrophoneInput::~UBMicrophoneInput()
{
    if (mAudioInput)
        delete mAudioInput;
}

int UBMicrophoneInput::channelCount()
{
    return mAudioFormat.channelCount();
}

int UBMicrophoneInput::sampleRate()
{
    return mAudioFormat.sampleRate();
}

/* Return the sample size in bits */
int UBMicrophoneInput::sampleSize()
{
    return mAudioFormat.sampleSize();
}

/** Return the sample format in FFMpeg style (AVSampleFormat enum) */
int UBMicrophoneInput::sampleFormat()
{
    enum AVSampleFormat {
        AV_SAMPLE_FMT_NONE = -1,
        AV_SAMPLE_FMT_U8,
        AV_SAMPLE_FMT_S16,
        AV_SAMPLE_FMT_S32,
        AV_SAMPLE_FMT_FLT,
        AV_SAMPLE_FMT_DBL,
        AV_SAMPLE_FMT_U8P,
        AV_SAMPLE_FMT_S16P,
        AV_SAMPLE_FMT_S32P,
        AV_SAMPLE_FMT_FLTP,
        AV_SAMPLE_FMT_DBLP,
        AV_SAMPLE_FMT_NB
    };

    int sampleSize = mAudioFormat.sampleSize();
    QAudioFormat::SampleType sampleType = mAudioFormat.sampleType();

    // qDebug() << "Input sample format: " << sampleSize << "bits " << sampleType;

    switch (sampleType) {
        case QAudioFormat::Unknown:
            return AV_SAMPLE_FMT_NONE;

        case QAudioFormat::SignedInt:
            if (sampleSize == 16)
                return AV_SAMPLE_FMT_S16;
            if (sampleSize == 32)
                return AV_SAMPLE_FMT_S32;

        case QAudioFormat::UnSignedInt:
            if (sampleSize == 8)
                return AV_SAMPLE_FMT_U8;

        case QAudioFormat::Float:
            return AV_SAMPLE_FMT_FLT;

        default:
            return AV_SAMPLE_FMT_NONE;
    }
}

QString UBMicrophoneInput::codec()
{
    return mAudioFormat.codec();
}

qint64 UBMicrophoneInput::processUSecs() const
{
    return mAudioInput->processedUSecs();
}

bool UBMicrophoneInput::init()
{
    if (mAudioDeviceInfo.isNull()) {
        qWarning("No audio input device selected; using default");
        mAudioDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    }

    qDebug() << "Input device name: " << mAudioDeviceInfo.deviceName();

    mAudioFormat = mAudioDeviceInfo.preferredFormat();

    mAudioInput = new QAudioInput(mAudioDeviceInfo, mAudioFormat, NULL);
    //mAudioInput->setNotifyInterval(100);

    connect(mAudioInput, SIGNAL(stateChanged(QAudio::State)),
            this, SLOT(onAudioInputStateChanged(QAudio::State)));

    return true;
}

void UBMicrophoneInput::start()
{
    qDebug() << "starting audio input";

    mIODevice = mAudioInput->start();

    connect(mIODevice, SIGNAL(readyRead()),
            this, SLOT(onDataReady()));

    if (mAudioInput->error() == QAudio::OpenError)
        qWarning() << "Error opening audio input";
}

void UBMicrophoneInput::stop()
{
    mAudioInput->stop();
}

QStringList UBMicrophoneInput::availableDevicesNames()
{
    QStringList names;
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    foreach (QAudioDeviceInfo device, devices) {
        names.push_back(device.deviceName());
    }

    return names;
}

void UBMicrophoneInput::setInputDevice(QString name)
{
    if (name.isEmpty()) {
        mAudioDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
        return;
    }

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    bool found = false;

    foreach (QAudioDeviceInfo device, devices) {
        if (device.deviceName() == name) {
            mAudioDeviceInfo = device;
            found = true;
            break;
        }
    }

    if (!found) {
        qWarning() << "Audio input device not found; using default instead";
        mAudioDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    }

}

void UBMicrophoneInput::onDataReady()
{
    int numBytes = mAudioInput->bytesReady();

    if (numBytes > 0)
        emit dataAvailable(mIODevice->read(numBytes));
}

void UBMicrophoneInput::onAudioInputStateChanged(QAudio::State state)
{
    qDebug() << "Audio input state changed to " << state;
    switch (state) {
        case QAudio::StoppedState:
            if (mAudioInput->error() != QAudio::NoError) {
                emit error(getErrorString(mAudioInput->error()));
            }
            break;

        // handle other states?

        default:
            break;
    }
}


/**
 * @brief Return a meaningful error string based on QAudio error codes
 */
QString UBMicrophoneInput::getErrorString(QAudio::Error errorCode)
{
    switch (errorCode) {
        case QAudio::NoError :
            return "";

        case QAudio::OpenError :
            return "Couldn't open the audio device";

        case QAudio::IOError :
            return "Error reading from audio device";

        case QAudio::UnderrunError :
            return "Underrun error";

        case QAudio::FatalError :
            return "Fatal error; audio device unusable";

    }
    return "";
}
